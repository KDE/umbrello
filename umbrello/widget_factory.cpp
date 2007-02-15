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
#include "classifier.h"
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
#include "objectwidget.h"
#include "messagewidget.h"
#include "statewidget.h"
#include "forkjoinwidget.h"
#include "activitywidget.h"
#include "combinedfragmentwidget.h"
#include "seqlinewidget.h"
#include "preconditionwidget.h"
#include "endoflifewidget.h"
#include "signalwidget.h"
#include "cmds.h"

namespace Widget_Factory {

UMLWidget *createWidget(UMLView *view, UMLObject *o) {
    QPoint pos = view->getPos();
    int y = pos.y();
    Uml::Diagram_Type diagramType = view->getType();
    Uml::Object_Type type = o->getBaseType();
    UMLDoc *doc = UMLApp::app()->getDocument();
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

    	UMLApp::app()->executeCommand(new Uml::cmdCreateWidget(view, newWidget));
    }

    return newWidget;
}

bool validateObjType(Uml::Object_Type expected, UMLObject *o) {
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
    if (tag == "statewidget" || tag == "notewidget" || tag == "boxwidget" ||
        tag == "floatingtext" || tag == "activitywidget" || tag == "forkjoin" || tag == "preconditionwidget" || tag == "endoflifewidget" ||
        tag == "combinedFragmentwidget"  || tag == "signalwidget"  || "floatingdashdinewidget" ||
            // tests for backward compatibility:
            tag == "UML:StateWidget" || tag == "UML:NoteWidget" ||
            tag=="UML:CombinedFragmentWidget" || tag == "UML:FloatingTextWidget" || tag == "UML:SignalWidget" || tag == "UML:ActivityWidget") {
        // Loading of widgets which do NOT represent any UMLObject, 
        // just graphic stuff with no real model information
        //FIXME while boxes and texts are just diagram objects, activities and
        // states should be UMLObjects
        if (tag == "statewidget"
                || tag == "UML:StateWidget") {         // for bkwd compatibility
            widget = new StateWidget(view, StateWidget::Normal, Uml::id_Reserved);
        } else if (tag == "notewidget"
                   || tag == "UML:NoteWidget") {          // for bkwd compatibility
            widget = new NoteWidget(view, Uml::id_Reserved);
        } else if (tag == "boxwidget") {
            widget = new BoxWidget(view, Uml::id_Reserved);
        } else if (tag == "floatingtext"
                   || tag == "UML:FloatingTextWidget") {  // for bkwd compatibility
            widget = new FloatingTextWidget(view, Uml::tr_Floating, "", Uml::id_Reserved);
        } else if (tag == "activitywidget"
                   || tag == "UML:ActivityWidget") {      // for bkwd compatibility
            widget = new ActivityWidget(view, ActivityWidget::Initial, Uml::id_Reserved);
        } else if (tag == "forkjoin") {
            widget = new ForkJoinWidget(view, false, Uml::id_Reserved);
        } else if (tag == "preconditionwidget") {
            widget = new PreconditionWidget(view, NULL, Uml::id_Reserved);
	} else if (tag == "endoflifewidget") {
            widget = new EndOfLifeWidget(view, NULL, Uml::id_Reserved);
	} else if (tag == "combinedFragmentwidget") {
            widget = new CombinedFragmentWidget(view, CombinedFragmentWidget::Ref, Uml::id_Reserved);
        } else if (tag == "signalwidget") {
            widget = new SignalWidget(view, SignalWidget::Send,  Uml::id_Reserved);
        }
    } else {
        // Find the UMLObject and create the Widget to represent it
        Uml::IDType id = STR2ID(idStr);
        UMLDoc *umldoc = UMLApp::app()->getDocument();
        UMLObject *o = umldoc->findObjectById(id);
        if (o == NULL) {
            kError() << "makeWidgetFromXMI: cannot find object with id "
                      << ID2STR(id) << endl;
            return NULL;
        }

        if (tag == "actorwidget"
                || tag == "UML:ActorWidget") {           // for bkwd compatibility
            if (validateObjType(Uml::ot_Actor, o))
                widget = new ActorWidget(view, static_cast<UMLActor*>(o));
        } else if (tag == "usecasewidget"
                   || tag == "UML:UseCaseWidget") {  // for bkwd compatibility
            if (validateObjType(Uml::ot_UseCase, o))
                widget = new UseCaseWidget(view, static_cast<UMLUseCase*>(o));
        } else if (tag == "classwidget"
                   || tag == "UML:ClassWidget"       // for bkwd compatibility
                   || tag == "UML:ConceptWidget") {  // for bkwd compatibility
            if (validateObjType(Uml::ot_Class, o))
                widget = new ClassifierWidget(view, static_cast<UMLClassifier*>(o));
        } else if (tag == "packagewidget") {
            if (validateObjType(Uml::ot_Package, o))
                widget = new PackageWidget(view, static_cast<UMLPackage*>(o));
        } else if (tag == "componentwidget") {
            if (validateObjType(Uml::ot_Component, o))
                widget = new ComponentWidget(view, static_cast<UMLComponent*>(o));
        } else if (tag == "nodewidget") {
            if (validateObjType(Uml::ot_Node, o))
                widget = new NodeWidget(view, static_cast<UMLNode*>(o));
        } else if (tag == "artifactwidget") {
            if (validateObjType(Uml::ot_Artifact, o))
                widget = new ArtifactWidget(view, static_cast<UMLArtifact*>(o));
        } else if (tag == "interfacewidget") {
            if (validateObjType(Uml::ot_Interface, o))
                widget = new ClassifierWidget(view, static_cast<UMLClassifier*>(o));
        } else if (tag == "datatypewidget") {
            if (validateObjType(Uml::ot_Datatype, o))
                widget = new DatatypeWidget(view, static_cast<UMLClassifier*>(o));
        } else if (tag == "enumwidget") {
            if (validateObjType(Uml::ot_Enum, o))
                widget = new EnumWidget(view, static_cast<UMLEnum*>(o));
        } else if (tag == "entitywidget") {
            if (validateObjType(Uml::ot_Entity, o))
                widget = new EntityWidget(view, static_cast<UMLEntity*>(o));
        } else if (tag == "objectwidget"
                   || tag == "UML:ObjectWidget") {  // for bkwd compatibility
            widget = new ObjectWidget(view, o );
        } else {
            kWarning() << "Trying to create an unknown widget:" << tag << endl;
        }
    }
    return widget;
}

}   // end namespace Widget_Factory

