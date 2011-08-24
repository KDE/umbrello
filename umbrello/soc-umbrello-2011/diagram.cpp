/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#include "diagram.h"
 
#include "umlview.h"
#include "floatingtextwidget.h"
#include "umlscene.h"
#include "widget_factory.h"
#include "objectwidget.h"
#include "entity.h"
#include "umlclassifierlist.h"
#include "umlattributelist.h"
#include "umldoc.h"
#include "messagewidget.h"
#include "umlwidget.h"
#include "idchangelog.h"
#include "umlassociationlist.h"
#include "associationwidget.h"
#include "uml.h"
#include "cmds/widget/cmd_create_widget.h"
#include "uniqueid.h"
#include "soc-umbrello-2011/objectwidget.h"

using namespace Uml;

namespace QGV { 


Diagram::Diagram(UMLView *view) 
  : m_type(Uml::dt_Undefined),
  m_documentation(QString()),
  m_name(QString()),
  m_view(view),
  m_id(Uml::id_None),
  m_localId(Uml::id_None),
  m_doc(UMLApp::app()->document()),
  m_scene(m_view->scene()),
  m_activated(true)
{

  connect(m_doc, SIGNAL(sigObjectCreated(UMLObject *)),
            this, SLOT(slotObjectCreated(UMLObject *)));  
   
    
}

Diagram::~Diagram()
{
}

UMLView *Diagram::umlview() const
{
  return m_view;
}

Uml::Diagram_Type Diagram::typeDiagram() const
{
  return m_type;
}

QString Diagram::documentation() const
{
  return m_documentation;
}

QString Diagram::name() const
{
  return m_name;
}

void Diagram::setUmlView(UMLView* view)
{
  m_view = view;
}

void Diagram::setTypeDiagram(Uml::Diagram_Type t)
{
  m_type = t;
}

void Diagram::setDocumentation(QString doc)
{
  m_documentation = doc;
}

void Diagram::setName(QString name)
{
  m_name = name;
}

void Diagram::setPos(QPointF pos)
{
  m_pos = pos;
}

QPointF Diagram::pos() const
{
  return m_pos;
}

Uml::IDType Diagram::localId() const
{
  return m_localId;
}

void Diagram::setLocalId(Uml::IDType id)
{
  m_localId = id;
}

void Diagram::setId(Uml::IDType id)
{
  m_id = id;
}

Uml::IDType Diagram::id() const
{
  return m_id;
}

UMLWidgetList_new Diagram::selectedWidgets() const
{
    UMLWidgetList_new list;
    foreach(QGraphicsItem *item, m_view->scene()->selectedItems()) {
        UMLWidget *wid = dynamic_cast<UMLWidget*>(item);
        if(wid) {
            list << wid;
        }
    }
    return list;
}


int Diagram::selectCount(bool filterText) const
{
    if (!filterText)
        return selectedWidgets().count();
    int counter = 0;
    UMLWidget * temp = 0;
    foreach(temp, selectedWidgets()) {
        if (temp->widget_type() == Uml::wt_Text) {
            const FloatingTextWidget *ft = dynamic_cast<const FloatingTextWidget*>(temp);
            if (ft->textRole() == Uml::tr_Floating)
                counter++;
        } else {
            counter++;
        }
    }
    return counter;

}

void Diagram::slotObjectCreated(UMLObject *obj)
{

    UMLWidget* newWidget = Widget_Factory::create_Widget(UMLApp::app()->current_View(), obj);


    if (newWidget == NULL)
        return;

    newWidget->setVisible(true);
    newWidget->setActive(true);
    //newWidget->setFont(getFont());
    //newWidget->slotColorChanged(getID());
    //newWidget->slotLineWidthChanged(getID());
    //newWidget->updateComponentSize();

    if (m_view->diagram()->typeDiagram() == Uml::dt_Sequence) {
        // Set proper position on the sequence line widget which is
        // attached to the object widget.
        ObjectWidget *ow = dynamic_cast<ObjectWidget*>(newWidget);
        //if (ow)
            //ow->moveEvent(NULL);
    }

    m_widgetlist.append(newWidget);
    
    m_scene->addItem(newWidget);

    switch (obj->baseType()) {
      case Uml::ot_Actor:
      case Uml::ot_UseCase:
      case Uml::ot_Class:
      case Uml::ot_Package:
      case Uml::ot_Component:
      case Uml::ot_Node:
      case Uml::ot_Artifact:
      case Uml::ot_Interface:
      case Uml::ot_Enum:
      case Uml::ot_Entity:
      case Uml::ot_Datatype:
      case Uml::ot_Category:
        //createAutoAssociations(newWidget);
        // We need to invoke createAutoAttributeAssociations()
        // on all other widgets again because the newly created
        // widget might saturate some latent attribute assocs.
//         foreach(UMLWidget* w,  m_widgetlist) {
//             if (w != newWidget) {
//                 createAutoAttributeAssociations(w);
// 
//                 if (o->baseType() == ot_Entity)
//                     createAutoConstraintAssociations(w);
//             }
//         }
        break;
    default:
        break;
    }

}

void Diagram::slotObjectRemoved(UMLObject * o)
{
    m_paste = false;
    Uml::IDType id = o->id();
    UMLWidget* obj = 0;
    foreach(obj, m_widgetlist) {
        if (obj->id() != id)
            continue;
        removeWidget(obj);
        break;
    }
}

void Diagram::removeWidget(UMLWidget* o)
{
    if (!o)
        return;

    emit sigWidgetRemoved(o);

    //removeAssociations(o);

//     Uml::Widget_Type t = o->type();
//     if (type() == Uml::Diagram_Type::dt_Sequence && t == Uml::Widget_Type::wt_Object) {
//         //checkMessages(static_cast<ObjectWidget*>(o));
//     }
// 
//     if (t == Uml::Widget_Type::wt_Message) {
//         //m_messagewidgetlist.removeAll(static_cast<MessageWidget*>(o));
//     } else
//         //m_widgetlist.removeAll(o);
    
    o->deleteLater();
    m_doc->setModified(true);

}

void Diagram::setupNewWidget(UMLWidget *w)
{

    w->setX(m_pos.x());
    w->setY(m_pos.y());
    w->setVisible(true);
    w->setActive(true);
    //w->setFont(getFont());
    //w->slotColorChanged(getID());
    //w->slotLineWidthChanged(getID());
    m_widgetlist.append(w);
    m_doc->setModified();

    UMLApp::app()->executeCommand(new CmdCreateWidget(UMLApp::app()->current_View(), w));
}

UMLWidget *Diagram::getWidgetAt(const QPointF& p)
{
    qreal relativeSize = 10000;  // start with an arbitrary large number
    UMLWidget  *retObj = NULL;
    UMLWidgetListIt_new it(m_widgetlist);
    UMLWidget* obj = 0;
    foreach(obj,  m_widgetlist) {
        const qreal s = obj->onWidget(p);
        if (!s)
            continue;
        if (s < relativeSize) {
            relativeSize = s;
            retObj = obj;
        }
    }
    return retObj;

}

bool Diagram::isOpen() const
{
    return m_isopen;
}

void Diagram::setIsOpen(bool open)
{
  m_isopen = open;
}

QRectF Diagram::boundingRect() const
{
    return m_scene->itemsBoundingRect();
}

void Diagram::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*0*/)
{
  
}

void Diagram::clearSelected()
{
   selectedWidgets().clear();
  
}

void Diagram::setCreateObject(bool bCreate)
{
    m_createobject = bCreate;  
}
    

void Diagram::addObject(UMLObject *object)
{
    m_createobject = true;
    if (m_doc->addUMLObject(object))
        m_doc->signalUMLObjectCreated(object);  // m_bCreateObject is reset by slotObjectCreated()
    else
        m_createobject = false;  
}

bool Diagram::createObject() const
{
  return m_createobject;
}

void Diagram::activate()
{
    foreach(UMLWidget* obj , m_widgetlist) {
        if (obj->isActive() || obj->type() == Uml::wt_Message) {
            continue;
        }

       if (obj->isActive()) {
           obj->setVisible(true);
       } else {
           m_widgetlist.removeAll(obj);
           delete obj;
       }
    }//end foreach

    //Activate Message widgets
//     foreach(UMLWidget* obj , m_messagewidgetlist) {
//         if (obj->isActive())
//              continue;
// 
//         obj->setActive(m_doc->changeLog());
//         obj->setVisible(true);
// 
//     }

//     foreach(AssociationWidget* aw , m_associationlist) {
//         if (aw->activate()) {
//             aw->setVisibility(true);
//         } else {
//             m_associationlist.removeAll(aw);
//             delete aw;
//         }
//     }
}

void Diagram::activateAfterLoad(bool bUseLog)
{
    if (m_activated)
        return;
    if (bUseLog) {
        //beginPartialWidgetPaste();
    }

    //now activate them all
    activate();

    if (bUseLog) {
        //endPartialWidgetPaste();
    }
    //resizeCanvasToItems();
    m_view->setZoom(m_view->zoom());
    m_activated = true;
}

bool Diagram::addWidget(UMLWidget * pWidget , bool isPasteOperation /*false*/)
{
    if (!pWidget) {
        return false;
    }
    Uml::Widget_Type type = pWidget->widget_type();
    if (isPasteOperation) {
        //if (type == Uml::wt_Message)
            //m_widgetlist.append(static_cast<MessageWidget*>(pWidget));
        //else
        m_widgetlist.append(pWidget);
        return true;
    }
    if (!isPasteOperation && findWidget(pWidget->id())) {
        return false;
    }
    IDChangeLog * log = m_doc->changeLog();
    if (isPasteOperation && (!log || !m_pIDChangesLog)) {
        return false;
    }
    qreal wX = pWidget->pos().x();
    qreal wY = pWidget->pos().y();
    bool xIsOutOfRange = (wX <= 0 || wX >= FloatingTextWidget::restrictPositionMax);
    bool yIsOutOfRange = (wY <= 0 || wY >= FloatingTextWidget::restrictPositionMax);
    if (xIsOutOfRange || yIsOutOfRange) {
        QString name = pWidget->name();
        if (name.isEmpty()) {
            FloatingTextWidget *ft = dynamic_cast<FloatingTextWidget*>(pWidget);
            if (ft)
                name = ft->displayText();
        }
        if (xIsOutOfRange) {
            pWidget->setX(0);
            wX = 0;
        }
        if (yIsOutOfRange) {
            pWidget->setY(0);
            wY = 0;
        }
    }
    if (wX < m_pos.x())
        m_pos.setX(wX);
    if (wY < m_pos.y())
        m_pos.setY(wY);


    switch (type) {

    case wt_Class:
    case wt_Package:
    case wt_Component:
    case wt_Node:
    case wt_Artifact:
    case wt_Interface:
    case wt_Enum:
    case wt_Entity:
    case wt_Datatype:
    case wt_Actor:
    case wt_UseCase:
    case wt_Category: {
        Uml::IDType id = pWidget->id();
        Uml::IDType newID = log->findNewID(id);
        if (newID == Uml::id_None) {   // happens after a cut
            if (id == Uml::id_None)
                return false;
            newID = id; //don't stop paste
        } else
            pWidget->setId(newID);
        UMLObject * pObject = m_doc->findObjectById(newID);
        if (!pObject) {           
            return false;
        }
        pWidget->setUmlObject(pObject);
        if (findWidget(newID)) {
            //delete pWidget; 
            return true;
        }
        m_widgetlist.append(pWidget);
    }
    break;

    case wt_Message:
    case wt_Note:
    case wt_Box:
    case wt_Text:
    case wt_State:
    case wt_Activity:
    case wt_ObjectNode: {
        Uml::IDType newID = m_doc->assignNewID(pWidget->id());
        pWidget->setId(newID);
        if (type != wt_Message) {
            m_widgetlist.append(pWidget);
            return true;
        }
//         MessageWidget *pMessage = static_cast<MessageWidget *>(pWidget);
//         if (pMessage == NULL) {
//             return false;
//         }
//         ObjectWidget *objWidgetA = pMessage->getWidget(A);
//         ObjectWidget *objWidgetB = pMessage->getWidget(B);
//         Uml::IDType waID = objWidgetA->localID();
//         Uml::IDType wbID = objWidgetB->localID();
//         Uml::IDType newWAID = m_pIDChangesLog->findNewID(waID);
//         Uml::IDType newWBID = m_pIDChangesLog->findNewID(wbID);
/*        if (newWAID == Uml::id_None || newWBID == Uml::id_None) {
            return false;
        }
        objWidgetA->setLocalID(newWAID);
        objWidgetB->setLocalID(newWBID);
        FloatingTextWidget *ft = pMessage->floatingTextWidget();
        if (ft == NULL)
            qDebug() << "FloatingTextWidget of Message is NULL";
        else if (ft->id() == Uml::id_None)
            ft->setID(UniqueID::gen());
        else {
            Uml::IDType newTextID = m_doc->assignNewID(ft->id());
            ft->setID(newTextID);
        }
        m_messagewidgetlist.append(pMessage);*/
    }
    break;

    case wt_Object: {
        ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
        if (pObjectWidget == NULL) {
            return false;
        }
        Uml::IDType nNewLocalID = localId();
        Uml::IDType nOldLocalID = pObjectWidget->localId();
        m_pIDChangesLog->addIDChange(nOldLocalID, nNewLocalID);
        pObjectWidget->setLocalId(nNewLocalID);
        UMLObject *pObject = m_doc->findObjectById(pWidget->id());
        if (!pObject) {
            return false;
        }
        pWidget->setUmlObject(pObject);
        m_widgetlist.append(pWidget);
    }
    break;

    case wt_Precondition: {
        ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
        if (pObjectWidget == NULL) {
            return false;
        }
        Uml::IDType newID = log->findNewID(pWidget->id());
        if (newID == Uml::id_None) {
            return false;
        }
        pObjectWidget->setId(newID);
        Uml::IDType nNewLocalID = localId();
        Uml::IDType nOldLocalID = pObjectWidget->localId();
        m_pIDChangesLog->addIDChange(nOldLocalID, nNewLocalID);
        pObjectWidget->setLocalId(nNewLocalID);
        UMLObject *pObject = m_doc->findObjectById(newID);
        if (!pObject) {
            return false;
        }
        pWidget->setUmlObject(pObject);
        m_widgetlist.append(pWidget);
    }
    break;

    case wt_Pin:
    case wt_CombinedFragment:
    case wt_Signal: {
        ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
        if (pObjectWidget == NULL) {
            return false;
        }
        Uml::IDType newID = log->findNewID(pWidget->id());
        if (newID == Uml::id_None) {
            return false;
        }
        pObjectWidget->setId(newID);
        Uml::IDType nNewLocalID = localId();
        Uml::IDType nOldLocalID = pObjectWidget->localId();
        m_pIDChangesLog->addIDChange(nOldLocalID, nNewLocalID);
        pObjectWidget->setLocalId(nNewLocalID);
        UMLObject *pObject = m_doc->findObjectById(newID);
        if (!pObject) {
            return false;
        }
        pWidget->setUmlObject(pObject);
        m_widgetlist.append(pWidget);
    }
    break;

    default:
        return false;
        break;
    }

    return true;
}

UMLWidget * Diagram::findWidget(Uml::IDType id)
{
    foreach(UMLWidget* obj, m_widgetlist) {
        if (obj->type() == wt_Object) {
            if (static_cast<ObjectWidget *>(obj)->localId() == id)
                return obj;
        } else if (obj->id() == id) {
            return obj;
        }
    }

//     foreach(UMLWidget* obj, m_messagewidgetlist) {
//         if (obj->id() == id)
//             return obj;
//     }

    return 0;
}


}












