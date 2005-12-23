/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "import_utils.h"
// qt/kde includes
#include <qmap.h>
#include <qregexp.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlobject.h"
#include "docwindow.h"
#include "package.h"
#include "enum.h"
#include "datatype.h"
#include "classifier.h"
#include "operation.h"
#include "attribute.h"
#include "template.h"
#include "association.h"

namespace Import_Utils {

/**
 * Flag manipulated by createUMLObject().
 * Global state is generally bad, I know.
 * It would be cleaner to make this into a return value from
 * createUMLObject().
 */
bool bNewUMLObjectWasCreated = false;

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

bool newUMLObjectWasCreated() {
    return bNewUMLObjectWasCreated;
}

QString formatComment(const QString &comment) {
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
                                        QString name,
                                        UMLPackage *parentPkg,
                                        QString comment,
                                        QString stereotype) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    UMLObject * o = umldoc->findUMLObject(name, type, parentPkg);
    bNewUMLObjectWasCreated = false;
    if (o == NULL) {
        // Strip possible adornments and look again.
        int isConst = name.contains(QRegExp("^const "));
        name.remove(QRegExp("^const\\s+"));
        QString typeName(name);
        const int isAdorned = typeName.contains( QRegExp("[^\\w: ]") );
        const int isPointer = typeName.contains('*');
        const int isRef = typeName.contains('&');
        typeName.remove(QRegExp("[^\\w: ].*$"));
        UMLObject *origType = umldoc->findUMLObject(typeName, Uml::ot_UMLObject, parentPkg);
        if (origType == NULL) {
            // Still not found. Create the stripped down type.
            if (bPutAtGlobalScope)
                parentPkg = NULL;
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
                    int wantNamespace = 1;
                    /* We know std and Qt are always a namespaces */
                    if (scopeName != "std" && scopeName != "Qt") {
                        wantNamespace = KMessageBox::questionYesNo(NULL,
                                        i18n("Is the scope %1 a namespace or a class?").arg(scopeName),
                                        i18n("C++ Import Requests Your Help"),
                                        i18n("Namespace"), i18n("Class"));
                    }
                    Uml::Object_Type ot = (wantNamespace == KMessageBox::Yes ? Uml::ot_Package : Uml::ot_Class);
                    o = umldoc->createUMLObject(ot, scopeName, parentPkg);
                    kapp->processEvents();
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
            origType = umldoc->createUMLObject(t, typeName, parentPkg);
            bNewUMLObjectWasCreated = true;
            kapp->processEvents();
        }
        if (isConst || isAdorned) {
            // Create the full given type (including adornments.)
            if (isConst)
                name.prepend("const ");
            if (bPutAtGlobalScope) {
                parentPkg = NULL;
                bPutAtGlobalScope = false;
            }
            o = umldoc->createUMLObject(Uml::ot_Datatype, name, parentPkg);
            kapp->processEvents();
            UMLDatatype *dt = static_cast<UMLDatatype*>(o);
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(origType);
            if (c)
                dt->setOriginType(c);
            else
                kdError() << "createUMLObject(" << name << "): "
                << "origType " << typeName << " is not a UMLClassifier"
                << endl;
            if (isRef || isPointer)
                dt->setIsReference();
            /*
            if (isPointer) {
                UMLObject *pointerDecl = umldoc->createUMLObject(Uml::ot_Datatype, type);
                UMLDatatype *dt = static_cast<UMLDatatype*>(pointerDecl);
                dt->setOriginType(classifier);
                dt->setIsReference();
                classifier = dt;
            }  */
        } else {
            o = origType;
        }
    } else if (parentPkg && !bPutAtGlobalScope) {
        if (o->getUMLPackage())
            o->getUMLPackage()->removeObject(o);
        o->setUMLPackage(parentPkg);
        parentPkg->addObject(o);
    }
    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        o->setDoc(strippedComment);
        UMLApp::app()->getDocWindow()->showDocumentation(o, true);
    }
    if (!stereotype.isEmpty()) {
        o->setStereotype(stereotype);
    }
    return o;
}

UMLOperation* makeOperation(UMLClassifier *parent, const QString &name) {
    UMLOperation *op = new UMLOperation(parent, name);
    return op;
}

UMLObject* insertAttribute(UMLClassifier *owner, Uml::Visibility scope, QString name,
                                        QString type, QString comment /* ="" */,
                                        bool isStatic /* =false */) {
    Uml::Object_Type ot = owner->getBaseType();
    if (ot != Uml::ot_Class) {
        kdDebug() << "insertAttribute: Don't know what to do with "
        << owner->getName() << " (object type " << ot << ")" << endl;
        return NULL;
    }
    UMLObject *o = owner->findChildObject(name);
    if (o) {
        return o;
    }
    UMLObject *attrType = owner->findTemplate(type);
    if (attrType == NULL) {
        bPutAtGlobalScope = true;
        attrType = createUMLObject(Uml::ot_UMLObject, type, owner);
        bPutAtGlobalScope = false;
    }
    UMLAttribute *attr = owner->addAttribute(name, attrType, scope);
    attr->setStatic(isStatic);
    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        attr->setDoc(strippedComment);
        UMLApp::app()->getDocWindow()->showDocumentation(attr, true);
    }

    UMLApp::app()->getDocument()->setModified(true);
    return attr;
}

void insertMethod(UMLClassifier *klass, UMLOperation *op,
                               Uml::Visibility scope, QString type,
                               bool isStatic, bool isAbstract, 
                               bool isFriend, bool isConstructor,
                               QString comment) {
    op->setVisibility(scope);
    if (!type.isEmpty()) {  // return type may be missing (constructor/destructor)
        if (type == klass->getName()) {
            op->setType(klass);
        } else {
            UMLObject *typeObj = klass->findTemplate(type);
            if (typeObj == NULL) {
                bPutAtGlobalScope = true;
                typeObj = createUMLObject(Uml::ot_UMLObject, type, klass);
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

    klass->addOperation(op);
    //umldoc->signalUMLObjectCreated(op);
    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        op->setDoc(strippedComment);
        UMLApp::app()->getDocWindow()->showDocumentation(op, true);
    }
    //setModified(true);
}

UMLAttribute* addMethodParameter(UMLOperation *method,
        QString type, QString name) {
    UMLClassifier *owner = static_cast<UMLClassifier*>(method->parent());
    UMLObject *typeObj = owner->findTemplate(type);
    if (typeObj == NULL) {
        bPutAtGlobalScope = true;
        typeObj = createUMLObject(Uml::ot_UMLObject, type, owner);
        bPutAtGlobalScope = false;
    }
    UMLAttribute *attr = new UMLAttribute(method, name);
    attr->setType(typeObj);
    method->addParm(attr);
    return attr;
}

void addEnumLiteral(UMLEnum *enumType, const QString &literal) {
    enumType->addEnumLiteral( literal );
}

void createGeneralization(UMLClassifier *child, UMLClassifier *parent) {
    UMLAssociation *assoc = new UMLAssociation( Uml::at_Generalization,
                            child, parent );
    UMLDoc *umldoc = UMLApp::app()->getDocument();
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

void addIncludePath(QString path) {
    if (! incPathList.contains(path))
        incPathList.append(path);
}

}  // end namespace Import_Utils

