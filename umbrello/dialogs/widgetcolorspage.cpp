/***************************************************************************
                          widgetcolorspage.cpp
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


 #include "widgetcolorspage.h"
 #include "../diagram/diagramelement.h"
 
 #include <kcolorbutton.h>
 #include <qcheckbox.h>

 namespace Umbrello{


WidgetColorsPage::WidgetColorsPage( DiagramElement *e, QWidget *parent, const char *name ):
	WidgetColorsBase( parent, name ), DialogPage( parent == 0 ), m_element( e )
{
	loadData();
}

WidgetColorsPage::~WidgetColorsPage()
{
}

void WidgetColorsPage::loadData()
{
	m_lineColorButton->setColor(m_element->pen().color());
	m_fillColorButton->setColor(m_element->brush().color());
	m_useDiagramColors->setChecked( !(m_element->useOwnPen() || m_element->useOwnBrush() ) );
}

void WidgetColorsPage::saveData()
{
	QPen pen = m_element->pen();
	pen.setColor(m_lineColorButton->color());
	m_element->setPen(pen);
	QBrush brush = m_element->brush();
	brush.setColor(m_fillColorButton->color());
	m_element->setBrush(brush);
	
	m_element->setUseOwnPen(!m_useDiagramColors->isChecked());
	m_element->setUseOwnBrush(!m_useDiagramColors->isChecked());
}

void WidgetColorsPage::apply()
{
	saveData();
}

void WidgetColorsPage::cancel()
{
	loadData();
}

void WidgetColorsPage::pageContentsModified()
{
	if(m_autoApply) saveData();
}

}


#include "widgetcolorspage.moc"
