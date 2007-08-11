/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2004-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "model_utils.h"

// qt/kde includes
#include <qregexp.h>
#include <qstringlist.h>
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// app includes
#include "umlobject.h"
#include "umlpackagelist.h"
#include "package.h"
#include "folder.h"
#include "classifier.h"
#include "enum.h"
#include "entity.h"
#include "template.h"
#include "operation.h"
#include "attribute.h"
#include "association.h"
#include "umlrole.h"
#include "umldoc.h"
#include "uml.h"
#include "codegenerator.h"

namespace Model_Utils {

bool isCloneable(Uml::Widget_Type type) {
    switch (type) {
    case Uml::wt_Actor:
    case Uml::wt_UseCase:
    case Uml::wt_Class:
    case Uml::wt_Interface:
    case Uml::wt_Enum:
    case Uml::wt_Datatype:
    case Uml::wt_Package:
    case Uml::wt_Component:
    case Uml::wt_Node:
    case Uml::wt_Artifact:
        return true;
    default:
        return false;
    }
}

UMLObject * findObjectInList(Uml::IDType id, const UMLObjectList& inList) {
    for (UMLObjectListIt oit(inList); oit.current(); ++oit) {
        UMLObject *obj = oit.current();
        if (obj->getID() == id)
            return obj;
        UMLObject *o;
        Uml::Object_Type t = obj->getBaseType();
        switch (t) {
        case Uml::ot_Folder:
        case Uml::ot_Package:
        case Uml::ot_Component:
            o = static_cast<UMLPackage*>(obj)->findObjectById(id);
            if (o)
                return o;
            break;
        case Uml::ot_Interface:
        case Uml::ot_Class:
        case Uml::ot_Enum:
        case Uml::ot_Entity:
            o = static_cast<UMLClassifier*>(obj)->findChildObjectById(id);
            if (o == NULL &&
                    (t == Uml::ot_Interface || t == Uml::ot_Class))
                o = ((UMLPackage*)obj)->findObjectById(id);
            if (o)
                return o;
            break;
        case Uml::ot_Association:
            {
                UMLAssociation *assoc = static_cast<UMLAssociation*>(obj);
                UMLRole *rA = assoc->getUMLRole(Uml::A);
                if (rA->getID() == id)
                    return rA;
                UMLRole *rB = assoc->getUMLRole(Uml::B);
                if (rB->getID() == id)
                    return rB;
            }
            break;
        default:
            break;
        }
    }
    return NULL;
}

UMLObject* findUMLObject(const UMLObjectList& inList,
                         const QString& inName,
                         Uml::Object_Type type /* = ot_UMLObject */,
                         UMLObject *currentObj /* = NULL */) {
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    QString name = inName;
    QStringList components;
#ifdef TRY_BUGFIX_120682
    // If we have a pointer or a reference in cpp we need to remove
    // the asterisks and ampersands in order to find the appropriate object
    if (UMLApp::app()->getActiveLanguage() == Uml::pl_Cpp) {
        if (name.endsWith("*"))
            name.remove("*");
        else if (name.contains("&"))
            name.remove("&");
    }
#endif
    QString nameWithoutFirstPrefix;
    if (name.contains("::"))
        components = QStringList::split("::", name);
    else if (name.contains("."))
        components = QStringList::split(".", name);
    if (components.size() > 1) {
        name = components.front();
        components.pop_front();
        nameWithoutFirstPrefix = components.join("::");
    }
    if (currentObj) {
        UMLPackage *pkg = NULL;
        if (dynamic_cast<UMLClassifierListItem*>(currentObj)) {
            currentObj = static_cast<UMLObject*>(currentObj->parent());
        }
        pkg = dynamic_cast<UMLPackage*>(currentObj);
        if (pkg == NULL)
            pkg = currentObj->getUMLPackage();
        // Remember packages that we've seen - for avoiding cycles.
        UMLPackageList seenPkgs;
        for (; pkg; pkg = currentObj->getUMLPackage()) {
            if (nameWithoutFirstPrefix.isEmpty()) {
                if (caseSensitive) {
                    if (pkg->getName() == name)
                        return pkg;
                } else if (pkg->getName().lower() == name.lower()) {
                    return pkg;
                }
            }
            if (seenPkgs.findRef(pkg) != -1) {
                kError() << "findUMLObject(" << name << "): "
                    << "breaking out of cycle involving "
                    << pkg->getName() << endl;
                break;
            }
            seenPkgs.append(pkg);
            UMLObjectList objectsInCurrentScope = pkg->containedObjects();
            for (UMLObjectListIt oit(objectsInCurrentScope); oit.current(); ++oit) {
                UMLObject *obj = oit.current();
                if (caseSensitive) {
                    if (obj->getName() != name)
                        continue;
                } else if (obj->getName().lower() != name.lower()) {
                    continue;
                }
                Uml::Object_Type foundType = obj->getBaseType();
                if (nameWithoutFirstPrefix.isEmpty()) {
                    if (type != Uml::ot_UMLObject && type != foundType) {
                        kDebug() << "findUMLObject: type mismatch for "
                            << name << " (seeking type: "
                            << type << ", found type: "
                            << foundType << ")" << endl;
                        // Class, Interface, and Datatype are all Classifiers
                        // and are considered equivalent.
                        // The caller must be prepared to handle possible mismatches.
                        if ((type == Uml::ot_Class ||
                             type == Uml::ot_Interface ||
                             type == Uml::ot_Datatype) &&
                            (foundType == Uml::ot_Class ||
                             foundType == Uml::ot_Interface ||
                             foundType == Uml::ot_Datatype)) {
                            return obj;
                        }
                        continue;
                    }
                    return obj;
                }
                if (foundType != Uml::ot_Package &&
                    foundType != Uml::ot_Folder &&
                    foundType != Uml::ot_Class &&
                    foundType != Uml::ot_Interface &&
                    foundType != Uml::ot_Component) {
                    kDebug() << "findUMLObject: found \"" << name
                        << "\" is not a package (?)" << endl;
                    continue;
                }
                UMLPackage *pkg = static_cast<UMLPackage*>(obj);
                return findUMLObject( pkg->containedObjects(),
                                      nameWithoutFirstPrefix, type );
            }
            currentObj = pkg;
        }
    }
    for (UMLObjectListIt oit(inList); oit.current(); ++oit) {
        UMLObject *obj = oit.current();
        if (caseSensitive) {
            if (obj->getName() != name)
                continue;
        } else if (obj->getName().lower() != name.lower()) {
            continue;
        }
        Uml::Object_Type foundType = obj->getBaseType();
        if (nameWithoutFirstPrefix.isEmpty()) {
            if (type != Uml::ot_UMLObject && type != foundType) {
                kDebug() << "findUMLObject: type mismatch for "
                    << name << " (seeking type: "
                    << type << ", found type: "
                    << foundType << ")" << endl;
                continue;
            }
            return obj;
        }
        if (foundType != Uml::ot_Package &&
            foundType != Uml::ot_Folder &&
            foundType != Uml::ot_Class &&
            foundType != Uml::ot_Interface &&
            foundType != Uml::ot_Component) {
            kDebug() << "findUMLObject: found \"" << name
                << "\" is not a package (?)" << endl;
            continue;
        }
        UMLPackage *pkg = static_cast<UMLPackage*>(obj);
        return findUMLObject( pkg->containedObjects(),
                              nameWithoutFirstPrefix, type );
    }
    return NULL;
}

QString uniqObjectName(Uml::Object_Type type, UMLPackage *parentPkg, QString prefix) {
    QString currentName = prefix;
    if (currentName.isEmpty()) {
        if(type == Uml::ot_Class)
            currentName = i18n("new_class");
        else if(type == Uml::ot_Actor)
            currentName = i18n("new_actor");
        else if(type == Uml::ot_UseCase)
            currentName = i18n("new_usecase");
        else if(type == Uml::ot_Package)
            currentName = i18n("new_package");
        else if(type == Uml::ot_Component)
            currentName = i18n("new_component");
        else if(type == Uml::ot_Node)
            currentName = i18n("new_node");
        else if(type == Uml::ot_Artifact)
            currentName = i18n("new_artifact");
        else if(type == Uml::ot_Interface)
            currentName = i18n("new_interface");
        else if(type == Uml::ot_Datatype)
            currentName = i18n("new_datatype");
        else if(type == Uml::ot_Enum)
            currentName = i18n("new_enum");
        else if(type == Uml::ot_Entity)
            currentName = i18n("new_entity");
        else if(type == Uml::ot_Folder)
            currentName = i18n("new_folder");
        else if(type == Uml::ot_Association)
            currentName = i18n("new_association");
        else {
            currentName = i18n("new_object");
            kWarning() << "unknown object type in umldoc::uniqObjectName()" << endl;
        }
    }
    UMLDoc *doc = UMLApp::app()->getDocument();
    QString name = currentName;
    for (int number = 1; !doc->isUnique(name, parentPkg); number++)  {
        name = currentName + '_' + QString::number(number);
    }
    return name;
}

bool isCommonXMIAttribute( const QString &tag ) {
    bool retval = (Uml::tagEq(tag, "name") ||
                   Uml::tagEq(tag, "visibility") ||
                   Uml::tagEq(tag, "isRoot") ||
                   Uml::tagEq(tag, "isLeaf") ||
                   Uml::tagEq(tag, "isAbstract") ||
                   Uml::tagEq(tag, "isSpecification") ||
                   Uml::tagEq(tag, "isActive") ||
                   Uml::tagEq(tag, "namespace") ||
                   Uml::tagEq(tag, "ownerScope") ||
                   Uml::tagEq(tag, "ModelElement.stereotype") ||
                   Uml::tagEq(tag, "GeneralizableElement.generalization") ||
                   Uml::tagEq(tag, "specialization") ||   //NYI
                   Uml::tagEq(tag, "clientDependency") || //NYI
                   Uml::tagEq(tag, "supplierDependency")  //NYI
                  );
    return retval;
}

bool isCommonDataType(QString type) {
    CodeGenerator *gen = UMLApp::app()->getGenerator();
    if (gen == NULL)
        return false;
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    QStringList dataTypes = gen->defaultDatatypes();
    QStringList::Iterator end(dataTypes.end());
    for (QStringList::Iterator it = dataTypes.begin(); it != end; ++it) {
        if (caseSensitive) {
            if (type == *it)
                return true;
        } else if (type.lower() == (*it).lower()) {
            return true;
        }
    }
    return false;
}

bool isClassifierListitem(Uml::Object_Type type) {
    if (type == Uml::ot_Attribute ||
        type == Uml::ot_Operation ||
        type == Uml::ot_Template ||
        type == Uml::ot_EntityAttribute ||
        type == Uml::ot_EnumLiteral) {
        return true;
    } else {
        return false;
    }
}

Uml::Model_Type guessContainer(UMLObject *o) {
    Uml::Object_Type ot = o->getBaseType();
    if (ot == Uml::ot_Package && o->getStereotype() == "subsystem")
        return Uml::mt_Component;
    Uml::Model_Type mt = Uml::N_MODELTYPES;
    switch (ot) {
        case Uml::ot_Package:   // CHECK: packages may appear in other views?
        case Uml::ot_Interface:
        case Uml::ot_Datatype:
        case Uml::ot_Enum:
        case Uml::ot_Class:
        case Uml::ot_Attribute:
        case Uml::ot_Operation:
        case Uml::ot_EnumLiteral:
        case Uml::ot_Template:
            mt = Uml::mt_Logical;
            break;
        case Uml::ot_Actor:
        case Uml::ot_UseCase:
            mt = Uml::mt_UseCase;
            break;
        case Uml::ot_Component:
        case Uml::ot_Artifact:  // trouble: artifact can also appear at Deployment
            mt = Uml::mt_Component;
            break;
        case Uml::ot_Node:
            mt = Uml::mt_Deployment;
            break;
        case Uml::ot_Entity:
        case Uml::ot_EntityAttribute:
            mt = Uml::mt_EntityRelationship;
            break;
        case Uml::ot_Association:
            {
                UMLAssociation *assoc = static_cast<UMLAssociation*>(o);
                UMLDoc *umldoc = UMLApp::app()->getDocument();
                for (int r = Uml::A; r <= Uml::B; r++) {
                    UMLObject *roleObj = assoc->getObject((Uml::Role_Type)r);
                    if (roleObj == NULL) {
                        // Ouch! we have been called while types are not yet resolved
                        return Uml::N_MODELTYPES;
                    }
                    UMLPackage *pkg = roleObj->getUMLPackage();
                    if (pkg) {
                        while (pkg->getUMLPackage()) {  // wind back to root
                            pkg = pkg->getUMLPackage();
                        }
                        const Uml::Model_Type m = umldoc->rootFolderType(pkg);
                        if (m != Uml::N_MODELTYPES)
                            return m;
                    }
                    mt = guessContainer(roleObj);
                    if (mt != Uml::mt_Logical)
                        break;
                }
            }
            break;
        default:
            break;
    }
    return mt;
}

int stringToDirection(QString input, Uml::Parameter_Direction & result) {
    QRegExp dirx("^(in|out|inout)");
    int pos = dirx.search(input);
    if (pos == -1)
        return 0;
    const QString& dirStr = dirx.capturedTexts().first();
    uint dirLen = dirStr.length();
    if (input.length() > dirLen && !input[dirLen].isSpace())
        return 0;       // no match after all.
    if (dirStr == "out")
        result = Uml::pd_Out;
    else if (dirStr == "inout")
        result = Uml::pd_InOut;
    else
        result = Uml::pd_In;
    return dirLen;
}

Parse_Status parseTemplate(QString t, NameAndType& nmTp, UMLClassifier *owningScope) {

    UMLDoc *pDoc = UMLApp::app()->getDocument();

    t = t.stripWhiteSpace();
    if (t.isEmpty())
        return PS_Empty;

    QStringList nameAndType = QStringList::split( QRegExp("\\s*:\\s*"), t);
    if (nameAndType.count() == 2) {
        UMLObject *pType = NULL;
        if (nameAndType[1] != "class") {
            pType = pDoc->findUMLObject(nameAndType[1], Uml::ot_UMLObject, owningScope);
            if (pType == NULL)
                return PS_Unknown_ArgType;
        }
        nmTp = NameAndType(nameAndType[0], pType);
    } else {
        nmTp = NameAndType(t, NULL);
    }
    return PS_OK;
}

Parse_Status parseAttribute(QString a, NameAndType& nmTp, UMLClassifier *owningScope,
                            Uml::Visibility *vis /* = 0 */) {
    UMLDoc *pDoc = UMLApp::app()->getDocument();

    a = a.simplifyWhiteSpace();
    if (a.isEmpty())
        return PS_Empty;

    int colonPos = a.find(':');
    if (colonPos < 0) {
        nmTp = NameAndType(a, NULL);
        return PS_OK;
    }
    QString name = a.left(colonPos).stripWhiteSpace();
    if (vis) {
        QRegExp mnemonicVis("^([\\+\\#\\-\\~] *)");
        int pos = mnemonicVis.search(name);
        if (pos == -1) {
            *vis = Uml::Visibility::Private;  // default value
        } else {
            QString caption = mnemonicVis.cap(1);
            QString strVis = caption.left(1);
            if (strVis == "+")
                *vis = Uml::Visibility::Public;
            else if (strVis == "#")
                *vis = Uml::Visibility::Protected;
            else if (strVis == "-")
                *vis = Uml::Visibility::Private;
            else
                *vis = Uml::Visibility::Implementation;
        }
        name.remove(mnemonicVis);
    }
    Uml::Parameter_Direction pd = Uml::pd_In;
    if (name.startsWith("in ")) {
        pd = Uml::pd_In;
        name = name.mid(3);
    } else if (name.startsWith("inout ")) {
        pd = Uml::pd_InOut;
        name = name.mid(6);
    } else if (name.startsWith("out ")) {
        pd = Uml::pd_Out;
        name = name.mid(4);
    }
    a = a.mid(colonPos + 1).stripWhiteSpace();
    if (a.isEmpty()) {
        nmTp = NameAndType(name, NULL, pd);
        return PS_OK;
    }
    QStringList typeAndInitialValue = QStringList::split( QRegExp("\\s*=\\s*"), a );
    const QString &type = typeAndInitialValue[0];
    UMLObject *pType = pDoc->findUMLObject(type, Uml::ot_UMLObject, owningScope);
    if (pType == NULL) {
        nmTp = NameAndType(name, NULL, pd);
        return PS_Unknown_ArgType;
    }
    QString initialValue;
    if (typeAndInitialValue.count() == 2) {
        initialValue = typeAndInitialValue[1];
    }
    nmTp = NameAndType(name, pType, pd, initialValue);
    return PS_OK;
}

Parse_Status parseOperation(QString m, OpDescriptor& desc, UMLClassifier *owningScope) {
    UMLDoc *pDoc = UMLApp::app()->getDocument();

    m = m.simplifyWhiteSpace();
    if (m.isEmpty())
        return PS_Empty;
    if (m.contains(QRegExp("operator *()"))) {
        // C++ special case: two sets of parentheses
        desc.m_name = "operator()";
        m.remove(QRegExp("operator *()"));
    } else {
        /**
         * The search pattern includes everything up to the opening parenthesis
         * because UML also permits non programming-language oriented designs
         * using narrative names, for example "check water temperature".
         */
        QRegExp beginningUpToOpenParenth( "^([^\\(]+)" );
        int pos = beginningUpToOpenParenth.search(m);
        if (pos == -1)
            return PS_Illegal_MethodName;
        desc.m_name = beginningUpToOpenParenth.cap(1);
    }
    desc.m_pReturnType = NULL;
    QRegExp pat = QRegExp("\\) *:(.*)$");
    int pos = pat.search(m);
    if (pos != -1) {  // return type is optional
        QString retType = pat.cap(1);
        retType = retType.stripWhiteSpace();
        if (retType != "void") {
            UMLObject *pRetType = owningScope->findTemplate(retType);
            if (pRetType == NULL) {
                pRetType = pDoc->findUMLObject(retType, Uml::ot_UMLObject, owningScope);
                if (pRetType == NULL)
                    return PS_Unknown_ReturnType;
            }
            desc.m_pReturnType = pRetType;
        }
    }
    // Remove possible empty parentheses ()
    m.remove( QRegExp("\\s*\\(\\s*\\)") );
    desc.m_args.clear();
    pat = QRegExp( "\\((.*)\\)" );
    pos = pat.search(m);
    if (pos == -1)  // argument list is optional
        return PS_OK;
    QString arglist = pat.cap(1);
    arglist = arglist.stripWhiteSpace();
    if (arglist.isEmpty())
        return PS_OK;
    QStringList args = QStringList::split( QRegExp("\\s*,\\s*"), arglist);
    for (QStringList::Iterator lit = args.begin(); lit != args.end(); ++lit) {
        NameAndType nmTp;
        Parse_Status ps = parseAttribute(*lit, nmTp, owningScope);
        if (ps)
            return ps;
        desc.m_args.append(nmTp);
    }
    return PS_OK;
}

QString psText(Parse_Status value) {
    const QString text[] = {
                               i18n("OK"), i18n("Empty"), i18n("Malformed argument"),
                               i18n("Unknown argument type"), i18n("Illegal method name"),
                               i18n("Unknown return type"), i18n("Unspecified error")
                           };
    return text[(unsigned) value];
}

QString progLangToString(Uml::Programming_Language pl) {
    switch (pl) {
        case Uml::pl_ActionScript:
            return "ActionScript";
        case Uml::pl_Ada:
            return "Ada";
        case Uml::pl_Cpp:
            return "C++";
        case Uml::pl_CSharp:
            return "C#";
        case Uml::pl_D:
            return "D";
        case Uml::pl_IDL:
            return "IDL";
        case Uml::pl_Java:
            return "Java";
        case Uml::pl_JavaScript:
            return "JavaScript";
        case Uml::pl_Pascal:
            return "Pascal";
        case Uml::pl_Perl:
            return "Perl";
        case Uml::pl_PHP:
            return "PHP";
        case Uml::pl_PHP5:
            return "PHP5";
        case Uml::pl_Python:
            return "Python";
        case Uml::pl_Ruby:
            return "Ruby";
        case Uml::pl_SQL:
            return "SQL";
        case Uml::pl_Tcl:
            return "Tcl";
        case Uml::pl_XMLSchema:
            return "XMLSchema";
        default:
            break;
    }
    return QString::null;
}

Uml::Programming_Language stringToProgLang(QString str) {
    if (str == "ActionScript")
        return Uml::pl_ActionScript;
    if (str == "Ada")
        return Uml::pl_Ada;
    if (str == "C++" || str == "Cpp")  // "Cpp" only for bkwd compatibility
        return Uml::pl_Cpp;
    if (str == "C#")
        return Uml::pl_CSharp;
    if (str == "D")
        return Uml::pl_D;
    if (str == "IDL")
        return Uml::pl_IDL;
    if (str == "Java")
        return Uml::pl_Java;
    if (str == "JavaScript")
        return Uml::pl_JavaScript;
    if (str == "Pascal")
        return Uml::pl_Pascal;
    if (str == "Perl")
        return Uml::pl_Perl;
    if (str == "PHP")
        return Uml::pl_PHP;
    if (str == "PHP5")
        return Uml::pl_PHP5;
    if (str == "Python")
        return Uml::pl_Python;
    if (str == "Ruby")
        return Uml::pl_Ruby;
    if (str == "SQL")
        return Uml::pl_SQL;
    if (str == "Tcl")
        return Uml::pl_Tcl;
    if (str == "XMLSchema")
        return Uml::pl_XMLSchema;
    return Uml::pl_Reserved;
}

bool typeIsRootView(Uml::ListView_Type type) {
    switch (type) {
        case Uml::lvt_View:
        case Uml::lvt_Logical_View:
        case Uml::lvt_UseCase_View:
        case Uml::lvt_Component_View:
        case Uml::lvt_Deployment_View:
        case Uml::lvt_EntityRelationship_Model:
            return true;
            break;
        default:
            break;
    }
    return false;
}

bool typeIsCanvasWidget(Uml::ListView_Type type) {
    switch (type) {
        case Uml::lvt_Actor:
        case Uml::lvt_UseCase:
        case Uml::lvt_Class:
        case Uml::lvt_Package:
        case Uml::lvt_Logical_Folder:
        case Uml::lvt_UseCase_Folder:
        case Uml::lvt_Component_Folder:
        case Uml::lvt_Deployment_Folder:
        case Uml::lvt_EntityRelationship_Folder:
        case Uml::lvt_Subsystem:
        case Uml::lvt_Component:
        case Uml::lvt_Node:
        case Uml::lvt_Artifact:
        case Uml::lvt_Interface:
        case Uml::lvt_Datatype:
        case Uml::lvt_Enum:
        case Uml::lvt_Entity:
            return true;
            break;
        default:
            break;
    }
    return false;
}

bool typeIsFolder(Uml::ListView_Type type) {
    if (typeIsRootView(type) ||
            type == Uml::lvt_Datatype_Folder ||
            type == Uml::lvt_Logical_Folder ||
            type == Uml::lvt_UseCase_Folder ||
            type == Uml::lvt_Component_Folder ||
            type == Uml::lvt_Deployment_Folder ||
            type == Uml::lvt_EntityRelationship_Folder) {
        return true;
    } else {
        return false;
    }
}

bool typeIsContainer(Uml::ListView_Type type) {
    if (typeIsFolder(type))
        return true;
    return (type == Uml::lvt_Package ||
            type == Uml::lvt_Subsystem ||
            type == Uml::lvt_Component);
}

bool typeIsClassifierList(Uml::ListView_Type type) {
    if (type == Uml::lvt_Attribute ||
        type == Uml::lvt_Operation ||
        type == Uml::lvt_Template ||
        type == Uml::lvt_EntityAttribute ||
        type == Uml::lvt_EnumLiteral) {
        return true;
    } else {
        return false;
    }
}

bool typeIsDiagram(Uml::ListView_Type type) {
    if (type == Uml::lvt_Class_Diagram ||
            type == Uml::lvt_Collaboration_Diagram ||
            type == Uml::lvt_State_Diagram ||
            type == Uml::lvt_Activity_Diagram ||
            type == Uml::lvt_Sequence_Diagram ||
            type == Uml::lvt_UseCase_Diagram ||
            type == Uml::lvt_Component_Diagram ||
            type == Uml::lvt_Deployment_Diagram ||
            type == Uml::lvt_EntityRelationship_Diagram) {
        return true;
    } else {
        return false;
    }
}

Uml::Model_Type convert_DT_MT(Uml::Diagram_Type dt) {
    Uml::Model_Type mt;
    switch (dt) {
        case Uml::dt_UseCase:
            mt = Uml::mt_UseCase;
            break;
        case Uml::dt_Collaboration:
        case Uml::dt_Class:
        case Uml::dt_Sequence:
        case Uml::dt_State:
        case Uml::dt_Activity:
            mt = Uml::mt_Logical;
            break;
        case Uml::dt_Component:
            mt = Uml::mt_Component;
            break;
        case Uml::dt_Deployment:
            mt = Uml::mt_Deployment;
            break;
        case Uml::dt_EntityRelationship:
            mt = Uml::mt_EntityRelationship;
            break;
        default:
            kError() << "Model_Utils::convert_DT_MT: illegal input value " << dt << endl;
            mt = Uml::N_MODELTYPES;
            break;
    }
    return mt;
}

Uml::ListView_Type convert_MT_LVT(Uml::Model_Type mt) {
    Uml::ListView_Type lvt = Uml::lvt_Unknown;
    switch (mt) {
        case Uml::mt_Logical:
            lvt = Uml::lvt_Logical_View;
            break;
        case Uml::mt_UseCase:
            lvt = Uml::lvt_UseCase_View;
            break;
        case Uml::mt_Component:
            lvt = Uml::lvt_Component_View;
            break;
        case Uml::mt_Deployment:
            lvt = Uml::lvt_Deployment_View;
            break;
        case Uml::mt_EntityRelationship:
            lvt = Uml::lvt_EntityRelationship_Model;
            break;
        default:
            break;
    }
    return lvt;
}

Uml::Model_Type convert_LVT_MT(Uml::ListView_Type lvt) {
    Uml::Model_Type mt = Uml::N_MODELTYPES;
    switch (lvt) {
        case Uml::lvt_Logical_View:
            mt = Uml::mt_Logical;
            break;
        case Uml::lvt_UseCase_View:
            mt = Uml::mt_UseCase;
            break;
        case Uml::lvt_Component_View:
            mt = Uml::mt_Component;
            break;
        case Uml::lvt_Deployment_View:
            mt = Uml::mt_Deployment;
            break;
        case Uml::lvt_EntityRelationship_Model:
            mt = Uml::mt_EntityRelationship;
            break;
        default:
            break;
    }
    return mt;
}

Uml::ListView_Type convert_DT_LVT(Uml::Diagram_Type dt) {
    Uml::ListView_Type type =  Uml::lvt_Unknown;
    switch(dt) {
    case Uml::dt_UseCase:
        type = Uml::lvt_UseCase_Diagram;
        break;

    case Uml::dt_Class:
        type = Uml::lvt_Class_Diagram;
        break;

    case Uml::dt_Sequence:
        type = Uml::lvt_Sequence_Diagram;
        break;

    case Uml::dt_Collaboration:
        type = Uml::lvt_Collaboration_Diagram;
        break;

    case Uml::dt_State:
        type = Uml::lvt_State_Diagram;
        break;

    case Uml::dt_Activity:
        type = Uml::lvt_Activity_Diagram;
        break;

    case Uml::dt_Component:
        type = Uml::lvt_Component_Diagram;
        break;

    case Uml::dt_Deployment:
        type = Uml::lvt_Deployment_Diagram;
        break;

    case Uml::dt_EntityRelationship:
        type = Uml::lvt_EntityRelationship_Diagram;
        break;

    default:
        kWarning() << "convert_DT_LVT() called on unknown diagram type" << endl;
    }
    return type;
}

Uml::ListView_Type convert_OT_LVT(UMLObject *o) {
    Uml::Object_Type ot = o->getBaseType();
    Uml::ListView_Type type =  Uml::lvt_Unknown;
    switch(ot) {
    case Uml::ot_UseCase:
        type = Uml::lvt_UseCase;
        break;

    case Uml::ot_Actor:
        type = Uml::lvt_Actor;
        break;

    case Uml::ot_Class:
        type = Uml::lvt_Class;
        break;

    case Uml::ot_Package:
        type = Uml::lvt_Package;
        break;

    case Uml::ot_Folder:
        {
            UMLDoc *umldoc = UMLApp::app()->getDocument();
            UMLFolder *f = static_cast<UMLFolder*>(o);
            do {
                const Uml::Model_Type mt = umldoc->rootFolderType(f);
                if (mt != Uml::N_MODELTYPES) {
                    switch (mt) {
                        case Uml::mt_Logical:
                            type = Uml::lvt_Logical_Folder;
                            break;
                        case Uml::mt_UseCase:
                            type = Uml::lvt_UseCase_Folder;
                            break;
                        case Uml::mt_Component:
                            type = Uml::lvt_Component_Folder;
                            break;
                        case Uml::mt_Deployment:
                            type = Uml::lvt_Deployment_Folder;
                            break;
                        case Uml::mt_EntityRelationship:
                            type = Uml::lvt_EntityRelationship_Folder;
                            break;
                        default:
                            break;
                    }
                    return type;
                }
            } while ((f = static_cast<UMLFolder*>(f->getUMLPackage())) != NULL);
            kError() << "convert_OT_LVT(" << o->getName()
                << "): internal error - object is not properly nested in folder"
                << endl;
        }
        break;

    case Uml::ot_Component:
        type = Uml::lvt_Component;
        break;

    case Uml::ot_Node:
        type = Uml::lvt_Node;
        break;

    case Uml::ot_Artifact:
        type = Uml::lvt_Artifact;
        break;

    case Uml::ot_Interface:
        type = Uml::lvt_Interface;
        break;

    case Uml::ot_Datatype:
        type = Uml::lvt_Datatype;
        break;

    case Uml::ot_Enum:
        type = Uml::lvt_Enum;
        break;

    case Uml::ot_EnumLiteral:
        type = Uml::lvt_EnumLiteral;
        break;

    case Uml::ot_Entity:
        type = Uml::lvt_Entity;
        break;

    case Uml::ot_EntityAttribute:
        type = Uml::lvt_EntityAttribute;
        break;

    case Uml::ot_Attribute:
        type = Uml::lvt_Attribute;
        break;

    case Uml::ot_Operation:
        type = Uml::lvt_Operation;
        break;

    case Uml::ot_Template:
        type = Uml::lvt_Template;
        break;
    default:
        break;
    }
    return type;
}

Uml::Object_Type convert_LVT_OT(Uml::ListView_Type lvt) {
    Uml::Object_Type ot = (Uml::Object_Type)0;
    switch (lvt) {
    case Uml::lvt_UseCase:
        ot = Uml::ot_UseCase;
        break;

    case Uml::lvt_Actor:
        ot = Uml::ot_Actor;
        break;

    case Uml::lvt_Class:
        ot = Uml::ot_Class;
        break;

    case Uml::lvt_Package:
    case Uml::lvt_Subsystem:
        ot = Uml::ot_Package;
        break;

    case Uml::lvt_Component:
        ot = Uml::ot_Component;
        break;

    case Uml::lvt_Node:
        ot = Uml::ot_Node;
        break;

    case Uml::lvt_Artifact:
        ot = Uml::ot_Artifact;
        break;

    case Uml::lvt_Interface:
        ot = Uml::ot_Interface;
        break;

    case Uml::lvt_Datatype:
        ot = Uml::ot_Datatype;
        break;

    case Uml::lvt_Enum:
        ot = Uml::ot_Enum;
        break;

    case Uml::lvt_Entity:
        ot = Uml::ot_Entity;
        break;

    case Uml::lvt_EntityAttribute:
        ot = Uml::ot_EntityAttribute;
        break;

    case Uml::lvt_Attribute:
        ot = Uml::ot_Attribute;
        break;

    case Uml::lvt_Operation:
        ot = Uml::ot_Operation;
        break;

    case Uml::lvt_Template:
        ot = Uml::ot_Template;
        break;

    case Uml::lvt_EnumLiteral:
        ot = Uml::ot_EnumLiteral;
        break;

    default:
        if (typeIsFolder(lvt))
            ot = Uml::ot_Folder;
        break;
    }
    return ot;
}

Uml::Icon_Type convert_LVT_IT(Uml::ListView_Type lvt) {
    Uml::Icon_Type icon = Uml::it_Home;
    switch (lvt) {
        case Uml::lvt_UseCase_View:
        case Uml::lvt_UseCase_Folder:
            icon = Uml::it_Folder_Grey;
            break;
        case Uml::lvt_Logical_View:
        case Uml::lvt_Logical_Folder:
            icon = Uml::it_Folder_Green;
            break;
        case Uml::lvt_Datatype_Folder:
            icon = Uml::it_Folder_Orange;
            break;
        case Uml::lvt_Component_View:
        case Uml::lvt_Component_Folder:
            icon = Uml::it_Folder_Red;
            break;
        case Uml::lvt_Deployment_View:
        case Uml::lvt_Deployment_Folder:
            icon = Uml::it_Folder_Violet;
            break;
        case Uml::lvt_EntityRelationship_Model:
        case Uml::lvt_EntityRelationship_Folder:
            icon = Uml::it_Folder_Cyan;
            break;

        case Uml::lvt_Actor:
            icon = Uml::it_Actor;
            break;
        case Uml::lvt_UseCase:
            icon = Uml::it_UseCase;
            break;
        case Uml::lvt_Class:
            icon = Uml::it_Class;
            break;
        case Uml::lvt_Package:
            icon = Uml::it_Package;
            break;
        case Uml::lvt_Subsystem:
            icon = Uml::it_Subsystem;
            break;
        case Uml::lvt_Component:
            icon = Uml::it_Component;
            break;
        case Uml::lvt_Node:
            icon = Uml::it_Node;
            break;
        case Uml::lvt_Artifact:
            icon = Uml::it_Artifact;
            break;
        case Uml::lvt_Interface:
            icon = Uml::it_Interface;
            break;
        case Uml::lvt_Datatype:
            icon = Uml::it_Datatype;
            break;
        case Uml::lvt_Enum:
            icon = Uml::it_Enum;
            break;
        case Uml::lvt_Entity:
            icon = Uml::it_Entity;
            break;
        case Uml::lvt_Template:
            icon = Uml::it_Template;
            break;
        case Uml::lvt_Attribute:
            icon = Uml::it_Private_Attribute;
            break;
        case Uml::lvt_EntityAttribute:
            icon = Uml::it_Private_Attribute;
            break;
        case Uml::lvt_EnumLiteral:
            icon = Uml::it_Public_Attribute;
            break;
        case Uml::lvt_Operation:
            icon = Uml::it_Public_Method;
            break;

        case Uml::lvt_Class_Diagram:
            icon = Uml::it_Diagram_Class;
            break;
        case Uml::lvt_UseCase_Diagram:
            icon = Uml::it_Diagram_Usecase;
            break;
        case Uml::lvt_Sequence_Diagram:
            icon = Uml::it_Diagram_Sequence;
            break;
        case Uml::lvt_Collaboration_Diagram:
            icon = Uml::it_Diagram_Collaboration;
            break;
        case Uml::lvt_State_Diagram:
            icon = Uml::it_Diagram_State;
            break;
        case Uml::lvt_Activity_Diagram:
            icon = Uml::it_Diagram_Activity;
            break;
        case Uml::lvt_Component_Diagram:
            icon = Uml::it_Diagram_Component;
            break;
        case Uml::lvt_Deployment_Diagram:
            icon = Uml::it_Diagram_Deployment;
            break;
        case Uml::lvt_EntityRelationship_Diagram:
            icon = Uml::it_Diagram_EntityRelationship;
            break;

        default:
            break;
    }
    return icon;
}

Uml::Diagram_Type convert_LVT_DT(Uml::ListView_Type lvt) {
    Uml::Diagram_Type dt = Uml::dt_Undefined;
    switch (lvt) {
        case Uml::lvt_Class_Diagram:
            dt = Uml::dt_Class;
            break;
        case Uml::lvt_UseCase_Diagram:
            dt = Uml::dt_UseCase;
            break;
        case Uml::lvt_Sequence_Diagram:
            dt = Uml::dt_Sequence;
            break;
        case Uml::lvt_Collaboration_Diagram:
            dt = Uml::dt_Collaboration;
            break;
        case Uml::lvt_State_Diagram:
            dt = Uml::dt_State;
            break;
        case Uml::lvt_Activity_Diagram:
            dt = Uml::dt_Activity;
            break;
        case Uml::lvt_Component_Diagram:
            dt = Uml::dt_Component;
            break;
        case Uml::lvt_Deployment_Diagram:
            dt = Uml::dt_Deployment;
            break;
        case Uml::lvt_EntityRelationship_Diagram:
            dt = Uml::dt_EntityRelationship;
            break;
        default:
            break;
    }
    return dt;
}

Uml::Model_Type convert_OT_MT(Uml::Object_Type ot) {
    Uml::Model_Type mt = Uml::N_MODELTYPES;
    switch (ot) {
        case Uml::ot_Actor:
        case Uml::ot_UseCase:
            mt = Uml::mt_UseCase;
            break;
        case Uml::ot_Component:
        case Uml::ot_Artifact:
            mt = Uml::mt_Component;
            break;
        case Uml::ot_Node:
            mt = Uml::mt_Deployment;
            break;
        case Uml::ot_Entity:
        case Uml::ot_EntityAttribute:
            mt = Uml::mt_EntityRelationship;
            break;
        default:
            mt = Uml::mt_Logical;
            break;
    }
    return mt;
}

}  // namespace Model_Utils

