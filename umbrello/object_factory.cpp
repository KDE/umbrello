/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006-2012                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "object_factory.h"

// app includes
#include "debug_utils.h"
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
#include "category.h"
#include "association.h"
#include "umldoc.h"
#include "uml.h"
#include "uniqueconstraint.h"
#include "foreignkeyconstraint.h"
#include "checkconstraint.h"
#include "codegenerator.h"
#include "model_utils.h"
#include "uniqueid.h"
#include "cmds.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

// qt includes
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtGui/QApplication>

namespace Object_Factory {

Uml::IDType g_predefinedId = Uml::id_None;

/**
 * Control whether the createUMLObject() solicits a new unique ID for the
 * created object.
 * By default, unique ID generation is turned on.
 *
 * @param yesno   False turns UID generation off, true turns it on.
 */
void assignUniqueIdOnCreation(bool yesno)
{
    if (yesno)
        g_predefinedId = Uml::id_None;
    else
        g_predefinedId = Uml::id_Reserved;
}

/**
 * Return whether unique ID generation is on or off.
 */
bool assignUniqueIdOnCreation()
{
    return (g_predefinedId == Uml::id_None);
}

UMLObject* createNewUMLObject(UMLObject::ObjectType type, const QString &name,
                              UMLPackage *parentPkg)
{
    if (parentPkg == NULL) {
        uError() << name << ": parentPkg is NULL";
        return NULL;
    }
    UMLObject *o = NULL;
    switch (type) {
        case UMLObject::ot_Actor:
            o = new UMLActor(name, g_predefinedId);
            break;
        case UMLObject::ot_UseCase:
            o = new UMLUseCase(name, g_predefinedId);
            break;
        case UMLObject::ot_Class:
            o = new UMLClassifier(name, g_predefinedId);
            break;
        case UMLObject::ot_Package:
            o = new UMLPackage(name, g_predefinedId);
            break;
        case UMLObject::ot_Component:
            o = new UMLComponent(name, g_predefinedId);
            break;
        case UMLObject::ot_Node:
            o = new UMLNode(name, g_predefinedId);
            break;
        case UMLObject::ot_Artifact:
            o = new UMLArtifact(name, g_predefinedId);
            break;
        case UMLObject::ot_Interface: {
            UMLClassifier *c = new UMLClassifier(name, g_predefinedId);
            c->setBaseType(UMLObject::ot_Interface);
            o = c;
            break;
        }
        case UMLObject::ot_Datatype: {
            UMLClassifier *c = new UMLClassifier(name, g_predefinedId);
            c->setBaseType(UMLObject::ot_Datatype);
            o = c;
            break;
        }
        case UMLObject::ot_Enum:
            o = new UMLEnum(name, g_predefinedId);
            break;
        case UMLObject::ot_Entity:
            o = new UMLEntity(name, g_predefinedId);
            break;
        case UMLObject::ot_Folder:
            o = new UMLFolder(name, g_predefinedId);
            break;
        case UMLObject::ot_Category:
            o = new UMLCategory(name, g_predefinedId);
            break;
        default:
            uWarning() << "error unknown type: " << UMLObject::toString(type);
            return NULL;
    }
    o->setUMLPackage(parentPkg);

    // will be caled in cmdCreateUMLObject
    //parentPkg->addObject(o);

    UMLApp::app()->executeCommand(new Uml::CmdCreateUMLObject(o));
    qApp->processEvents();
    return o;
}

/**
 * Creates a UMLObject of the given type.
 *
 * @param type      The type of @ref UMLObject to create.
 * @param n         A name to give to the object (optional.)
 *                  If not given then an input dialog prompts
 *                  the user to supply a name.
 * @param parentPkg The object's parent package.
 * @param solicitNewName  Ask user for a different name if an object
 *                        of the given name already exists.
 *                        If set to false and the name already exists
 *                        then the existing object is returned.
 *                        The default is to ask for the new name.
 */
UMLObject* createUMLObject(UMLObject::ObjectType type, const QString &n,
                           UMLPackage *parentPkg /* = 0 */,
                           bool solicitNewName /* = true */)
{
    UMLDoc *doc = UMLApp::app()->document();
    if (parentPkg == NULL) {
        if (type == UMLObject::ot_Datatype) {
            parentPkg = doc->datatypeFolder();
        } else {
            Uml::ModelType mt = Model_Utils::convert_OT_MT(type);
            uDebug() << "Object_Factory::createUMLObject(" << n << "): "
                << "parentPkg is not set, assuming Model_Type " << mt.toString();
            parentPkg = doc->rootFolder(mt);
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
        name = KInputDialog::getText(i18nc("UMLObject name", "Name"), i18n("Enter name:"), name, &ok, (QWidget*)UMLApp::app());
        if (!ok) {
            return 0;
        }
        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."),
                               i18n("Invalid Name"));
            continue;
        }
        CodeGenerator *codegen = UMLApp::app()->generator();
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

UMLAttribute *createAttribute(UMLObject *parent, const QString& name, UMLObject *type)
{
    UMLAttribute *attr = new UMLAttribute(parent);
    attr->setName(name);
    attr->setType(type);
    if (g_predefinedId == Uml::id_None)
        attr->setID(UniqueID::gen());
    return attr;
}

UMLOperation *createOperation(UMLClassifier *parent, const QString& name)
{
    UMLOperation *op = new UMLOperation(parent, name, g_predefinedId);
    return op;
}

/**
 * Creates an operation, attribute, template, or enum literal
 * for the parent classifier.
 *
 * @param parent    The parent concept
 * @param type      The type to create
 * @return      Pointer to the UMLClassifierListItem created
 */
UMLClassifierListItem* createChildObject(UMLClassifier* parent, UMLObject::ObjectType type)
{
    UMLObject* returnObject = NULL;
    switch (type) {
    case UMLObject::ot_Attribute: {
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(parent);
            if (c && !c->isInterface())
                returnObject = c->createAttribute();
            break;
        }
    case UMLObject::ot_EntityAttribute: {
         UMLEntity *e = dynamic_cast<UMLEntity*>( parent );
         if ( e ) {
             returnObject = e->createAttribute();
         }
         break;
        }
    case UMLObject::ot_Operation: {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(parent);
            if (c)
                returnObject = c->createOperation();
            break;
        }
    case UMLObject::ot_Template: {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(parent);
            if (c)
                returnObject = c->createTemplate();
            break;
        }
    case UMLObject::ot_EnumLiteral: {
            UMLEnum* umlenum = dynamic_cast<UMLEnum*>(parent);
            if (umlenum) {
                returnObject = umlenum->createEnumLiteral();
            }
            break;
        }
    case UMLObject::ot_UniqueConstraint: {
            UMLEntity* umlentity = dynamic_cast<UMLEntity*>( parent );
            if ( umlentity ) {
                returnObject = umlentity->createUniqueConstraint();
            }
            break;
        }
    case UMLObject::ot_ForeignKeyConstraint: {
            UMLEntity* umlentity = dynamic_cast<UMLEntity*>( parent );
            if ( umlentity ) {
                returnObject = umlentity->createForeignKeyConstraint();
            }
            break;
        }
    case UMLObject::ot_CheckConstraint: {
            UMLEntity* umlentity = dynamic_cast<UMLEntity*>( parent );
            if ( umlentity ) {
                returnObject = umlentity->createCheckConstraint();
            }
            break;
        }
    default:
        uDebug() << "ERROR UMLDoc::createChildObject type:" << UMLObject::toString(type);
    }
    return static_cast<UMLClassifierListItem*>(returnObject);
}

/**
 * Make a new UMLObject according to the given XMI tag.
 * Used by loadFromXMI and clipboard paste.
 */
UMLObject* makeObjectFromXMI(const QString& xmiTag,
                             const QString& stereoID /* = QString() */)
{
    UMLObject* pObject = 0;
    if (UMLDoc::tagEq(xmiTag, "UseCase")) {
        pObject = new UMLUseCase();
    } else if (UMLDoc::tagEq(xmiTag, "Actor")) {
        pObject = new UMLActor();
    } else if (UMLDoc::tagEq(xmiTag, "Class")) {
        pObject = new UMLClassifier();
    } else if (UMLDoc::tagEq(xmiTag, "Package")) {
        if (!stereoID.isEmpty()) {
            UMLDoc *doc = UMLApp::app()->document();
            UMLObject *stereo = doc->findStereotypeById(STR2ID(stereoID));
            if (stereo && stereo->name() == "folder")
                pObject = new UMLFolder();
        }
        if (pObject == NULL)
            pObject = new UMLPackage();
    } else if (UMLDoc::tagEq(xmiTag, "Component")) {
        pObject = new UMLComponent();
    } else if (UMLDoc::tagEq(xmiTag, "Node")) {
        pObject = new UMLNode();
    } else if (UMLDoc::tagEq(xmiTag, "Artifact")) {
        pObject = new UMLArtifact();
    } else if (UMLDoc::tagEq(xmiTag, "Interface")) {
        UMLClassifier *c = new UMLClassifier();
        c->setBaseType(UMLObject::ot_Interface);
        pObject = c;
    } else if (UMLDoc::tagEq(xmiTag, "DataType") || UMLDoc::tagEq(xmiTag, "Primitive")
               || UMLDoc::tagEq(xmiTag, "Datatype")) {   // for bkwd compat.
        UMLClassifier *c = new UMLClassifier();
        c->setBaseType(UMLObject::ot_Datatype);
        pObject = c;
    } else if (UMLDoc::tagEq(xmiTag, "Enumeration") ||
               UMLDoc::tagEq(xmiTag, "Enum")) {   // for bkwd compat.
        pObject = new UMLEnum();
    } else if (UMLDoc::tagEq(xmiTag, "Entity")) {
        pObject = new UMLEntity();
    } else if (UMLDoc::tagEq(xmiTag, "Category")) {
        pObject = new UMLCategory();
    } else if (UMLDoc::tagEq(xmiTag, "Stereotype")) {
        pObject = new UMLStereotype();
    } else if (UMLDoc::tagEq(xmiTag, "Association") ||
               UMLDoc::tagEq(xmiTag, "AssociationClass")) {
        pObject = new UMLAssociation();
    } else if (UMLDoc::tagEq(xmiTag, "Generalization")) {
        pObject = new UMLAssociation(Uml::AssociationType::Generalization);
    } else if (UMLDoc::tagEq(xmiTag, "Realization") ||
               UMLDoc::tagEq(xmiTag, "Abstraction")) {
        pObject = new UMLAssociation(Uml::AssociationType::Realization);
    } else if (UMLDoc::tagEq(xmiTag, "Dependency")) {
        pObject = new UMLAssociation(Uml::AssociationType::Dependency);
    } else if (UMLDoc::tagEq(xmiTag, "Child2Category")) {
        pObject = new UMLAssociation(Uml::AssociationType::Child2Category);
    } else if (UMLDoc::tagEq(xmiTag, "Category2Parent")) {
        pObject = new UMLAssociation(Uml::AssociationType::Category2Parent);
    }

    return pObject;
}

}  // end namespace Object_Factory
