/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006-2007                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "object_factory.h"

// qt/kde includes
#include <qregexp.h>
#include <qstringlist.h>
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kinputdialog.h>

// app includes
#include "umlobject.h"
#include "umlpackagelist.h"
#include "package.h"
#include "folder.h"
#include "classifier.h"
#include "attribute.h"
#include "operation.h"
#include "enum.h"
#include "entity.h"
#include "actor.h"
#include "usecase.h"
#include "component.h"
#include "node.h"
#include "artifact.h"
#include "stereotype.h"
#include "association.h"
#include "umldoc.h"
#include "uml.h"
#include "codegenerator.h"
#include "model_utils.h"
#include "uniqueid.h"

namespace Object_Factory {

Uml::IDType g_predefinedId = Uml::id_None;

void assignUniqueIdOnCreation(bool yesno) {
    if (yesno)
        g_predefinedId = Uml::id_None;
    else
        g_predefinedId = Uml::id_Reserved;
}

bool assignUniqueIdOnCreation() {
    return (g_predefinedId == Uml::id_None);
}

UMLObject* createNewUMLObject(Uml::Object_Type type, const QString &name,
                              UMLPackage *parentPkg) {
    if (parentPkg == NULL) {
        kError() << "Object_Factory::createNewUMLObject(" << name
            << "): parentPkg is NULL" << endl;
        return NULL;
    }
    UMLObject *o = NULL;
    switch (type) {
        case Uml::ot_Actor:
            o = new UMLActor(name, g_predefinedId);
            break;
        case Uml::ot_UseCase:
            o = new UMLUseCase(name, g_predefinedId);
            break;
        case Uml::ot_Class:
            o = new UMLClassifier(name, g_predefinedId);
            break;
        case Uml::ot_Package:
            o = new UMLPackage(name, g_predefinedId);
            break;
        case Uml::ot_Component:
            o = new UMLComponent(name, g_predefinedId);
            break;
        case Uml::ot_Node:
            o = new UMLNode(name, g_predefinedId);
            break;
        case Uml::ot_Artifact:
            o = new UMLArtifact(name, g_predefinedId);
            break;
        case Uml::ot_Interface: {
            UMLClassifier *c = new UMLClassifier(name, g_predefinedId);
            c->setBaseType(Uml::ot_Interface);
            o = c;
            break;
        }
        case Uml::ot_Datatype: {
            UMLClassifier *c = new UMLClassifier(name, g_predefinedId);
            c->setBaseType(Uml::ot_Datatype);
            o = c;
            break;
        }
        case Uml::ot_Enum:
            o = new UMLEnum(name, g_predefinedId);
            break;
        case Uml::ot_Entity:
            o = new UMLEntity(name, g_predefinedId);
            break;
        case Uml::ot_Folder:
            o = new UMLFolder(name, g_predefinedId);
            break;
        default:
            kWarning() << "createNewUMLObject error unknown type: " << type << endl;
            return NULL;
    }
    o->setUMLPackage(parentPkg);
    UMLDoc *doc = UMLApp::app()->getDocument();
    parentPkg->addObject(o);
    doc->signalUMLObjectCreated(o);
    kapp->processEvents();
    return o;
}

UMLObject* createUMLObject(Uml::Object_Type type, const QString &n,
                           UMLPackage *parentPkg /* = NULL */,
                           bool solicitNewName /* = true */) {
    UMLDoc *doc = UMLApp::app()->getDocument();
    if (parentPkg == NULL) {
        if (type == Uml::ot_Datatype) {
            parentPkg = doc->getDatatypeFolder();
        } else {
            Uml::Model_Type mt = Model_Utils::convert_OT_MT(type);
            kDebug() << "Object_Factory::createUMLObject(" << n << "): "
                << "parentPkg is not set, assuming Model_Type " << mt << endl;
            parentPkg = doc->getRootFolder(mt);
        }
    }
    if (!n.isEmpty()) {
        UMLObject *o = doc->findUMLObject(n, type, parentPkg);
        if (o) {
            if (!solicitNewName)
                return o;
        } else {
            o = createNewUMLObject(type, n, parentPkg);
            return o;
        }
    }
    bool ok = false;
    QString name = Model_Utils::uniqObjectName(type, parentPkg, n);
    bool bValidNameEntered = false;
    do {
        name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), name, &ok, (QWidget*)UMLApp::app());
        if (!ok) {
            return 0;
        }
        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."),
                               i18n("Invalid Name"));
            continue;
        }
        CodeGenerator *codegen = UMLApp::app()->getGenerator();
        if (codegen != NULL && codegen->isReservedKeyword(name)) {
            KMessageBox::error(0, i18n("This is a reserved keyword for the language of the configured code generator."),
                               i18n("Reserved Keyword"));
            continue;
        }
        if (! doc->isUnique(name, parentPkg)) {
            KMessageBox::error(0, i18n("That name is already being used."),
                               i18n("Not a Unique Name"));
            continue;
        }
        bValidNameEntered = true;
    } while (bValidNameEntered == false);
    UMLObject *o = createNewUMLObject(type, name, parentPkg);
    return o;
}

UMLAttribute *createAttribute(UMLObject *parent, const QString& name, UMLObject *type) {
    UMLAttribute *attr = new UMLAttribute(parent);
    attr->setName(name);
    attr->setType(type);
    if (g_predefinedId == Uml::id_None)
        attr->setID(UniqueID::gen());
    return attr;
}

UMLOperation *createOperation(UMLClassifier *parent, const QString& name) {
    UMLOperation *op = new UMLOperation(parent, name, g_predefinedId);
    return op;
}

UMLClassifierListItem* createChildObject(UMLClassifier* parent, Uml::Object_Type type) {
    UMLObject* returnObject = NULL;
    switch (type) {
    case Uml::ot_Attribute:
    case Uml::ot_EntityAttribute: {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(parent);
            if (c && !c->isInterface())
                returnObject = c->createAttribute();
            break;
        }
    case Uml::ot_Operation: {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(parent);
            if (c)
                returnObject = c->createOperation();
            break;
        }
    case Uml::ot_Template: {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(parent);
            if (c)
                returnObject = c->createTemplate();
            break;
        }
    case Uml::ot_EnumLiteral: {
            UMLEnum* umlenum = dynamic_cast<UMLEnum*>(parent);
            if (umlenum) {
                returnObject = umlenum->createEnumLiteral();
            }
            break;
        }
    default:
        kDebug() << "ERROR UMLDoc::createChildObject type:" << type << endl;
    }
    return static_cast<UMLClassifierListItem*>(returnObject);
}

UMLObject* makeObjectFromXMI(const QString& xmiTag,
                             const QString& stereoID /* = QString::null */) {
    UMLObject* pObject = 0;
    if (Uml::tagEq(xmiTag, "UseCase")) {
        pObject = new UMLUseCase();
    } else if (Uml::tagEq(xmiTag, "Actor")) {
        pObject = new UMLActor();
    } else if (Uml::tagEq(xmiTag, "Class")) {
        pObject = new UMLClassifier();
    } else if (Uml::tagEq(xmiTag, "Package")) {
        if (!stereoID.isEmpty()) {
            UMLDoc *doc = UMLApp::app()->getDocument();
            UMLObject *stereo = doc->findStereotypeById(STR2ID(stereoID));
            if (stereo && stereo->getName() == "folder")
                pObject = new UMLFolder();
        }
        if (pObject == NULL)
            pObject = new UMLPackage();
    } else if (Uml::tagEq(xmiTag, "Component")) {
        pObject = new UMLComponent();
    } else if (Uml::tagEq(xmiTag, "Node")) {
        pObject = new UMLNode();
    } else if (Uml::tagEq(xmiTag, "Artifact")) {
        pObject = new UMLArtifact();
    } else if (Uml::tagEq(xmiTag, "Interface")) {
        UMLClassifier *c = new UMLClassifier();
        c->setBaseType(Uml::ot_Interface);
        pObject = c;
    } else if (Uml::tagEq(xmiTag, "DataType") || Uml::tagEq(xmiTag, "Primitive")
               || Uml::tagEq(xmiTag, "Datatype")) {   // for bkwd compat.
        UMLClassifier *c = new UMLClassifier();
        c->setBaseType(Uml::ot_Datatype);
        pObject = c;
    } else if (Uml::tagEq(xmiTag, "Enumeration") ||
               Uml::tagEq(xmiTag, "Enum")) {   // for bkwd compat.
        pObject = new UMLEnum();
    } else if (Uml::tagEq(xmiTag, "Entity")) {
        pObject = new UMLEntity();
    } else if (Uml::tagEq(xmiTag, "Stereotype")) {
        pObject = new UMLStereotype();
    } else if (Uml::tagEq(xmiTag, "Association") ||
               Uml::tagEq(xmiTag, "AssociationClass")) {
        pObject = new UMLAssociation();
    } else if (Uml::tagEq(xmiTag, "Generalization")) {
        pObject = new UMLAssociation(Uml::at_Generalization);
    } else if (Uml::tagEq(xmiTag, "Realization") ||
               Uml::tagEq(xmiTag, "Abstraction")) {
        pObject = new UMLAssociation(Uml::at_Realization);
    } else if (Uml::tagEq(xmiTag, "Dependency")) {
        pObject = new UMLAssociation(Uml::at_Dependency);
    }
    return pObject;
}

}  // end namespace Object_Factory

