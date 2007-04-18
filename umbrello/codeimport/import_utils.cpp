/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "import_utils.h"
// qt/kde includes
#include <qmap.h>
#include <qregexp.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "../uml.h"
#include "../umldoc.h"
#include "../umllistview.h"
#include "../umllistviewitem.h"
#include "../umlobject.h"
#include "../package.h"
#include "../folder.h"
#include "../enum.h"
#include "../classifier.h"
#include "../operation.h"
#include "../attribute.h"
#include "../template.h"
#include "../association.h"
#include "../object_factory.h"

#include <stdlib.h>

namespace Import_Utils {

/**
 * Flag manipulated by createUMLObject().
 * Global state is generally bad, I know.
 * It would be cleaner to make this into a return value from
 * createUMLObject().
 */
bool bNewUMLObjectWasCreated = false;

/**
 * Related classifier for creation of dependencies on template
 * parameters in createUMLObject().
 */
UMLClassifier * gRelatedClassifier = NULL;

/**
 * On encountering a scoped typename string where the scopes
 * have not yet been seen, we synthesize UML objects for the
 * unknown scopes (using a question dialog to the user to decide
 * whether to treat a scope as a class or as a package.)
 * However, such an unknown scope is put at the global level.
 * I.e. before calling createUMLObject() we set this flag to true.
 */
bool bPutAtGlobalScope = false;

/**
 * The include path list (see addIncludePath() and includePathList())
 */
QStringList incPathList;

void putAtGlobalScope(bool yesno) {
    bPutAtGlobalScope = yesno;
}

void setRelatedClassifier(UMLClassifier *c) {
    gRelatedClassifier = c;
}

void assignUniqueIdOnCreation(bool yesno) {
    Object_Factory::assignUniqueIdOnCreation(yesno);
}

bool newUMLObjectWasCreated() {
    return bNewUMLObjectWasCreated;
}

QString formatComment(const QString &comment) {
    if (comment.isEmpty())
        return comment;

    QStringList lines = QStringList::split("\n", comment);
    QString& first = lines.first();
    QRegExp wordex("\\w");
    if (first.startsWith("/*")) {
        int wordpos = wordex.search(first);
        if (wordpos != -1)
            first = first.mid(wordpos);  // remove comment start
        else
            lines.pop_front();  // nothing interesting on this line
    }
    QString& last = lines.last();
    int endpos = last.find("*/");
    if (endpos != -1) {
        if (last.contains(wordex))
            last = last.mid(0, endpos - 1);  // remove comment end
        else
            lines.pop_back();  // nothing interesting on this line
    }
    if (! lines.count())
        return "";

    QStringList::Iterator end(lines.end());
    for (QStringList::Iterator lit(lines.begin()); lit != end; ++lit) {
        (*lit).remove(QRegExp("^\\s+"));
        (*lit).remove(QRegExp("^\\*+\\s?"));
    }
    return lines.join("\n");
}

/*
UMLObject* findUMLObject(QString name,
                                      Uml::Object_Type type) {
    // Why an extra wrapper? See comment at addMethodParameter()
    UMLObject * o = umldoc->findUMLObject(name, type);
    return o;
}
 */

UMLObject *createUMLObject(Uml::Object_Type type,
                           const QString& inName,
                           UMLPackage *parentPkg,
                           const QString& comment,
                           const QString& stereotype) {
    QString name = inName;
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    UMLFolder *logicalView = umldoc->getRootFolder(Uml::mt_Logical);
    const Uml::Programming_Language pl = UMLApp::app()->getActiveLanguage();
    if (parentPkg == NULL) {
        // kDebug() << "Import_Utils::createUMLObject(" << name
        //     << "): parentPkg is NULL, assuming Logical View" << endl;
        parentPkg = logicalView;
    }
    UMLObject * o = umldoc->findUMLObject(name, type, parentPkg);
    bNewUMLObjectWasCreated = false;
    if (o == NULL) {
        // Strip possible adornments and look again.
        int isConst = name.contains(QRegExp("^const "));
        name.remove(QRegExp("^const\\s+"));
        QString typeName(name);
        const int isAdorned = typeName.contains( QRegExp("[^\\w:\\. ]") );
        const int isPointer = typeName.contains('*');
        const int isRef = typeName.contains('&');
        typeName.remove(QRegExp("[^\\w:\\. ].*$"));
        typeName = typeName.simplifyWhiteSpace();
        UMLObject *origType = umldoc->findUMLObject(typeName, Uml::ot_UMLObject, parentPkg);
        if (origType == NULL) {
            // Still not found. Create the stripped down type.
            if (bPutAtGlobalScope)
                parentPkg = logicalView;
            // Find, or create, the scopes.
            QStringList components;
            if (typeName.contains("::")) {
                components = QStringList::split("::", typeName);
            } else if (typeName.contains(".")) {
                components = QStringList::split(".", typeName);
            }
            if (components.count() > 1) {
                typeName = components.back();
                components.pop_back();
                while ( components.count() ) {
                    QString scopeName = components.front();
                    components.pop_front();
                    o = umldoc->findUMLObject(scopeName, Uml::ot_UMLObject, parentPkg);
                    if (o) {
                        parentPkg = static_cast<UMLPackage*>(o);
                        continue;
                    }
                    int wantNamespace = KMessageBox::Yes;
                    if (pl == Uml::pl_Cpp) {
                        /* We know std and Qt are namespaces */
                        if (scopeName != "std" && scopeName != "Qt") {
                            wantNamespace = KMessageBox::questionYesNo(NULL,
                                        i18n("Is the scope %1 a namespace or a class?").arg(scopeName),
                                        i18n("C++ Import Requests Your Help"),
                                        i18n("Namespace"), i18n("Class"));
                        }
                    }
                    Uml::Object_Type ot = (wantNamespace == KMessageBox::Yes ? Uml::ot_Package : Uml::ot_Class);
                    o = Object_Factory::createUMLObject(ot, scopeName, parentPkg);
                    parentPkg = static_cast<UMLPackage*>(o);
                    UMLListView *listView = UMLApp::app()->getListView();
                    UMLListViewItem *lvitem = listView->findUMLObject(o);
                    listView->setCurrentItem(lvitem);
                }
                // All scope qualified datatypes live in the global scope.
                bPutAtGlobalScope = true;
            }
            Uml::Object_Type t = type;
            if (type == Uml::ot_UMLObject || isAdorned)
                t = Uml::ot_Class;
            origType = Object_Factory::createUMLObject(t, typeName, parentPkg, false);
            bNewUMLObjectWasCreated = true;
            bPutAtGlobalScope = false;
        }
        if (isConst || isAdorned) {
            // Create the full given type (including adornments.)
            if (isConst)
                name.prepend("const ");
            o = Object_Factory::createUMLObject(Uml::ot_Datatype, name,
                                                umldoc->getDatatypeFolder(),
                                                false); //solicitNewName
            UMLClassifier *dt = static_cast<UMLClassifier*>(o);
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(origType);
            if (c)
                dt->setOriginType(c);
            else
                kError() << "createUMLObject(" << name << "): "
                << "origType " << typeName << " is not a UMLClassifier"
                << endl;
            if (isRef || isPointer)
                dt->setIsReference();
            /*
            if (isPointer) {
                UMLObject *pointerDecl = Object_Factory::createUMLObject(Uml::ot_Datatype, type);
                UMLClassifier *dt = static_cast<UMLClassifier*>(pointerDecl);
                dt->setOriginType(classifier);
                dt->setIsReference();
                classifier = dt;
            }  */
        } else {
            o = origType;
        }
    } else if (parentPkg && !bPutAtGlobalScope) {
        UMLPackage *existingPkg = o->getUMLPackage();
        if (existingPkg != umldoc->getDatatypeFolder()) {
            if (existingPkg)
                existingPkg->removeObject(o);
            else
                kError() << "createUMLObject(" << name << "): "
                    << "o->getUMLPackage() was NULL" << endl;
            o->setUMLPackage(parentPkg);
            parentPkg->addObject(o);
        }
    }
    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        o->setDoc(strippedComment);
    }
    if (!stereotype.isEmpty()) {
        o->setStereotype(stereotype);
    }
    if (gRelatedClassifier == NULL || gRelatedClassifier == o)
        return o;
    QRegExp templateInstantiation("^[\\w:\\.]+\\s*<(.*)>");
    int pos = templateInstantiation.search(name);
    if (pos == -1)
        return o;
    // Create dependencies on template parameters.
    QString caption = templateInstantiation.cap(1);
    QStringList params = QStringList::split(QRegExp("[^\\w:\\.]+"), caption);
    if (!params.count())
        return o;
    QStringList::Iterator end(params.end());
    for (QStringList::Iterator it(params.begin()); it != end; ++it) {
        UMLObject *p = umldoc->findUMLObject(*it, Uml::ot_UMLObject, parentPkg);
        if (p == NULL || p->getBaseType() == Uml::ot_Datatype)
            continue;
        const Uml::Association_Type at = Uml::at_Dependency;
        UMLAssociation *assoc = umldoc->findAssociation(at, gRelatedClassifier, p);
        if (assoc)
            continue;
        assoc = new UMLAssociation(at, gRelatedClassifier, p);
        assoc->setUMLPackage(umldoc->getRootFolder(Uml::mt_Logical));
        umldoc->addAssociation(assoc);
    }
    return o;
}

UMLOperation* makeOperation(UMLClassifier *parent, const QString &name) {
    UMLOperation *op = Object_Factory::createOperation(parent, name);
    return op;
}

UMLObject* insertAttribute(UMLClassifier *owner,
                           Uml::Visibility scope,
                           const QString& name,
                           UMLClassifier *attrType,
                           const QString& comment /* ="" */,
                           bool isStatic /* =false */) {
    Uml::Object_Type ot = owner->getBaseType();
    Uml::Programming_Language pl = UMLApp::app()->getActiveLanguage();
    if (! (ot == Uml::ot_Class || ot == Uml::ot_Interface && pl == Uml::pl_Java)) {
        kDebug() << "insertAttribute: Don't know what to do with "
        << owner->getName() << " (object type " << ot << ")" << endl;
        return NULL;
    }
    UMLObject *o = owner->findChildObject(name, Uml::ot_Attribute);
    if (o) {
        return o;
    }

    UMLAttribute *attr = owner->addAttribute(name, attrType, scope);
    attr->setStatic(isStatic);
    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        attr->setDoc(strippedComment);
    }

    UMLApp::app()->getDocument()->setModified(true);
    return attr;
}

UMLObject* insertAttribute(UMLClassifier *owner, Uml::Visibility scope,
                           const QString& name,
                           const QString& type,
                           const QString& comment /* ="" */,
                           bool isStatic /* =false */) {
    UMLObject *attrType = owner->findTemplate(type);
    if (attrType == NULL) {
        bPutAtGlobalScope = true;
        gRelatedClassifier = owner;
        attrType = createUMLObject(Uml::ot_UMLObject, type, owner);
        gRelatedClassifier = NULL;
        bPutAtGlobalScope = false;
    }
    return insertAttribute (owner, scope, name,
                            static_cast<UMLClassifier*>(attrType),
                            comment, isStatic);
}

void insertMethod(UMLClassifier *klass, UMLOperation* &op,
                  Uml::Visibility scope, const QString& type,
                  bool isStatic, bool isAbstract,
                  bool isFriend, bool isConstructor,
                  const QString& comment) {
    op->setVisibility(scope);
    if (!type.isEmpty()     // return type may be missing (constructor/destructor)
        && type != "void") {
        if (type == klass->getName()) {
            op->setType(klass);
        } else {
            UMLObject *typeObj = klass->findTemplate(type);
            if (typeObj == NULL) {
                bPutAtGlobalScope = true;
                gRelatedClassifier = klass;
                typeObj = createUMLObject(Uml::ot_UMLObject, type, klass);
                gRelatedClassifier = NULL;
                bPutAtGlobalScope = false;
                op->setType(typeObj);
            }
        }
    }

    op->setStatic(isStatic);
    op->setAbstract(isAbstract);

    // if the operation is friend, add it as a stereotype
    if (isFriend)
        op->setStereotype("friend");
    // if the operation is a constructor, add it as a stereotype
    if (isConstructor)
        op->setStereotype("constructor");

    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        op->setDoc(strippedComment);
    }

    UMLAttributeList params = op->getParmList();
    UMLOperation *exist = klass->checkOperationSignature(op->getName(), params);
    if (exist) {
        // copy contents to existing operation
        exist->setVisibility(scope);
        exist->setStatic(isStatic);
        exist->setAbstract(isAbstract);
        if (! strippedComment.isEmpty())
            exist->setDoc(strippedComment);
        UMLAttributeList exParams = exist->getParmList();
        UMLAttribute *param, *exParam = exParams.first();
        for (UMLAttributeListIt it(params); (param = it.current()) != NULL;
                                            ++it, exParam = exParams.next()) {
            exParam->setName(param->getName());
            exParam->setVisibility(param->getVisibility());
            exParam->setStatic(param->getStatic());
            exParam->setAbstract(param->getAbstract());
            exParam->setDoc(param->getDoc());
            exParam->setInitialValue(param->getInitialValue());
            exParam->setParmKind(param->getParmKind());
        }
        // delete incoming UMLOperation and pass out the existing one
        delete op;
        op = exist;
    } else {
        klass->addOperation(op);
    }
}

UMLAttribute* addMethodParameter(UMLOperation *method,
                                 const QString& type,
                                 const QString& name) {
    UMLClassifier *owner = static_cast<UMLClassifier*>(method->parent());
    UMLObject *typeObj = owner->findTemplate(type);
    if (typeObj == NULL) {
        bPutAtGlobalScope = true;
        gRelatedClassifier = owner;
        typeObj = createUMLObject(Uml::ot_UMLObject, type, owner);
        gRelatedClassifier = NULL;
        bPutAtGlobalScope = false;
    }
    UMLAttribute *attr = Object_Factory::createAttribute(method, name, typeObj);
    method->addParm(attr);
    return attr;
}

void addEnumLiteral(UMLEnum *enumType, const QString &literal, const QString &comment) {
    UMLObject *el = enumType->addEnumLiteral(literal);
    el->setDoc(comment);
}

void createGeneralization(UMLClassifier *child, UMLClassifier *parent) {
    // if the child is an interface, so is the parent.
    if (child->isInterface())
        parent->setBaseType(Uml::ot_Interface);
    Uml::Association_Type association = Uml::at_Generalization;

    if (parent->isInterface() && !child->isInterface()) {
        // if the parent is an interface, but the child is not, then
        // this is really realization.
        //
        association = Uml::at_Realization;
    }
    UMLAssociation *assoc = new UMLAssociation(association, child, parent);
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    assoc->setUMLPackage(umldoc->getRootFolder(Uml::mt_Logical));
    umldoc->addAssociation(assoc);
}

void createGeneralization(UMLClassifier *child, const QString &parentName) {
    UMLObject *parentObj = createUMLObject( Uml::ot_Class, parentName );
    UMLClassifier *parent = static_cast<UMLClassifier*>(parentObj);
    createGeneralization(child, parent);
}

QStringList includePathList() {
    QStringList includePathList(incPathList);
    char *umbrello_incpath = getenv( "UMBRELLO_INCPATH" );
    if (umbrello_incpath) {
        includePathList += QStringList::split( ':', umbrello_incpath );
    }
    return includePathList;
}

void addIncludePath(const QString& path) {
    if (! incPathList.contains(path))
        incPathList.append(path);
}


bool isDatatype(const QString& name, UMLPackage *parentPkg) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    UMLObject * o = umldoc->findUMLObject(name, Uml::ot_Datatype, parentPkg);
    return (o!=NULL);
}

}  // end namespace Import_Utils

