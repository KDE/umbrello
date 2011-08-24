/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "objectwidget.h"

#include "umldoc.h"
#include "soc-umbrello-2011/umlview.h"
#include "umlwidgetcontroller.h"

using namespace Uml;

namespace QGV {
  
ObjectWidget::ObjectWidget(UMLView* view, UMLObject* o, UMLWidgetController* widgetController /*0*/): UMLWidget(view, o, widgetController)
{
  m_view = view;
  m_object = o;
}

ObjectWidget::ObjectWidget(UMLView* view, IDType id /*Uml::id_None*/, UMLWidgetController* widgetController /*0*/): UMLWidget(view, id, widgetController)
{
  m_id = id;
  m_view = view;
}

ObjectWidget::~ObjectWidget()
{

}

Uml::IDType ObjectWidget::localId() const
{
  return m_localId;
}

Uml::IDType ObjectWidget::id() const
{
  return m_id;
}

void ObjectWidget::setId(IDType id)
{
  m_id = id;
}

void ObjectWidget::setLocalId(IDType id)
{
  m_localId = id;
}

  
}