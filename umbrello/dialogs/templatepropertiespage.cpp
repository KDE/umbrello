/***************************************************************************
                          templatepropertiespage.cpp
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
 

#include "templatepropertiespage.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../template.h"

#include <qlineedit.h>



namespace Umbrello{


TemplatePropertiesPage::TemplatePropertiesPage(UMLTemplate *t, QWidget *parent, const char *name)
                :TemplatePropertiesBase( parent, name ),
		DialogPage( parent == 0 ),
		m_umlObject(t)
{
	loadData();
	connect(m_type,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_name,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	
	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}
                         
	

TemplatePropertiesPage::~TemplatePropertiesPage( )
{

}

void TemplatePropertiesPage::apply()
{
	saveData();
}

void TemplatePropertiesPage::cancel()
{
	loadData();
}

void TemplatePropertiesPage::pageContentsModified()
{
	if(m_autoApply)
	{
		saveData();
	}
}


void TemplatePropertiesPage::loadData()
{
	disconnect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
	
	m_type->clear();
	m_type->insertStringList( UMLApp::app()->getDocument()->getModelTypes() );
	m_type->setCurrentText(m_umlObject->getTypeName());
	m_name->setText(m_umlObject->getName());
	
	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

void TemplatePropertiesPage::saveData()
{
	m_umlObject->blockSignals( true );
	
	m_umlObject->setTypeName( m_type->currentText() );
	m_umlObject->setName(m_name->text());
	
	m_umlObject->blockSignals( false );
	m_umlObject->emitModified( );
}

} // namespace Umbrello

#include "templatepropertiespage.moc"


