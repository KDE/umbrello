/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widget_factory.h"

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
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
#include "cmds.h"

namespace Widget_Factory {

UMLWidget *createWidget(UMLScene *scene, UMLObject *o)
{
    QPoint pos = scene->getPos();
    int y = pos.y();
    Uml::DiagramType diagramType = scene->type();
    UMLObject::ObjectType type = o->baseType();
    UMLWidget *newWidget = NULL;
    switch (type) {
    case UMLObject::ot_Actor:
        if (diagramType == Uml::DiagramType::Sequence) {
            ObjectWidget *ow = new ObjectWidget(scene, o, scene->getLocalID());
            ow->setDrawAsActor(true);
            y = ow->topMargin();
            newWidget = ow;
        } else
            newWidget = new ActorWidget(scene, static_cast<UMLActor*>(o));
        break;
    case UMLObject::ot_UseCase:
        newWidget = new UseCaseWidget(scene, static_cast<UMLUseCase*>(o));
        break;
    case UMLObject::ot_Package:
        newWidget = new PackageWidget(scene, static_cast<UMLPackage*>(o));
        break;
    case UMLObject::ot_Component:
        newWidget = new ComponentWidget(scene, static_cast<UMLComponent*>(o));
        if (diagramType == Uml::DiagramType::Deployment) {
            newWidget->setIsInstance(true);
        }
        break;
    case UMLObject::ot_Node:
        newWidget = new NodeWidget(scene, static_cast<UMLNode*>(o));
        break;
    case UMLObject::ot_Artifact:
        newWidget = new ArtifactWidget(scene, static_cast<UMLArtifact*>(o));
        break;
    case UMLObject::ot_Datatype:
        newWidget = new DatatypeWidget(scene, static_cast<UMLClassifier*>(o));
        break;
    case UMLObject::ot_Enum:
        newWidget = new EnumWidget(scene, static_cast<UMLEnum*>(o));
        break;
    case UMLObject::ot_Entity:
        newWidget = new EntityWidget(scene, static_cast<UMLEntity*>(o));
        break;
    case UMLObject::ot_Interface:
        if (diagramType == Uml::DiagramType::Sequence || diagramType == Uml::DiagramType::Collaboration) {
            ObjectWidget *ow = new ObjectWidget(scene, o, scene->getLocalID() );
            if (diagramType == Uml::DiagramType::Sequence) {
                y = ow->topMargin();
            }
            newWidget = ow;
        } else {
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            ClassifierWidget* interfaceWidget = new ClassifierWidget(scene, c);
            if (diagramType == Uml::DiagramType::Component || diagramType == Uml::DiagramType::Deployment) {
                interfaceWidget->setDrawAsCircle(true);
            }
            newWidget = interfaceWidget;
        }
        break;
    case UMLObject::ot_Class:
        //see if we really want an object widget or class widget
        if (diagramType == Uml::DiagramType::Class || diagramType == Uml::DiagramType::Component) {
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            ClassifierWidget *cw = new ClassifierWidget(scene, c);
            if (diagramType == Uml::DiagramType::Component)
                cw->setDrawAsCircle(true);
            newWidget = cw;
        } else {
            ObjectWidget *ow = new ObjectWidget(scene, o, scene->getLocalID() );
            if (diagramType == Uml::DiagramType::Sequence) {
                y = ow->topMargin();
            }
            newWidget = ow;
        }
        break;
    case UMLObject::ot_Category:
        newWidget = new CategoryWidget(scene, static_cast<UMLCategory*>(o));
        break;
    default:
        uWarning() << "trying to create an invalid widget";
    }

    if (newWidget) {
        newWidget->setX( pos.x() );
        newWidget->setY( y );
    }

    return newWidget;
}

bool validateObjType(UMLObject::ObjectType expected, UMLObject* &o, Uml::IDType id)
{
    if (o == NULL) {
        uDebug() << "Widget_Factory::validateObjType: creating new object of type "
                 << expected << endl;
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

UMLWidget* makeWidgetFromXMI(const QString& tag,
                             const QString& idStr, UMLScene *scene)
{
    UMLWidget *widget = NULL;

        // Loading of widgets which do NOT represent any UMLObject,
        // just graphic stuff with no real model information
        //FIXME while boxes and texts are just diagram objects, activities and
        // states should be UMLObjects
    if (tag == "statewidget" || tag == "UML:StateWidget") {
        widget = new StateWidget(scene, StateWidget::Normal, Uml::id_Reserved);
    } else if (tag == "notewidget" || tag == "UML:NoteWidget") {
        widget = new NoteWidget(scene, NoteWidget::Normal, Uml::id_Reserved);
    } else if (tag == "boxwidget") {
        widget = new BoxWidget(scene, Uml::id_Reserved);
    } else if (tag == "floatingtext" || tag == "UML:FloatingTextWidget") {
        widget = new FloatingTextWidget(scene, Uml::TextRole::Floating, "", Uml::id_Reserved);
    } else if (tag == "activitywidget" || tag == "UML:ActivityWidget") {
        widget = new ActivityWidget(scene, ActivityWidget::Initial, Uml::id_Reserved);
    } else if (tag == "messagewidget") {
        widget = new MessageWidget(scene, Uml::sequence_message_asynchronous, Uml::id_Reserved);
    } else if (tag == "forkjoin") {
        widget = new ForkJoinWidget(scene, false, Uml::id_Reserved);
    } else if (tag == "preconditionwidget") {
        widget = new PreconditionWidget(scene, NULL, Uml::id_Reserved);
    } else if (tag == "combinedFragmentwidget") {
        widget = new CombinedFragmentWidget(scene, CombinedFragmentWidget::Ref, Uml::id_Reserved);
    } else if (tag == "signalwidget") {
        widget = new SignalWidget(scene, SignalWidget::Send,  Uml::id_Reserved);
    } else if (tag == "floatingdashlinewidget") {
        widget = new FloatingDashLineWidget(scene, Uml::id_Reserved);
    } else if (tag == "objectnodewidget") {
        widget = new ObjectNodeWidget(scene, ObjectNodeWidget::Normal, Uml::id_Reserved);
    } else if (tag == "regionwidget") {
        widget = new RegionWidget(scene, Uml::id_Reserved);
    } else if (tag == "pinwidget") {
        widget = new PinWidget(scene, NULL, Uml::id_Reserved);
    }
    else
    {
        // Loading of widgets which represent an UMLObject

        // Find the UMLObject and create the Widget to represent it
        Uml::IDType id = STR2ID(idStr);
        UMLDoc *umldoc = UMLApp::app()->document();
        UMLObject *o = umldoc->findObjectById(id);
        if (o == NULL) {
            uDebug() << "makeWidgetFromXMI: cannot find object with id "
                << ID2STR(id) << endl;
        }

        if (tag == "actorwidget" || tag == "UML:ActorWidget") {
            if (validateObjType(UMLObject::ot_Actor, o, id))
                widget = new ActorWidget(scene, static_cast<UMLActor*>(o));
        } else if (tag == "usecasewidget" || tag ==  "UML:UseCaseWidget") {
            if (validateObjType(UMLObject::ot_UseCase, o, id))
                widget = new UseCaseWidget(scene, static_cast<UMLUseCase*>(o));
        } else if (tag == "classwidget" || tag == "UML:ClassWidget") {
            if (validateObjType(UMLObject::ot_Class, o, id))
                widget = new ClassifierWidget(scene, static_cast<UMLClassifier*>(o));
        } else if (tag == "packagewidget") {
            if (validateObjType(UMLObject::ot_Package, o, id))
                widget = new PackageWidget(scene, static_cast<UMLPackage*>(o));
        } else if (tag == "componentwidget") {
            if (validateObjType(UMLObject::ot_Component, o, id))
                widget = new ComponentWidget(scene, static_cast<UMLComponent*>(o));
        } else if (tag == "nodewidget") {
            if (validateObjType(UMLObject::ot_Node, o, id))
                widget = new NodeWidget(scene, static_cast<UMLNode*>(o));
        } else if (tag == "artifactwidget") {
            if (validateObjType(UMLObject::ot_Artifact, o, id))
                widget = new ArtifactWidget(scene, static_cast<UMLArtifact*>(o));
        } else if (tag == "interfacewidget") {
            if (validateObjType(UMLObject::ot_Interface, o, id))
                widget = new ClassifierWidget(scene, static_cast<UMLClassifier*>(o));
        } else if (tag == "datatypewidget") {
            if (validateObjType(UMLObject::ot_Datatype, o, id))
                widget = new DatatypeWidget(scene, static_cast<UMLClassifier*>(o));
        } else if (tag == "enumwidget") {
            if (validateObjType(UMLObject::ot_Enum, o, id))
                widget = new EnumWidget(scene, static_cast<UMLEnum*>(o));
        } else if (tag == "entitywidget") {
            if (validateObjType(UMLObject::ot_Entity, o, id))
                widget = new EntityWidget(scene, static_cast<UMLEntity*>(o));
        } else if (tag == "categorywidget") {
            if (validateObjType(UMLObject::ot_Category, o, id))
                widget = new CategoryWidget(scene, static_cast<UMLCategory*>(o));
        } else if (tag == "objectwidget" || tag == "UML:ObjectWidget") {
            widget = new ObjectWidget(scene, o );
        } else {
            uWarning() << "Trying to create an unknown widget:" << tag;
        }
    }
    return widget;
}

}   // end namespace Widget_Factory
