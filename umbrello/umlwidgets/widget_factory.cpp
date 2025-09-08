/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "widget_factory.h"

// app includes
#include "activitywidget.h"
#include "umlactor.h"
#include "actorwidget.h"
#include "umlartifact.h"
#include "artifactwidget.h"
#include "associationwidget.h"
#include "boxwidget.h"
#include "umlcategory.h"
#include "categorywidget.h"
#include "umlclassifier.h"
#include "classifierwidget.h"
#include "cmds.h"
#include "combinedfragmentwidget.h"
#include "umlcomponent.h"
#include "componentwidget.h"
#include "datatypewidget.h"
#define DBG_SRC QStringLiteral("Widget_Factory")
#include "debug_utils.h"
#include "umlentity.h"
#include "entitywidget.h"
#include "umlenum.h"
#include "enumwidget.h"
#include "instance.h"
#include "floatingdashlinewidget.h"
#include "floatingtextwidget.h"
#include "umlfolder.h"
#include "forkjoinwidget.h"
#include "interfacewidget.h"
#include "messagewidget.h"
#include "node.h"
#include "nodewidget.h"
#include "notewidget.h"
#include "object_factory.h"
#include "objectnodewidget.h"
#include "objectwidget.h"
#include "package.h"
#include "packagewidget.h"
#include "pinwidget.h"
#include "port.h"
#include "portwidget.h"
#include "preconditionwidget.h"
#include "regionwidget.h"
#include "signalwidget.h"
#include "statewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "usecase.h"
#include "usecasewidget.h"

DEBUG_REGISTER(Widget_Factory)

namespace Widget_Factory {

/**
 * Create a UMLWidget in the given view and representing the given document object.
 */
UMLWidget *createWidget(UMLScene *scene, UMLObject *o)
{
    QPointF pos = scene->pos();
    int y = pos.y();
    Uml::DiagramType::Enum diagramType = scene->type();
    UMLObject::ObjectType type = o->baseType();
    UMLWidget  *newWidget = nullptr;
    switch (type) {
    case UMLObject::ot_Actor:
        if (diagramType == Uml::DiagramType::Sequence) {
            ObjectWidget *ow = new ObjectWidget(scene, o);
            ow->setDrawAsActor(true);
            y = ow->topMargin();
            newWidget = ow;
        } else
            newWidget = new ActorWidget(scene, o->asUMLActor());
        break;
    case UMLObject::ot_UseCase:
        newWidget = new UseCaseWidget(scene, o->asUMLUseCase());
        break;
    case UMLObject::ot_Folder:
        newWidget = new PackageWidget(scene, o->asUMLPackage());
        break;
    case UMLObject::ot_Package:
        newWidget = new ClassifierWidget(scene, o->asUMLPackage());
        break;
    case UMLObject::ot_Component:
        newWidget = new ComponentWidget(scene, o->asUMLComponent());
        if (diagramType == Uml::DiagramType::Deployment) {
            newWidget->setIsInstance(true);
        }
        break;
    case UMLObject::ot_Port:
        {
            newWidget = new PortWidget(scene, o->asUMLPort());
        }
        break;
    case UMLObject::ot_Node:
        newWidget = new NodeWidget(scene, o->asUMLNode());
        break;
    case UMLObject::ot_Artifact:
        newWidget = new ArtifactWidget(scene, o->asUMLArtifact());
        break;
    case UMLObject::ot_Datatype:
        newWidget = new DatatypeWidget(scene, o->asUMLClassifier());
        break;
    case UMLObject::ot_Enum:
        newWidget = new EnumWidget(scene, o->asUMLEnum());
        break;
    case UMLObject::ot_Entity:
        newWidget = new EntityWidget(scene, o->asUMLEntity());
        break;
    case UMLObject::ot_Interface:
        if (diagramType == Uml::DiagramType::Sequence || diagramType == Uml::DiagramType::Collaboration) {
            ObjectWidget *ow = new ObjectWidget(scene, o);
            if (diagramType == Uml::DiagramType::Sequence) {
                y = ow->topMargin();
            }
            newWidget = ow;
        } else {
            UMLClassifier *c = o->asUMLClassifier();
            InterfaceWidget* interfaceWidget = new InterfaceWidget(scene, c);
            if (diagramType == Uml::DiagramType::Component || diagramType == Uml::DiagramType::Deployment) {
                interfaceWidget->setDrawAsCircle(true);
            }
            newWidget = interfaceWidget;
        }
        break;
    case UMLObject::ot_Class:
        if (diagramType == Uml::DiagramType::Object)
            break;

        //see if we really want an object widget or class widget
        if (diagramType == Uml::DiagramType::Class || diagramType == Uml::DiagramType::Component) {
            UMLClassifier *c = o->asUMLClassifier();
            ClassifierWidget *cw = new ClassifierWidget(scene, c);
            if (diagramType == Uml::DiagramType::Component)
                cw->setDrawAsCircle(true);
            newWidget = cw;
        } else {
            ObjectWidget *ow = new ObjectWidget(scene, o);
            if (diagramType == Uml::DiagramType::Sequence) {
                y = ow->topMargin();
            }
            newWidget = ow;
        }
        break;
    case UMLObject::ot_Instance:
        newWidget = new ClassifierWidget(scene, o->asUMLInstance());
        break;

    case UMLObject::ot_Category:
        newWidget = new CategoryWidget(scene, o->asUMLCategory());
        break;
    default:
        logWarn2("Widget_Factory trying to create an invalid widget (%1) for %2",
                 UMLObject::toString(type), o->name());
    }

    if (newWidget) {
        logDebug1("Widget_Factory::createWidget(%1)", newWidget->baseType());
        if (newWidget->baseType() != WidgetBase::wt_Pin &&
            newWidget->baseType() != WidgetBase::wt_Port) {
            newWidget->setX(pos.x());
            newWidget->setY(y);
        }
    }

    return newWidget;
}

bool validateObjType(UMLObject::ObjectType expected, UMLObject* &o, Uml::ID::Type id)
{
    if (o == nullptr) {
        logDebug1("Widget_Factory::validateObjType: creating new object of type %1",
                  expected);
        QString artificialName = QStringLiteral("LOST_") + Uml::ID::toString(id);
        o = Object_Factory::createUMLObject(expected, artificialName, nullptr, false);
        if (o == nullptr)
            return false;
        o->setID(id);
        UMLPackage *parentPkg = o->umlPackage();
        parentPkg->addObject(o);
        return true;
    }
    UMLObject::ObjectType actual = o->baseType();
    if (actual == expected)
        return true;
    logError3("Widget_Factory::validateObjType(%1): expected type %2, actual type %3",
              o->name(), UMLObject::toString(expected), UMLObject::toString(actual));
    return false;
}

/**
 * Create a UMLWidget according to the given XMI tag.
 */
UMLWidget* makeWidgetFromXMI(const QString& tag,
                             const QString& idStr, UMLScene *scene)
{
    UMLWidget  *widget = nullptr;

        // Loading of widgets which do NOT represent any UMLObject,
        // just graphic stuff with no real model information
        //FIXME while boxes and texts are just diagram objects, activities and
        // states should be UMLObjects
    if (tag == QStringLiteral("statewidget") || tag == QStringLiteral("UML:StateWidget")) {
        widget = new StateWidget(scene, StateWidget::Normal, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("notewidget") || tag == QStringLiteral("UML:NoteWidget")) {
        widget = new NoteWidget(scene, NoteWidget::Normal, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("boxwidget")) {
        widget = new BoxWidget(scene, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("floatingtext") || tag == QStringLiteral("UML:FloatingTextWidget")) {
        widget = new FloatingTextWidget(scene, Uml::TextRole::Floating, QString(), Uml::ID::Reserved);
    } else if (tag == QStringLiteral("activitywidget") || tag == QStringLiteral("UML:ActivityWidget")) {
        widget = new ActivityWidget(scene, ActivityWidget::Initial, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("messagewidget")) {
        widget = new MessageWidget(scene, Uml::SequenceMessage::Asynchronous, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("forkjoin")) {
        widget = new ForkJoinWidget(scene, Qt::Vertical, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("preconditionwidget")) {
        widget = new PreconditionWidget(scene, nullptr, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("combinedFragmentwidget")) {
        widget = new CombinedFragmentWidget(scene, CombinedFragmentWidget::Ref, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("signalwidget")) {
        widget = new SignalWidget(scene, SignalWidget::Send,  Uml::ID::Reserved);
    } else if (tag == QStringLiteral("floatingdashlinewidget")) {
        widget = new FloatingDashLineWidget(scene, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("objectnodewidget")) {
        widget = new ObjectNodeWidget(scene, ObjectNodeWidget::Normal, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("regionwidget")) {
        widget = new RegionWidget(scene, Uml::ID::Reserved);
    } else if (tag == QStringLiteral("pinwidget")) {
        widget = new PinWidget(scene, nullptr, Uml::ID::Reserved);
    }
    else
    {
        // Loading of widgets which represent a UMLObject

        // Find the UMLObject and create the Widget to represent it
        Uml::ID::Type id = Uml::ID::fromString(idStr);
        UMLDoc *umldoc = UMLApp::app()->document();
        UMLObject *o = umldoc->findObjectById(id);
        if (o == nullptr) {
            logError1("Widget_Factory::makeWidgetFromXMI: cannot find object with id %1",
                      Uml::ID::toString(id));
            delete widget;
            return nullptr;
        }

        if (tag == QStringLiteral("actorwidget") || tag == QStringLiteral("UML:ActorWidget")) {
            if (validateObjType(UMLObject::ot_Actor, o, id))
                widget = new ActorWidget(scene, o->asUMLActor());
        } else if (tag == QStringLiteral("usecasewidget") || tag ==  QStringLiteral("UML:UseCaseWidget")) {
            if (validateObjType(UMLObject::ot_UseCase, o, id))
                widget = new UseCaseWidget(scene, o->asUMLUseCase());
        } else if (tag == QStringLiteral("classwidget") ||
                   tag == QStringLiteral("UML:ClassWidget") || tag == QStringLiteral("UML:ConceptWidget")) {
            if (validateObjType(UMLObject::ot_Class, o, id) || validateObjType(UMLObject::ot_Package, o, id))
                widget = new ClassifierWidget(scene, o->asUMLClassifier());
        } else if (tag == QStringLiteral("packagewidget")) {
            if (validateObjType(UMLObject::ot_Package, o, id))
                widget = new ClassifierWidget(scene, o->asUMLPackage());
        } else if (tag == QStringLiteral("componentwidget")) {
            if (validateObjType(UMLObject::ot_Component, o, id))
                widget = new ComponentWidget(scene, o->asUMLComponent());
        } else if (tag == QStringLiteral("portwidget")) {
            if (validateObjType(UMLObject::ot_Port, o, id))
                widget = new PortWidget(scene, o->asUMLPort());
        } else if (tag == QStringLiteral("nodewidget")) {
            if (validateObjType(UMLObject::ot_Node, o, id))
                widget = new NodeWidget(scene, o->asUMLNode());
        } else if (tag == QStringLiteral("artifactwidget")) {
            if (validateObjType(UMLObject::ot_Artifact, o, id))
                widget = new ArtifactWidget(scene, o->asUMLArtifact());
        } else if (tag == QStringLiteral("interfacewidget")) {
            if (validateObjType(UMLObject::ot_Interface, o, id))
                widget = new InterfaceWidget(scene, o->asUMLClassifier());
        } else if (tag == QStringLiteral("datatypewidget")) {
            if (validateObjType(UMLObject::ot_Datatype, o, id))
                widget = new DatatypeWidget(scene, o->asUMLClassifier());
        } else if (tag == QStringLiteral("enumwidget")) {
            if (validateObjType(UMLObject::ot_Enum, o, id))
                widget = new EnumWidget(scene, o->asUMLEnum());
        } else if (tag == QStringLiteral("entitywidget")) {
            if (validateObjType(UMLObject::ot_Entity, o, id))
                widget = new EntityWidget(scene, o->asUMLEntity());
        } else if (tag == QStringLiteral("categorywidget")) {
            if (validateObjType(UMLObject::ot_Category, o, id))
                widget = new CategoryWidget(scene, o->asUMLCategory());
        } else if (tag == QStringLiteral("objectwidget") || tag == QStringLiteral("UML:ObjectWidget")) {
            widget = new ObjectWidget(scene, o);
        } else if(tag == QStringLiteral("instancewidget") || tag == QStringLiteral("UML:InstanceWidget")) {
            if (validateObjType(UMLObject::ot_Instance, o, id))
                widget = new ClassifierWidget(scene, o->asUMLInstance());
        }
        else {
            logWarn1("Widget_Factory::makeWidgetFromXMI: Trying to create an unknown widget %1", tag);
        }
    }
    return widget;
}

}   // end namespace Widget_Factory

