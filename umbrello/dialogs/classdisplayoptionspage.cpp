/***************************************************************************
                          classdisplayoptionspage.cpp
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
 

#include "classdisplayoptionspage.h"
#include "../diagram/classwidget.h"

#include <qcheckbox.h>


namespace Umbrello {


ClassDisplayOptionsPage::ClassDisplayOptionsPage(ClassWidget *w, QWidget *parent, const char *name):
	ClassDisplayOptionsBase(parent, name), m_widget(w)
{

}



ClassDisplayOptionsPage::~ClassDisplayOptionsPage( )
{
}

void ClassDisplayOptionsPage::apply()
{
}

void ClassDisplayOptionsPage::cancel()
{
}

void ClassDisplayOptionsPage::pageContentsModified()
{
}

void ClassDisplayOptionsPage::loadData()
{
}


void ClassDisplayOptionsPage::saveData()
{
}

}

#include "classdisplayoptionspage.moc"

