/***************************************************************************
                          attributepropertiespage.cpp
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
 

#include "attributepropertiespage.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../attribute.h"

#include <qlineedit.h>
#include <qtextedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>

namespace Umbrello{


AttributePropertiesPage::AttributePropertiesPage(UMLAttribute *a, QWidget *parent, const char *name)
                :AttributePropertiesBase( parent, name ),
		DialogPage( parent == 0 ),
		m_umlObject(a)
{
	m_type->setEditable(true);
	loadData();
	connect(m_type,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_name,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_initialValue,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_static,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	connect(m_public,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	connect(m_protected,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	connect(m_private,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));

	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}
                         
	

AttributePropertiesPage::~AttributePropertiesPage( )
{

}

void AttributePropertiesPage::apply()
{
	saveData();
}

void AttributePropertiesPage::cancel()
{
	loadData();
}

void AttributePropertiesPage::pageContentsModified()
{
	if(m_autoApply)
	{
		saveData();
	}
}


void AttributePropertiesPage::loadData()
{
	disconnect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
	m_type->clear();
	m_type->insertStringList( UMLApp::app()->getDocument()->getModelTypes() );
	m_type->setCurrentText(m_umlObject->getTypeName());
	m_name->setText(m_umlObject->getName());
	m_initialValue->setText(m_umlObject->getInitialValue());
	m_static->setChecked(m_umlObject->getStatic());
	switch(m_umlObject->getScope())
	{
		case Uml::Public:
			m_public->setChecked(true);
			break;
		case Uml::Protected:
			m_protected->setChecked(true);
			break;
		case Uml::Private:
		default:
			m_private->setChecked(true);
	}
	
	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

void AttributePropertiesPage::saveData()
{
	m_umlObject->blockSignals( true );
	
	m_umlObject->setTypeName( m_type->currentText() );
	m_umlObject->setName(m_name->text());
	m_umlObject->setInitialValue(m_initialValue->text());
	m_umlObject->setStatic(m_static->isChecked());
	if (m_public->isChecked())
		m_umlObject->setScope(Uml::Public);
	else if (m_protected->isChecked())
		m_umlObject->setScope(Uml::Protected);
	else
		m_umlObject->setScope(Uml::Private);
	
	m_umlObject->blockSignals( false );
	m_umlObject->emitModified( );
}

} // namespace Umbrello

#include "attributepropertiespage.moc"


