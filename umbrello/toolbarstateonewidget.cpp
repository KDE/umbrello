/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstateonewidget.h"

// kde includes
#include <kdebug.h>

// local includes
#include "floatingtextwidget.h"
#include "pinwidget.h"
#include "preconditionwidget.h"
#include "endoflifewidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "activitywidget.h"
#include "umlwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "dialog_utils.h"

#include <klocale.h>
#include <kmessagebox.h>


using namespace Uml;

ToolBarStateOneWidget::ToolBarStateOneWidget(UMLView *umlView) : ToolBarStatePool(umlView) {
    m_umlView = umlView;
    m_firstObject = 0;
}

ToolBarStateOneWidget::~ToolBarStateOneWidget() {
}

void ToolBarStateOneWidget::init() {
    ToolBarStatePool::init();
}

void ToolBarStateOneWidget::cleanBeforeChange() {
    ToolBarStatePool::cleanBeforeChange();
}

void ToolBarStateOneWidget::mouseMove(QMouseEvent* ome) {
    ToolBarStatePool::mouseMove(ome);
}

void ToolBarStateOneWidget::slotWidgetRemoved(UMLWidget* widget) {
    ToolBarState::slotWidgetRemoved(widget);
}

void ToolBarStateOneWidget::setCurrentElement() {
    m_isObjectWidgetLine = false;
    ObjectWidget* objectWidgetLine = m_pUMLView->onWidgetLine(m_pMouseEvent->pos());
    if (objectWidgetLine) {
        setCurrentWidget(objectWidgetLine);
        m_isObjectWidgetLine = true;
	return;
    }

    UMLWidget *widget = m_pUMLView->testOnWidget(m_pMouseEvent->pos());
    if (widget) {
	kDebug()<<"setCurrentElement()!!!!!!!!!!!!!!!!!!!!!" <<endl;
        setCurrentWidget(widget);
        return;
    }
}

void ToolBarStateOneWidget::mouseReleaseWidget() {
    Uml::Widget_Type widgetType = getWidgetType();
    UMLWidget * widget = 0;

    if (widgetType == Uml::wt_Precondition) {
	m_firstObject = 0;
    }
    if (widgetType == Uml::wt_Pin) {
	kDebug()<<"mouseReleaseWidget()!!!!!!!!!!!!!!!!!!!!!" <<endl;
	m_firstObject = 0;
    }
    else if (widgetType == Uml::wt_EndOfLife) {
	//test if there isn't an endoflife widget on the selected widget yet
	UMLWidgetListIt w_it(m_umlView->getWidgetList());
	while( ( widget = w_it.current() ) ) {
            ++w_it;
            if (widget->getBaseType() == Uml::wt_EndOfLife) {
		if (dynamic_cast<EndOfLifeWidget*>(widget)->m_pOw[Uml::A]->getID() == getCurrentWidget()->getID())
		    return;
		else
		     m_firstObject = 0;
	    }
	}
    }
kDebug()<<"avant avant if !!!!!!!!!!!!!!!!!!!!!" <<getCurrentWidget()->getBaseType() <<endl;
    if (m_pMouseEvent->button() != Qt::LeftButton ||(
                getCurrentWidget()->getBaseType() != Uml::wt_Object &&
                getCurrentWidget()->getBaseType() != Uml::wt_Activity)) {
	kDebug()<<"avant return if !!!!!!!!!!!!!!!!!!!!!" <<Uml::wt_Activity <<endl;
        return;
    }
	kDebug()<<"avant if !!!!!!!!!!!!!!!!!!!!!" <<endl;
    if (!m_firstObject && widgetType == Uml::wt_Pin) {
	kDebug()<<"dans if !!!!!!!!!!!!!!!!!!!!!" <<endl;
        setWidget(getCurrentWidget());
	return ;
    } 

    if (!m_isObjectWidgetLine && !m_firstObject) {
	return;
    }
    
    if (!m_firstObject) {
        setWidget(getCurrentWidget());
    } 

}

void ToolBarStateOneWidget::mouseReleaseEmpty() {
}

void ToolBarStateOneWidget::setWidget(UMLWidget* firstObject) {
    m_firstObject = firstObject;

    UMLWidget * umlwidget = NULL;
    //m_pUMLView->viewport()->setMouseTracking(true);
    if (getWidgetType() == Uml::wt_Precondition) {
   	umlwidget = new PreconditionWidget(m_pUMLView, static_cast<ObjectWidget*>(m_firstObject));

	Dialog_Utils::askNameForWidget(umlwidget, i18n("Enter Precondition Name"), i18n("Enter the precondition"), i18n("new precondition"));
    	    // Create the widget. Some setup functions can remove the widget.
    	
    }

    if (getWidgetType() == Uml::wt_Pin) {
	kDebug() <<"SETWIDGET !!!!!!!!!!!!!!!!!!!!!!!" <<endl;
   	umlwidget = new PinWidget(m_pUMLView, static_cast<ActivityWidget*>(m_firstObject));
    	    // Create the widget. Some setup functions can remove the widget.
    	
    }
    if (getWidgetType() == Uml::wt_EndOfLife) {
   	umlwidget = new EndOfLifeWidget(m_pUMLView, static_cast<ObjectWidget*>(m_firstObject));

	//Dialog_Utils::askNameForWidget(umlwidget, i18n("Enter Precondition Name"), i18n("Enter the precondition"), i18n("new precondition"));
    	    // Create the widget. Some setup functions can remove the widget.
    	
    }

    if (umlwidget != NULL) {
            m_pUMLView->setupNewWidget(umlwidget);
    }

}


Uml::Widget_Type ToolBarStateOneWidget::getWidgetType() {
    if (getButton() == WorkToolBar::tbb_Seq_Precondition) {
        return Uml::wt_Precondition;
    }
    if (getButton() == WorkToolBar::tbb_Seq_End_Of_Life) {
        return Uml::wt_EndOfLife;
    }
    if (getButton() == WorkToolBar::tbb_Pin) {
        return Uml::wt_Pin;
    }
}


#include "toolbarstateonewidget.moc"
