/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "object_factory.h"

// app includes
#include "actor.h"
#include "artifact.h"
#include "association.h"
#include "attribute.h"
#include "category.h"
#include "checkconstraint.h"
#include "classifier.h"
#include "cmds.h"
#include "codegenerator.h"
#include "component.h"
#include "datatype.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "enum.h"
#include "entity.h"
#include "folder.h"
#include "foreignkeyconstraint.h"
#include "instance.h"
#include "model_utils.h"
#include "node.h"
#include "package.h"
#include "port.h"
#include "operation.h"
#include "stereotype.h"
#include "usecase.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlpackagelist.h"
#include "uniqueconstraint.h"
#include "uniqueid.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes

// qt includes
#include <QApplication>
#include <QRegExp>
#include <QStringList>

namespace Object_Factory {

Uml::ID::Type g_predefinedId = Uml::ID::None;

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
        g_predefinedId = Uml::ID::None;
    else
        g_predefinedId = Uml::ID::Reserved;
}

/**
 * Return whether unique ID generation is on or off.
 */
bool assignUniqueIdOnCreation()
{
    return (g_predefinedId == Uml::ID::None);
}

/**
 * Creates a new UMLObject of the given type.
 * No check is made for whether the object named \a name already exists.
 * If the name shall be checked then use @ref createUMLObject.
 *
 * @param type      The type of @ref UMLObject to create.
 * @param name      Name to give to the object (mandatory.)
 * @param parentPkg The object's parent package.
 * @param undoable  Whether to insert the object creation into the undo stack (default: true.)
 */
UMLObject* createNewUMLObject(UMLObject::ObjectType type, const QString &name,
                              UMLPackage *parentPkg, bool undoable /* = true */)
{
    if (parentPkg == 0) {
        uError() << name << ": parentPkg is NULL";
        return 0;
    }
    QPointer<UMLObject> o = 0;
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
        case UMLObject::ot_Port:
            o = new UMLPort(name, g_predefinedId);
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
            UMLDatatype *c = new UMLDatatype(name, g_predefinedId);
            o = c;
            break;
        }
        case UMLObject::ot_Instance:
            o = new UMLInstance(name, g_predefinedId);
            if (parentPkg->isUMLClassifier()) {
                UMLClassifier *c = parentPkg->asUMLClassifier();
                o->asUMLInstance()->setClassifierCmd(c, false);
                parentPkg = c->umlPackage();
            }
            break;
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
        case UMLObject::ot_SubSystem: {
            o = new UMLPackage(name, g_predefinedId);
            o->setStereotypeCmd(QLatin1String("subsystem"));
            break;
        }
        default:
            uWarning() << "error unknown type: " << UMLObject::toString(type);
            return 0;
    }

    if (!undoable) {
        o->setUMLPackage(parentPkg);
        UMLApp::app()->document()->signalUMLObjectCreated(o);
        qApp->processEvents();
        return o;
    }

    // One user action can result in multiple commands when adding objects via
    // the toolbar. E.g. "create uml object" and "create widget". Wrap all
    // commands in one macro. When adding items via list view, this macro will
    // contain only the "create uml object" command.
    UMLApp::app()->beginMacro(i18n("Create UML object : %1", name));

    o->setUMLPackage(parentPkg);

    UMLApp::app()->executeCommand(new Uml::CmdCreateUMLObject(o));

    UMLApp::app()->document()->signalUMLObjectCreated(o);

    qApp->processEvents();

    UMLApp::app()->endMacro();
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
    if (parentPkg == 0) {
        if (type == UMLObject::ot_Datatype) {
            parentPkg = doc->datatypeFolder();
        } else {
            Uml::ModelType::Enum mt = Model_Utils::convert_OT_MT(type);
            uDebug() << "Object_Factory::createUMLObject(" << n << "): "
                << "parentPkg is not set, assuming Model_Type "
                << Uml::ModelType::toString(mt);
            parentPkg = doc->rootFolder(mt);
        }
    }
    if (!n.isEmpty()) {
        UMLObject *o = doc->findUMLObject(n, type, parentPkg);
        if (o == 0) {
            o = createNewUMLObject(type, n, parentPkg);
            return o;
        }
        if (!solicitNewName) {
            if (type == UMLObject::ot_UMLObject || o->baseType() == type) {
                uDebug() << o->name() << " already known - returning existing object";
                return o;
            }
            uWarning() << o->name() << " exists but is of type "
                       << UMLObject::toString(o->baseType())
                       << " - creating new object of type "
                       << UMLObject::toString(type);
            o = createNewUMLObject(type, n, parentPkg, false);
            return o;
        }
    }

    bool bValidNameEntered = false;
    QString name = Model_Utils::uniqObjectName(type, parentPkg, n);
    if (name == n) {
        bValidNameEntered = true;
    }

    while (bValidNameEntered == false) {
        bool ok = Dialog_Utils::askNewName(type, name);
        if (!ok) {
            return 0;
        }
        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."),
                               i18n("Invalid Name"));
            continue;
        }
        if (type != UMLObject::ot_Datatype) {
            CodeGenerator *codegen = UMLApp::app()->generator();
            if (codegen != 0 && codegen->isReservedKeyword(name)) {
                KMessageBox::error(0, i18n("This is a reserved keyword for the language of the configured code generator."),
                                   i18n("Reserved Keyword"));
                continue;
            }
        }
        if (! doc->isUnique(name, parentPkg) && type != UMLObject::ot_Instance) {
            KMessageBox::error(0, i18n("That name is already being used."),
                               i18n("Not a Unique Name"));
            continue;
        }
        bValidNameEntered = true;
    }

    UMLObject *o = createNewUMLObject(type, name, parentPkg);
    return o;
}

UMLAttribute *createAttribute(UMLObject *parent, const QString& name, UMLObject *type)
{
    UMLAttribute *attr = new UMLAttribute(parent);
    attr->setName(name);
    attr->setType(type);
    if (g_predefinedId == Uml::ID::None)
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
 * @param name      Optional name of object (skips creation dialog)
 * @return      Pointer to the UMLClassifierListItem created
 */
UMLClassifierListItem* createChildObject(UMLClassifier* parent, UMLObject::ObjectType type, const QString& name)
{
    UMLObject* returnObject = 0;
    switch (type) {
    case UMLObject::ot_Attribute: {
        UMLClassifier *c = parent->asUMLClassifier();
            if (c && !c->isInterface())
                returnObject = c->createAttribute(name);
            break;
        }
    case UMLObject::ot_EntityAttribute: {
         UMLEntity *e = parent->asUMLEntity();
         if (e) {
             returnObject = e->createAttribute(name);
         }
         break;
        }
    case UMLObject::ot_Operation: {
            UMLClassifier *c = parent->asUMLClassifier();
            if (c)
                returnObject = c->createOperation(name);
            break;
        }
    case UMLObject::ot_Template: {
            UMLClassifier *c = parent->asUMLClassifier();
            if (c)
                returnObject = c->createTemplate(name);
            break;
        }
    case UMLObject::ot_EnumLiteral: {
            UMLEnum* umlenum = parent->asUMLEnum();
            if (umlenum) {
                returnObject = umlenum->createEnumLiteral(name);
            }
            break;
        }
    case UMLObject::ot_UniqueConstraint: {
            UMLEntity* umlentity = parent->asUMLEntity();
            if (umlentity) {
                returnObject = umlentity->createUniqueConstraint(name);
            }
            break;
        }
    case UMLObject::ot_ForeignKeyConstraint: {
            UMLEntity* umlentity = parent->asUMLEntity();
            if (umlentity) {
                returnObject = umlentity->createForeignKeyConstraint(name);
            }
            break;
        }
    case UMLObject::ot_CheckConstraint: {
            UMLEntity* umlentity = parent->asUMLEntity();
            if (umlentity) {
                returnObject = umlentity->createCheckConstraint(name);
            }
            break;
        }
    default:
        break;
    }
    if (!returnObject) {
        uError() << "Object_Factory::createChildObject type:" << UMLObject::toString(type);
        return nullptr;
    }
    UMLClassifierListItem *ucli = returnObject->asUMLClassifierListItem();
    if (!ucli) {
        uError() << "Object_Factory::createChildObject internal: result is "
                 << "not a UMLClassifierListItem";
    }
    return ucli;
}

/**
 * Make a new UMLObject according to the given XMI tag.
 * Used by loadFromXMI1 and clipboard paste.
 */
UMLObject* makeObjectFromXMI(const QString& xmiTag,
                             const QString& stereoID /* = QString() */)
{
    UMLObject* pObject = 0;
    if (UMLDoc::tagEq(xmiTag, QLatin1String("UseCase"))) {
        pObject = new UMLUseCase();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Actor"))) {
        pObject = new UMLActor();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Class"))) {
        pObject = new UMLClassifier();
    } else if(UMLDoc::tagEq(xmiTag, QLatin1String("Instance"))) {
        pObject = new UMLInstance();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Package"))) {
        if (!stereoID.isEmpty()) {
            UMLDoc *doc = UMLApp::app()->document();
            UMLObject *stereo = doc->findStereotypeById(Uml::ID::fromString(stereoID));
            if (stereo && stereo->name() == QLatin1String("folder"))
                pObject = new UMLFolder();
        }
        if (pObject == 0)
            pObject = new UMLPackage();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Component"))) {
        pObject = new UMLComponent();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Port"))) {
        pObject = new UMLPort();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Node"))) {
        pObject = new UMLNode();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Artifact"))) {
        pObject = new UMLArtifact();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Interface"))) {
        UMLClassifier *c = new UMLClassifier();
        c->setBaseType(UMLObject::ot_Interface);
        pObject = c;
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("DataType"))
            || UMLDoc::tagEq(xmiTag, QLatin1String("Datatype"))   // for bkwd compat.
            || UMLDoc::tagEq(xmiTag, QLatin1String("Primitive"))
            || UMLDoc::tagEq(xmiTag, QLatin1String("PrimitiveType"))) {
        UMLDatatype *c = new UMLDatatype();
        pObject = c;
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Enumeration")) ||
               UMLDoc::tagEq(xmiTag, QLatin1String("Enum"))) {   // for bkwd compat.
        pObject = new UMLEnum();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Entity"))) {
        pObject = new UMLEntity();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Category"))) {
        pObject = new UMLCategory();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Stereotype"))) {
        pObject = new UMLStereotype();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Association")) ||
               UMLDoc::tagEq(xmiTag, QLatin1String("AssociationClass"))) {
        pObject = new UMLAssociation();
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Generalization")) ||
               UMLDoc::tagEq(xmiTag, QLatin1String("generalization"))) {
        pObject = new UMLAssociation(Uml::AssociationType::Generalization);
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Realization")) ||
               UMLDoc::tagEq(xmiTag, QLatin1String("Abstraction"))) {
        pObject = new UMLAssociation(Uml::AssociationType::Realization);
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Dependency"))) {
        pObject = new UMLAssociation(Uml::AssociationType::Dependency);
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Aggregation"))) {  // Embarcadero's Describe
        pObject = new UMLAssociation(Uml::AssociationType::Aggregation);
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Child2Category"))) {
        pObject = new UMLAssociation(Uml::AssociationType::Child2Category);
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Category2Parent"))) {
        pObject = new UMLAssociation(Uml::AssociationType::Category2Parent);
    }

    return pObject;
}

}  // end namespace Object_Factory
