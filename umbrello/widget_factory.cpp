/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widget_factory.h"

// qt/kde includes
#include <kdebug.h>

// app includes
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
#include "actorwidget.h"
#include "actor.h"
#include "usecasewidget.h"
#include "usecase.h"
#include "notewidget.h"
#include "boxwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "statewidget.h"
#include "forkjoinwidget.h"
#include "activitywidget.h"
#include "seqlinewidget.h"

namespace Widget_Factory {

UMLWidget *createWidget(UMLView *view, UMLObject *o) {
    QPoint pos = view->getPos();
    int y = pos.y();
    Uml::Diagram_Type diagramType = view->getType();
    Uml::Object_Type type = o->getBaseType();
    UMLWidget *newWidget = NULL;
    switch (type) {
    case Uml::ot_Actor:
        if (diagramType == Uml::dt_Sequence) {
            ObjectWidget *ow = new ObjectWidget(view, o, view->getLocalID());
            ow->setDrawAsActor(true);
            y = ow->topMargin();
            newWidget = ow;
        } else
            newWidget = new ActorWidget(view, static_cast<UMLActor*>(o));
        break;
    case Uml::ot_UseCase:
        newWidget = new UseCaseWidget(view, static_cast<UMLUseCase*>(o));
        break;
    case Uml::ot_Package:
        newWidget = new PackageWidget(view, static_cast<UMLPackage*>(o));
        break;
    case Uml::ot_Component:
        newWidget = new ComponentWidget(view, static_cast<UMLComponent*>(o));
        if (diagramType == Uml::dt_Deployment) {
            newWidget->setIsInstance(true);
        }
        break;
    case Uml::ot_Node:
        newWidget = new NodeWidget(view, static_cast<UMLNode*>(o));
        break;
    case Uml::ot_Artifact:
        newWidget = new ArtifactWidget(view, static_cast<UMLArtifact*>(o));
        break;
    case Uml::ot_Datatype:
        newWidget = new DatatypeWidget(view, static_cast<UMLClassifier*>(o));
        break;
    case Uml::ot_Enum:
        newWidget = new EnumWidget(view, static_cast<UMLEnum*>(o));
        break;
    case Uml::ot_Entity:
        newWidget = new EntityWidget(view, static_cast<UMLEntity*>(o));
        break;
    case Uml::ot_Interface:
        if (diagramType == Uml::dt_Sequence || diagramType == Uml::dt_Collaboration) {
            ObjectWidget *ow = new ObjectWidget(view, o, view->getLocalID() );
            if (diagramType == Uml::dt_Sequence) {
                y = ow->topMargin();
            }
            newWidget = ow;
        } else {
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            ClassifierWidget* interfaceWidget = new ClassifierWidget(view, c);
            if (diagramType == Uml::dt_Component || diagramType == Uml::dt_Deployment) {
                interfaceWidget->setDrawAsCircle(true);
            }
            newWidget = interfaceWidget;
        }
        break;
    case Uml::ot_Class:
        //see if we really want an object widget or class widget
        if (diagramType == Uml::dt_Class || diagramType == Uml::dt_Component) {
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            ClassifierWidget *cw = new ClassifierWidget(view, c);
            if (diagramType == Uml::dt_Component)
                cw->setDrawAsCircle(true);
            newWidget = cw;
        } else {
            ObjectWidget *ow = new ObjectWidget(view, o, view->getLocalID() );
            if (diagramType == Uml::dt_Sequence) {
                y = ow->topMargin();
            }
            newWidget = ow;
        }
        break;
    default:
        kWarning() << "trying to create an invalid widget" << endl;
    }

    if (newWidget) {
        newWidget->setX( pos.x() );
        newWidget->setY( y );
    }
    return newWidget;
}

bool validateObjType(Uml::Object_Type expected, UMLObject* &o, Uml::IDType id) {
    if (o == NULL) {
        kDebug() << "Widget_Factory::validateObjType: creating new object of type "
                 << expected << endl;
        QString artificialName = "LOST_" + ID2STR(id);
        o = Object_Factory::createUMLObject(expected, artificialName, NULL, false);
        if (o == NULL)
            return false;
        o->setID(id);
        UMLPackage *parentPkg = o->getUMLPackage();
        parentPkg->addObject(o);
        return true;
    }
    Uml::Object_Type actual = o->getBaseType();
    if (actual == expected)
        return true;
    kError() << "validateObjType(" << o->getName()
        << "): expected type " << expected << ", actual type "
        << actual << endl;
    return false;
}

UMLWidget* makeWidgetFromXMI(const QString& tag,
                             const QString& idStr, UMLView *view) {
    UMLWidget *widget = NULL;

        // Loading of widgets which do NOT represent any UMLObject,
        // just graphic stuff with no real model information
        //FIXME while boxes and texts are just diagram objects, activities and
        // states should be UMLObjects
    if (tag == "statewidget" || tag == "UML:StateWidget") {
        widget = new StateWidget(view, StateWidget::Normal, Uml::id_Reserved);
    } else if (tag == "notewidget" || tag == "UML:NoteWidget") {
        widget = new NoteWidget(view, Uml::id_Reserved);
    } else if (tag == "boxwidget") {
        widget = new BoxWidget(view, Uml::id_Reserved);
    } else if (tag == "floatingtext" || tag == "UML:FloatingTextWidget") {
        widget = new FloatingTextWidget(view, Uml::tr_Floating, "", Uml::id_Reserved);
    } else if (tag == "activitywidget" || tag == "UML:ActivityWidget") {
        widget = new ActivityWidget(view, ActivityWidget::Initial, Uml::id_Reserved);
    } else if (tag == "messagewidget") {
        widget = new MessageWidget(view, Uml::sequence_message_asynchronous, Uml::id_Reserved);
    } else if (tag == "forkjoin") {
        widget = new ForkJoinWidget(view, false, Uml::id_Reserved);
    } else {
        // Loading of widgets which represent an UMLObject

        // Find the UMLObject and create the Widget to represent it
        Uml::IDType id = STR2ID(idStr);
        UMLDoc *umldoc = UMLApp::app()->getDocument();
        UMLObject *o = umldoc->findObjectById(id);
        if (o == NULL) {
            kDebug() << "makeWidgetFromXMI: cannot find object with id "
                      << ID2STR(id) << endl;
        }

        if (tag == "actorwidget" || tag == "UML:ActorWidget") {
            if (validateObjType(Uml::ot_Actor, o, id))
                widget = new ActorWidget(view, static_cast<UMLActor*>(o));
        } else if (tag == "usecasewidget" || tag ==  "UML:UseCaseWidget") {
            if (validateObjType(Uml::ot_UseCase, o, id))
                widget = new UseCaseWidget(view, static_cast<UMLUseCase*>(o));
        } else if (tag == "classwidget" || tag == "UML:ClassWidget") {
            if (validateObjType(Uml::ot_Class, o, id))
                widget = new ClassifierWidget(view, static_cast<UMLClassifier*>(o));
        } else if (tag == "packagewidget") {
            if (validateObjType(Uml::ot_Package, o, id))
                widget = new PackageWidget(view, static_cast<UMLPackage*>(o));
        } else if (tag == "componentwidget") {
            if (validateObjType(Uml::ot_Component, o, id))
                widget = new ComponentWidget(view, static_cast<UMLComponent*>(o));
        } else if (tag == "nodewidget") {
            if (validateObjType(Uml::ot_Node, o, id))
                widget = new NodeWidget(view, static_cast<UMLNode*>(o));
        } else if (tag == "artifactwidget") {
            if (validateObjType(Uml::ot_Artifact, o, id))
                widget = new ArtifactWidget(view, static_cast<UMLArtifact*>(o));
        } else if (tag == "interfacewidget") {
            if (validateObjType(Uml::ot_Interface, o, id))
                widget = new ClassifierWidget(view, static_cast<UMLClassifier*>(o));
        } else if (tag == "datatypewidget") {
            if (validateObjType(Uml::ot_Datatype, o, id))
                widget = new DatatypeWidget(view, static_cast<UMLClassifier*>(o));
        } else if (tag == "enumwidget") {
            if (validateObjType(Uml::ot_Enum, o, id))
                widget = new EnumWidget(view, static_cast<UMLEnum*>(o));
        } else if (tag == "entitywidget") {
            if (validateObjType(Uml::ot_Entity, o, id))
                widget = new EntityWidget(view, static_cast<UMLEntity*>(o));
        } else if (tag == "objectwidget" || tag == "UML:ObjectWidget") {
            widget = new ObjectWidget(view, o );
        } else {
            kWarning() << "Trying to create an unknown widget:" << tag << endl;
        }
    }
    return widget;
}

}   // end namespace Widget_Factory

