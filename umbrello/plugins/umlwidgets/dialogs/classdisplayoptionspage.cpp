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
#include "../classwidget.h"

#include <qcheckbox.h>


namespace Umbrello {


ClassDisplayOptionsPage::ClassDisplayOptionsPage(ClassWidget *w, QWidget *parent, const char *name):
	ClassDisplayOptionsBase(parent, name),DialogPage(parent == 0), m_widget(w)
{
	loadData();
}



ClassDisplayOptionsPage::~ClassDisplayOptionsPage( )
{
}

void ClassDisplayOptionsPage::apply()
{
	saveData();
}

void ClassDisplayOptionsPage::cancel()
{
	loadData();
}

void ClassDisplayOptionsPage::pageContentsModified()
{
}

void ClassDisplayOptionsPage::loadData()
{
	int ops = m_widget->nameDisplayOptions();
	m_showStereotype->setChecked( ops & ClassWidget::ShowStereotype );
	m_showPackage->setChecked( ops & ClassWidget::ShowPackage );
	
	ops = m_widget->opsDisplayOptions();
	m_showOperations->setChecked( ops & ClassWidget::ShowOps );
	m_showParameters->setChecked( ops & ClassWidget::ShowParameterList );
	
	ops = m_widget->attsDisplayOptions();
	m_showAttributes->setChecked( ops & ClassWidget::ShowAtts );
	m_showInitialValue->setChecked( ops & ClassWidget::ShowInitialValue );
}


void ClassDisplayOptionsPage::saveData()
{

	int ops = 0;
	if(m_showStereotype->isChecked())
		ops |= ClassWidget::ShowStereotype;
	if(m_showPackage->isChecked())
		ops |= ClassWidget::ShowPackage;
	m_widget->setNameDisplayOptions( ops );
	
	ops = 0;
	if(m_showOperations->isChecked())
		ops |= ClassWidget::ShowOps;
	if(m_showParameters->isChecked())
		ops |= ClassWidget::ShowParameterList;
	m_widget->setOpsDisplayOptions( ops );
	
	ops = 0L;
	if(m_showAttributes->isChecked())
		ops |= ClassWidget::ShowAtts;
	if(m_showInitialValue->isChecked())
		ops |= ClassWidget::ShowInitialValue;
	m_widget->setAttsDisplayOptions( ops );
	
}

}

#include "classdisplayoptionspage.moc"

