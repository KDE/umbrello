/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "classimport.h"
// qt/kde includes
#include <qmap.h>
#include <qregexp.h>
//#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "uml.h"
#include "umldoc.h"
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
#include "idlimport.h"
#include "classparser/lexer.h"
#include "classparser/driver.h"
#include "classparser/cpptree2uml.h"

// static members
CppDriver * ClassImport::ms_driver;
QStringList ClassImport::ms_seenFiles;
bool ClassImport::ms_putAtGlobalScope;

class CppDriver : public Driver {
public:
    void setupLexer(Lexer* lexer) {
        Driver::setupLexer(lexer);
        lexer->setRecordComments(true);
    }
};

ClassImport::ClassImport() {
    ms_driver = new CppDriver();
    ms_putAtGlobalScope = false;
}

ClassImport::~ClassImport() {}

QString ClassImport::formatComment(const QString &comment) {
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
UMLObject* ClassImport::findUMLObject(QString name,
                                      Uml::Object_Type type) {
    // Why an extra wrapper? See comment at addMethodParameter()
    UMLObject * o = umldoc->findUMLObject(name, type);
    return o;
}
 */

UMLObject *ClassImport::createUMLObject(Uml::Object_Type type,
                                        QString name,
                                        UMLPackage *parentPkg,
                                        QString comment,
                                        QString stereotype) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    UMLObject * o = umldoc->findUMLObject(name, type, parentPkg);
    if (o == NULL) {
        // Strip possible adornments and look again.
        int isConst = name.contains(QRegExp("^const "));
        name.remove(QRegExp("^const\\s+"));
        QString typeName(name);
        int isPointer = typeName.contains('*');
        int isRef = typeName.contains('&');
        typeName.remove(QRegExp("[^\\w: ].*$"));
        UMLObject *origType = umldoc->findUMLObject(typeName, Uml::ot_UMLObject, parentPkg);
        if (origType == NULL) {
            // Still not found. Create the stripped down type.
            if (ms_putAtGlobalScope)
                parentPkg = NULL;
            // Find, or create, the scopes.
            if (typeName.contains("::")) {
                QStringList components = QStringList::split("::", typeName);
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
                    int wantNamespace = KMessageBox::questionYesNo(NULL,
                                        i18n("Is the scope %1 a namespace or a class?").arg(scopeName),
                                        i18n("C++ Import Requests Your Help"),
                                        i18n("Namespace"), i18n("Class"));
                    Uml::Object_Type ot = (wantNamespace == KMessageBox::Yes ? Uml::ot_Package : Uml::ot_Class);
                    o = umldoc->createUMLObject(ot, scopeName, parentPkg);
                    parentPkg = static_cast<UMLPackage*>(o);
                }
                // All scope qualified datatypes live in the global scope.
                ms_putAtGlobalScope = true;
            }
            Uml::Object_Type t = type;
            if (type == Uml::ot_UMLObject || isConst || isRef || isPointer)
                t = Uml::ot_Class;
            origType = umldoc->createUMLObject(t, typeName, parentPkg);
        }
        if (isConst || isPointer || isRef) {
            // Create the full given type (including adornments.)
            if (isConst)
                name.prepend("const ");
            if (ms_putAtGlobalScope)
                parentPkg = NULL;
            o = umldoc->createUMLObject(Uml::ot_Datatype, name, parentPkg);
            UMLDatatype *dt = static_cast<UMLDatatype*>(o);
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(origType);
            if (c)
                dt->setOriginType(c);
            else
                kdError() << "ClassImport::createUMLObject(" << name << "): "
                << "origType " << typeName << " is not a UMLClassifier"
                << endl;
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
    } else if (parentPkg && !ms_putAtGlobalScope) {
        o->setUMLPackage(parentPkg);
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

UMLOperation* ClassImport::makeOperation(UMLClassifier *parent, const QString &name) {
    UMLOperation *op = new UMLOperation(parent, name);
    return op;
}

UMLObject* ClassImport::insertAttribute(UMLClassifier *owner, Uml::Scope scope, QString name,
                                        QString type, QString comment /* ="" */,
                                        bool isStatic /* =false */) {
    Uml::Object_Type ot = owner->getBaseType();
    if (ot != Uml::ot_Class) {
        kdDebug() << "ClassImport::insertAttribute: Don't know what to do with "
        << owner->getName() << " (object type " << ot << ")" << endl;
        return NULL;
    }
    UMLObjectList atts = owner->findChildObject(Uml::ot_Attribute, name);
    if (atts.count()) {
        return atts.first();
    }
    UMLObject *attrType = owner->findTemplate(type);
    if (attrType == NULL) {
        ms_putAtGlobalScope = true;
        attrType = createUMLObject(Uml::ot_UMLObject, type, owner);
        ms_putAtGlobalScope = false;
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

void ClassImport::insertMethod(UMLClassifier *klass, UMLOperation *op,
                               Uml::Scope scope, QString type,
                               bool isStatic, bool isAbstract,
                               QString comment) {
    op->setScope(scope);
    if (!type.isEmpty()) {  // return type may be missing (constructor/destructor)
        if (type == klass->getName()) {
            op->setType(klass);
        } else {
            UMLObject *typeObj = klass->findTemplate(type);
            if (typeObj == NULL) {
                ms_putAtGlobalScope = true;
                typeObj = createUMLObject(Uml::ot_UMLObject, type, klass);
                ms_putAtGlobalScope = false;
                op->setType(typeObj);
            }
        }
    }
    op->setStatic(isStatic);
    op->setAbstract(isAbstract);
    klass->addOperation(op);
    //umldoc->signalUMLObjectCreated(op);
    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        op->setDoc(strippedComment);
        UMLApp::app()->getDocWindow()->showDocumentation(op, true);
    }
    //setModified(true);
}

UMLAttribute* ClassImport::addMethodParameter(UMLOperation *method,
        QString type, QString name) {
    UMLClassifier *owner = static_cast<UMLClassifier*>(method->parent());
    UMLObject *typeObj = owner->findTemplate(type);
    if (typeObj == NULL) {
        ms_putAtGlobalScope = true;
        typeObj = createUMLObject(Uml::ot_UMLObject, type, owner);
        ms_putAtGlobalScope = false;
    }
    UMLAttribute *attr = new UMLAttribute(method, name);
    attr->setType(typeObj);
    method->addParm(attr);
    return attr;
}

void ClassImport::addEnumLiteral(UMLEnum *enumType, const QString &literal) {
    enumType->addEnumLiteral( literal );
}

void ClassImport::createGeneralization(UMLClassifier *child, const QString &parentName) {
    UMLObject *parentObj = createUMLObject( Uml::ot_Class, parentName );
    UMLClassifier *parent = static_cast<UMLClassifier*>(parentObj);
    UMLAssociation *assoc = new UMLAssociation( Uml::at_Generalization,
                            child, parent );
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    umldoc->addAssociation(assoc);
}

QStringList ClassImport::includePathList() {
    QStringList includePathList;
    char *umbrello_incpath = getenv( "UMBRELLO_INCPATH" );
    if (umbrello_incpath) {
        includePathList = QStringList::split( ':', umbrello_incpath );
    }
    return includePathList;
}

void ClassImport::feedTheModel(QString fileName) {
    QMap<QString, Dependence> deps = ms_driver->dependences(fileName);
    if (! deps.empty()) {
        QMap<QString, Dependence>::Iterator it;
        for (it = deps.begin(); it != deps.end(); ++it) {
            if (it.data().second == Dep_Global)  // don't want these
                continue;
            QString includeFile = it.key();
            if (includeFile.isEmpty()) {
                kdError() << fileName << ": " << it.data().first
                << " not found" << endl;
                continue;
            }
            kdDebug() << fileName << ": " << includeFile << " => " << it.data().first << endl;
            if (ms_seenFiles.find(includeFile) == ms_seenFiles.end())
                feedTheModel(includeFile);
        }
    }
    if (ms_seenFiles.find(fileName) != ms_seenFiles.end())
        return;
    ms_seenFiles.append(fileName);
    TranslationUnitAST *ast = ms_driver->translationUnit( fileName );
    if (ast == NULL) {
        kdError() << "ClassImport::feedTheModel: " << fileName << " not found" << endl;
        return;
    }
    CppTree2Uml modelFeeder( fileName );
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));
    modelFeeder.parseTranslationUnit( ast );
}

void ClassImport::importIDL(QStringList idlFileList) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    for (QStringList::Iterator fileIT = idlFileList.begin();
            fileIT != idlFileList.end(); ++fileIT) {
        QString fileName = (*fileIT);
        umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));
        IDLImport::parseFile(fileName);
    }
}

void ClassImport::importCPP(QStringList headerFileList) {
    // Reset the driver
    ms_driver->reset();
    // The driver shall attempt to parse included files.
    ms_driver->setResolveDependencesEnabled( true );
    // Add some standard include paths
    ms_driver->addIncludePath( "/usr/include" );
    ms_driver->addIncludePath( "/usr/include/c++" );
    ms_driver->addIncludePath( "/usr/include/g++" );
    ms_driver->addIncludePath( "/usr/local/include" );
    QStringList incPathList = includePathList();
    if (incPathList.count()) {
        QStringList::Iterator end(incPathList.end());
        for (QStringList::Iterator i(incPathList.begin()); i != end; ++i) {
            ms_driver->addIncludePath( *i );
        }

    }
    ms_seenFiles.clear();
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    for (QStringList::Iterator fileIT = headerFileList.begin();
            fileIT != headerFileList.end(); ++fileIT) {
        QString fileName = (*fileIT);
        umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));

        /// kapp->processEvents();
        // Invoking kapp->processEvents() improves responsiveness when importing
        // many large header files but slows down import because the list view is
        // intermittently updated.

        if (ms_seenFiles.find(fileName) != ms_seenFiles.end())
            continue;
        ms_driver->parseFile( fileName );
        feedTheModel(fileName);
    }
    umldoc->writeToStatusBar("Ready.");
}

