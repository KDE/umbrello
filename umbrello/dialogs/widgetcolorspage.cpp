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

 namespace Umbrello{


WidgetColorsPage::WidgetColorsPage( DiagramElement *e, QWidget *parent, const char *name ):
	WidgetColorsBase( parent, name ), m_element( e )
{
}

WidgetColorsPage::~WidgetColorsPage()
{
}

void WidgetColorsPage::apply()
{
}

void WidgetColorsPage::cancel()
{
}

 }


#include "widgetcolorspage.moc"
