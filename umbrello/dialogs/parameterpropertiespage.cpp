/***************************************************************************
                          parameterpropertiespage.cpp
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
 

#include "parameterpropertiespage.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../attribute.h"

#include <qlineedit.h>
#include <qtextedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>

namespace Umbrello{


ParameterPropertiesPage::ParameterPropertiesPage(UMLAttribute *a, QWidget *parent, const char *name)
                :ParameterPropertiesBase( parent, name ),
		DialogPage( parent == 0 ),
		m_umlObject(a)
{
	loadData();
	connect(m_type,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_name,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_initialValue,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));

	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}
                         
	

ParameterPropertiesPage::~ParameterPropertiesPage( )
{

}

void ParameterPropertiesPage::apply()
{
	saveData();
}

void ParameterPropertiesPage::cancel()
{
	loadData();
}

void ParameterPropertiesPage::pageContentsModified()
{
	if(m_autoApply)
	{
		saveData();
	}
}


void ParameterPropertiesPage::loadData()
{
	disconnect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
	
	m_type->clear();
	m_type->insertStringList( UMLApp::app()->getDocument()->getModelTypes() );
	m_type->setCurrentText(m_umlObject->getTypeName());
	m_name->setText(m_umlObject->getName());
	m_initialValue->setText(m_umlObject->getInitialValue());
	
	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

void ParameterPropertiesPage::saveData()
{
	m_umlObject->blockSignals( true );
	
	m_umlObject->setTypeName( m_type->currentText() );
	m_umlObject->setName(m_name->text());
	m_umlObject->setInitialValue(m_initialValue->text());
	
	m_umlObject->blockSignals( false );
	m_umlObject->emitModified( );
}

} // namespace Umbrello

#include "parameterpropertiespage.moc"


