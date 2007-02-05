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
#include "toolbarstatesequence.h"

// kde includes
#include <kdebug.h>

// local includes
#include "floatingtextwidget.h"
#include "preconditionwidget.h"
#include "endoflifewidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "dialog_utils.h"

#include <klocale.h>
#include <kmessagebox.h>

ToolBarStateSequence::ToolBarStateSequence(UMLView *umlView) : ToolBarStatePool(umlView) {
    m_firstObject = 0;
}

ToolBarStateSequence::~ToolBarStateSequence() {
}

void ToolBarStateSequence::init() {
    ToolBarStatePool::init();
}

void ToolBarStateSequence::cleanBeforeChange() {
    ToolBarStatePool::cleanBeforeChange();
}

void ToolBarStateSequence::mouseMove(QMouseEvent* ome) {
    ToolBarStatePool::mouseMove(ome);
}

void ToolBarStateSequence::slotWidgetRemoved(UMLWidget* widget) {
    ToolBarState::slotWidgetRemoved(widget);
}

void ToolBarStateSequence::setCurrentElement() {
    m_isObjectWidgetLine = false;

    ObjectWidget* objectWidgetLine = m_pUMLView->onWidgetLine(m_pMouseEvent->pos());
    if (objectWidgetLine) {
        setCurrentWidget(objectWidgetLine);
        m_isObjectWidgetLine = true;
        return;
    }

    UMLWidget *widget = m_pUMLView->testOnWidget(m_pMouseEvent->pos());
    if (widget) {
        setCurrentWidget(widget);
        return;
    }
}

void ToolBarStateSequence::mouseReleaseWidget() {
    
    if (m_pMouseEvent->button() != Qt::LeftButton ||
                getCurrentWidget()->getBaseType() != Uml::wt_Object) {
        return;
    }
    
    if (!m_isObjectWidgetLine && !m_firstObject) {
        return;
    }

    if (!m_firstObject) {
        setWidget(static_cast<ObjectWidget*>(getCurrentWidget()));
    } 
}

void ToolBarStateSequence::mouseReleaseEmpty() {
}

void ToolBarStateSequence::setWidget(ObjectWidget* firstObject) {
    m_firstObject = firstObject;

    UMLWidget * umlwidget = NULL;
    //m_pUMLView->viewport()->setMouseTracking(true);
    if (getWidgetType() == Uml::wt_Precondition) {
   	umlwidget = new PreconditionWidget(m_pUMLView, m_firstObject);

	Dialog_Utils::askNameForWidget(umlwidget, i18n("Enter Precondition Name"), i18n("Enter the precondition"), i18n("new precondition"));
    	    // Create the widget. Some setup functions can remove the widget.
    	
    }

    if (getWidgetType() == Uml::wt_EndOfLife) {
   	umlwidget = new EndOfLifeWidget(m_pUMLView, m_firstObject);

	//Dialog_Utils::askNameForWidget(umlwidget, i18n("Enter Precondition Name"), i18n("Enter the precondition"), i18n("new precondition"));
    	    // Create the widget. Some setup functions can remove the widget.
    	
    }

    if (umlwidget != NULL) {
            m_pUMLView->setupNewWidget(umlwidget);
    }
}


Uml::Widget_Type ToolBarStateSequence::getWidgetType() {
    if (getButton() == WorkToolBar::tbb_Seq_Precondition) {
        return Uml::wt_Precondition;
    }
    if (getButton() == WorkToolBar::tbb_Seq_End_Of_Life) {
        return Uml::wt_EndOfLife;
    }
}


#include "toolbarstatesequence.moc"
