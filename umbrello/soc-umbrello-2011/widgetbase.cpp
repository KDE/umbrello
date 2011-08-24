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

#include "widgetbase.h"
#include "umlobject.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"

namespace QGV {


WidgetBase::WidgetBase(UMLView* view): QObject(UMLApp::app()->current_View()),
  m_view(UMLApp::app()->current_View())
{
  init(m_view);
}


WidgetBase::~WidgetBase()
{

}

void WidgetBase::init(UMLView* view, Widget_Type type /* = Uml::wt_UMLWidget */)
{
    m_view = UMLApp::app()->current_View();
    m_type = type;
    m_object = NULL;
    if (m_view) {
        m_bUsesDiagramLineColour = true;
        m_bUsesDiagramLineWidth  = true;
        const Settings::OptionState& optionState = m_view->scene()->options();
        m_LineColour = optionState.uiState.lineColor;
        m_LineWidth  = optionState.uiState.lineWidth;
    } else {
        uError() << "WidgetBase constructor: SERIOUS PROBLEM - m_view is NULL";
        m_bUsesDiagramLineColour = false;
        m_bUsesDiagramLineWidth  = false;
        m_LineColour = QColor("black");
        m_LineWidth = 0; // initialize with 0 to have valid start condition
    }

}

void WidgetBase::setBaseType( Uml::Widget_Type type )
{
    m_type = type;
}

void WidgetBase::setUmlDoc(UMLDoc* doc)
{
  m_doc = doc;
}


void WidgetBase::setUmlObject(UMLObject* o)
{
  m_object = o;
}

void WidgetBase::setView(UMLView* view)
{
  m_view = view;
}

UMLDoc* WidgetBase::umlDoc() const
{
  return m_doc;
}

UMLObject* WidgetBase::umlObject() const
{
  return m_object;
}

UMLView* WidgetBase::view() const
{
  return m_view;
}

Uml::Widget_Type WidgetBase::widget_type()
{
  return m_type;
}

  
}