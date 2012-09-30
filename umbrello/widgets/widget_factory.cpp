/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widget_factory.h"

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "object_factory.h"
#include "floatingtextwidget.h"
#include "classifierwidget.h"
#include "classifier.h"
#include "packagewidget.h"
#include "package.h"
#include "folder.h"
#include "componentwidget.h"
#include "component.h"
#include "nodewidget.h"
#include "node.h"
#include "artifactwidget.h"
#include "artifact.h"
#include "datatypewidget.h"
#include "enumwidget.h"
#include "enum.h"
#include "entitywidget.h"
#include "entity.h"
#include "regionwidget.h"
#include "actorwidget.h"
#include "actor.h"
#include "usecasewidget.h"
#include "usecase.h"
#include "notewidget.h"
#include "boxwidget.h"
#include "category.h"
#include "associationwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "statewidget.h"
#include "forkjoinwidget.h"
#include "activitywidget.h"
#include "combinedfragmentwidget.h"
#include "seqlinewidget.h"
#include "preconditionwidget.h"
#include "signalwidget.h"
#include "floatingdashlinewidget.h"
#include "objectnodewidget.h"
#include "pinwidget.h"
#include "categorywidget.h"
#include "umlscene.h"

namespace Widget_Factory {

/**
 * Create a UMLWidget in the given view and representing the given document object.
 */
UMLWidget *createWidget(UMLScene *scene, UMLObject *o)
{
    UMLScenePoint pos = scene->pos();
    qreal y = pos.y();
    Uml::DiagramType diagramType = scene->type();
    UMLObject::ObjectType type = o->baseType();
    UMLWidget *newWidget = NULL;
    switch (type) {
    case UMLObject::ot_Actor:
        if (diagramType == Uml::DiagramType::Sequence) {
            ObjectWidget *ow = new ObjectWidget(o, scene->localID());
            ow->setDrawAsActor(true);
            y = ow->topMargin();
            newWidget = ow;
        } else
            newWidget = new ActorWidget(static_cast<UMLActor*>(o));
        break;
    case UMLObject::ot_UseCase:
        newWidget = new UseCaseWidget(static_cast<UMLUseCase*>(o));
        break;
    case UMLObject::ot_Package:
        newWidget = new PackageWidget(static_cast<UMLPackage*>(o));
        break;
    case UMLObject::ot_Component:
        newWidget = new ComponentWidget(static_cast<UMLComponent*>(o));
        if (diagramType == Uml::DiagramType::Deployment) {
            newWidget->setIsInstance(true);
        }
        break;
    case UMLObject::ot_Node:
        newWidget = new NodeWidget(static_cast<UMLNode*>(o));
        break;
    case UMLObject::ot_Artifact:
        newWidget = new ArtifactWidget(static_cast<UMLArtifact*>(o));
        break;
    case UMLObject::ot_Datatype:
        newWidget = new DatatypeWidget(static_cast<UMLClassifier*>(o));
        break;
    case UMLObject::ot_Enum:
        newWidget = new EnumWidget(static_cast<UMLEnum*>(o));
        break;
    case UMLObject::ot_Entity:
        newWidget = new EntityWidget(static_cast<UMLEntity*>(o));
        break;
    case UMLObject::ot_Interface:
        if (diagramType == Uml::DiagramType::Sequence || diagramType == Uml::DiagramType::Collaboration) {
            ObjectWidget *ow = new ObjectWidget(o, scene->localID());
            if (diagramType == Uml::DiagramType::Sequence) {
                y = ow->topMargin();
            }
            newWidget = ow;
        } else {
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            ClassifierWidget* interfaceWidget = new ClassifierWidget(c);
            if (diagramType == Uml::DiagramType::Component || diagramType == Uml::DiagramType::Deployment) {
                interfaceWidget->setVisualProperty(ClassifierWidget::DrawAsCircle, true);
            }
            newWidget = interfaceWidget;
        }
        break;
    case UMLObject::ot_Class:
        //see if we really want an object widget or class widget
        if (diagramType == Uml::DiagramType::Class || diagramType == Uml::DiagramType::Component) {
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            ClassifierWidget *cw = new ClassifierWidget(c);
            if (diagramType == Uml::DiagramType::Component)
                cw->setVisualProperty(ClassifierWidget::DrawAsCircle, true);
            newWidget = cw;
        } else {
            ObjectWidget *ow = new ObjectWidget(o, scene->localID() );
            if (diagramType == Uml::DiagramType::Sequence) {
                y = ow->topMargin();
            }
            newWidget = ow;
        }
        break;
    case UMLObject::ot_Category:
        newWidget = new CategoryWidget(static_cast<UMLCategory*>(o));
        break;
    default:
        uWarning() << "trying to create an invalid widget";
    }

    if (newWidget) {
        newWidget->setPos(pos.x(), y);
    }

    return newWidget;
}

/**
 * Create UMLObject with given object type and id.
 * @param expected   expected object type
 * @param o          reference to object
 * @param id         id of object
 * @return success status if valid object was created
 */
bool validateObjType(UMLObject::ObjectType expected, UMLObject* &o, Uml::IDType id)
{
    if (o == NULL) {
        uDebug() << "Widget_Factory::validateObjType: creating new object of type "
                 << expected;
        QString artificialName = "LOST_" + ID2STR(id);
        o = Object_Factory::createUMLObject(expected, artificialName, NULL, false);
        if (o == NULL)
            return false;
        o->setID(id);
        UMLPackage *parentPkg = o->umlPackage();
        parentPkg->addObject(o);
        return true;
    }
    UMLObject::ObjectType actual = o->baseType();
    if (actual == expected)
        return true;
    uError() << "validateObjType(" << o->name()
        << "): expected type " << UMLObject::toString(expected) << ", actual type "
        << UMLObject::toString(actual);
    return false;
}

/**
 * Create a UMLWidget according to the given XMI tag.
 */
UMLWidget* makeWidgetFromXMI(const QString& tag,
                             const QString& idStr, UMLScene *scene)
{
    UMLWidget *widget = 0;

    // Loading of widgets which do NOT represent any UMLObject,
    // just graphic stuff with no real model information
    //FIXME while boxes and texts are just diagram objects, activities and
    // states should be UMLObjects
    if (tag == "statewidget" || tag == "UML:StateWidget") {
        widget = new StateWidget(StateWidget::Normal, Uml::id_Reserved);
    } else if (tag == "notewidget" || tag == "UML:NoteWidget") {
        widget = new NoteWidget(NoteWidget::Normal, Uml::id_Reserved);
    } else if (tag == "boxwidget") {
        widget = new BoxWidget(Uml::id_Reserved);
    } else if (tag == "floatingtext" || tag == "UML:FloatingTextWidget") {
        widget = new FloatingTextWidget(Uml::TextRole::Floating, Uml::id_Reserved);
    } else if (tag == "activitywidget" || tag == "UML:ActivityWidget") {
        widget = new ActivityWidget(ActivityWidget::Initial, Uml::id_Reserved);
    } else if (tag == "messagewidget") {
        widget = new MessageWidget(Uml::sequence_message_asynchronous, Uml::id_Reserved);
    } else if (tag == "forkjoin") {
        widget = new ForkJoinWidget(Qt::Vertical, Uml::id_Reserved);
    } else if (tag == "preconditionwidget") {
        widget = new PreconditionWidget(NULL, Uml::id_Reserved);
    } else if (tag == "combinedFragmentwidget") {
        widget = new CombinedFragmentWidget(CombinedFragmentWidget::Ref, Uml::id_Reserved);
    } else if (tag == "signalwidget") {
        widget = new SignalWidget(SignalWidget::Send,  Uml::id_Reserved);
    } else if (tag == "floatingdashlinewidget") {
        widget = new FloatingDashLineWidget(0, Uml::id_Reserved);
    } else if (tag == "objectnodewidget") {
        widget = new ObjectNodeWidget(ObjectNodeWidget::Normal, Uml::id_Reserved);
    } else if (tag == "regionwidget") {
        widget = new RegionWidget(Uml::id_Reserved);
    } else if (tag == "pinwidget") {
        widget = new PinWidget(NULL, Uml::id_Reserved);
    }
    else  // Loading of widgets which represent an UMLObject
    {
        // Find the UMLObject and create the Widget to represent it
        Uml::IDType id = STR2ID(idStr);
        UMLDoc *umldoc = UMLApp::app()->document();
        UMLObject *o = umldoc->findObjectById(id);
        if (o == NULL) {
            uDebug() << "makeWidgetFromXMI: cannot find object with id "
                << ID2STR(id);
        }

        if (tag == "actorwidget" || tag == "UML:ActorWidget") {
            if (validateObjType(UMLObject::ot_Actor, o, id))
                widget = new ActorWidget(static_cast<UMLActor*>(o));
        } else if (tag == "usecasewidget" || tag ==  "UML:UseCaseWidget") {
            if (validateObjType(UMLObject::ot_UseCase, o, id))
                widget = new UseCaseWidget(static_cast<UMLUseCase*>(o));
        } else if (tag == "classwidget" || tag == "UML:ClassWidget") {
            if (validateObjType(UMLObject::ot_Class, o, id))
                widget = new ClassifierWidget(static_cast<UMLClassifier*>(o));
        } else if (tag == "packagewidget") {
            if (validateObjType(UMLObject::ot_Package, o, id))
                widget = new PackageWidget(static_cast<UMLPackage*>(o));
        } else if (tag == "componentwidget") {
            if (validateObjType(UMLObject::ot_Component, o, id))
                widget = new ComponentWidget(static_cast<UMLComponent*>(o));
        } else if (tag == "nodewidget") {
            if (validateObjType(UMLObject::ot_Node, o, id))
                widget = new NodeWidget(static_cast<UMLNode*>(o));
        } else if (tag == "artifactwidget") {
            if (validateObjType(UMLObject::ot_Artifact, o, id))
                widget = new ArtifactWidget(static_cast<UMLArtifact*>(o));
        } else if (tag == "interfacewidget") {
            if (validateObjType(UMLObject::ot_Interface, o, id))
                widget = new ClassifierWidget(static_cast<UMLClassifier*>(o));
        } else if (tag == "datatypewidget") {
            if (validateObjType(UMLObject::ot_Datatype, o, id))
                widget = new DatatypeWidget(static_cast<UMLClassifier*>(o));
        } else if (tag == "enumwidget") {
            if (validateObjType(UMLObject::ot_Enum, o, id))
                widget = new EnumWidget(static_cast<UMLEnum*>(o));
        } else if (tag == "entitywidget") {
            if (validateObjType(UMLObject::ot_Entity, o, id))
                widget = new EntityWidget(static_cast<UMLEntity*>(o));
        } else if (tag == "categorywidget") {
            if (validateObjType(UMLObject::ot_Category, o, id))
                widget = new CategoryWidget(static_cast<UMLCategory*>(o));
        } else if (tag == "objectwidget" || tag == "UML:ObjectWidget") {
            widget = new ObjectWidget(o);
        } else {
            uWarning() << "Trying to create an unknown widget:" << tag;
        }
    }

    if (widget) {
        uDebug() << "Added tag=" << tag << " / idStr=" << idStr;
        scene->addItem(widget);
    }
    return widget;
}

}   // end namespace Widget_Factory
