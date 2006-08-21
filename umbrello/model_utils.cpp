/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2004-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "model_utils.h"

// qt/kde includes
#include <qregexp.h>
#include <qstringlist.h>
#include <klocale.h>
#include <kdebug.h>

// app includes
#include "inputdialog.h"
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

UMLObject * findObjectInList(Uml::IDType id, UMLObjectList inList) {
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

UMLObject* findUMLObject(UMLObjectList inList, QString name,
                         Uml::Object_Type type /* = ot_UMLObject */,
                         UMLObject *currentObj /* = NULL */) {
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
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
    if (type != Uml::ot_Datatype) {
        if (name.contains("::"))
            components = QStringList::split("::", name);
        else if (name.contains("."))
            components = QStringList::split(".", name);
    }
    QString nameWithoutFirstPrefix;
    if (components.size() > 1) {
        if (name.contains(QRegExp("[^\\w:\\.]"))) {
            // It's obviously a datatype.
            // Scope qualified datatypes live in the global scope.
            for (UMLObjectListIt oit(inList); oit.current(); ++oit) {
                UMLObject *obj = oit.current();
                if (caseSensitive) {
                    if (obj->getName() == name)
                        return obj;
                } else if (obj->getName().lower() == name.lower()) {
                    return obj;
                }
            }
            return NULL;
        }
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
                kdError() << "findUMLObject(" << name << "): "
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
                        kdDebug() << "findUMLObject: type mismatch for "
                            << name << " (seeking type: "
                            << type << ", found type: "
                            << foundType << ")" << endl;
                        continue;
                    }
                    return obj;
                }
                if (foundType != Uml::ot_Package &&
                    foundType != Uml::ot_Class &&
                    foundType != Uml::ot_Interface &&
                    foundType != Uml::ot_Component) {
                    kdDebug() << "findUMLObject: found \"" << name
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
                kdDebug() << "findUMLObject: type mismatch for "
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
            kdDebug() << "findUMLObject: found \"" << name
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
            kdWarning() << "unknown object type in umldoc::uniqObjectName()" << endl;
        }
    }
    UMLDoc *doc = UMLApp::app()->getDocument();
    QString name = currentName;
    for (int number = 1; !doc->isUnique(name, parentPkg); number++)  {
        name = currentName + "_" + QString::number(number);
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
    Uml::Model_Type mt = Uml::mt_Logical;
    switch (ot) {
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
            mt = Uml::mt_EntityRelationship;
            break;
        case Uml::ot_Association:
            {
                UMLAssociation *assoc = static_cast<UMLAssociation*>(o);
                for (int r = Uml::A; r <= Uml::B; r++) {
                    UMLObject *roleObj = assoc->getObject((Uml::Role_Type)r);
                    if (roleObj == NULL) {
                        // Ouch! we have been called while types are not yet resolved
                        break;
                    }
                    UMLPackage *pkg = roleObj->getUMLPackage();
                    if (pkg) {
                        while (pkg->getUMLPackage()) {  // wind back to root
                            pkg = pkg->getUMLPackage();
                        }
                        UMLDoc *umldoc = UMLApp::app()->getDocument();
                        for (int i = 0; i < Uml::N_MODELTYPES; i++) {
                            Uml::Model_Type m = (Uml::Model_Type)i;
                            if (pkg == umldoc->getRootFolder(m))
                                return m;
                        }
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

Parse_Status parseAttribute(QString a, NameAndType& nmTp, UMLClassifier *owningScope) {
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
    /**
     * The search pattern includes everything until the opening parenthesis
     * because UML also permits non programming-language oriented designs
     * using narrative names, for example "check water temperature".
     */
    QRegExp pat( "^([^\\(]+)" );
    int pos = pat.search(m);
    if (pos == -1)
        return PS_Illegal_MethodName;
    desc.m_name = pat.cap(1);
    desc.m_pReturnType = NULL;
    pat = QRegExp("\\) *:(.*)$");
    pos = pat.search(m);
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
        // case Uml::pl_Csharp:
            // return "C#";
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
    // if (str == "C#")
        // return Uml::pl_Csharp;
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

}  // namespace Model_Utils

