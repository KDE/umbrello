/***************************************************************************
                          operationpropertiespage.cpp
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

#include "operationpropertiespage.h"
#include "../operation.h"
#include "../attribute.h"

#include <qlineedit.h>
#include <qtextedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlistview.h>


#include <kdebug.h>


OperationPropertiesPage::OperationPropertiesPage(UMLOperation *c, QWidget *parent, const char *name)
	: OperationPropertiesBase( parent, name ),DialogPage( parent == 0 ),
	  m_umlObject(c)
{
	loadData();
	connect(m_name,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_type,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_abstract,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	connect(m_static,SIGNAL(toggled(bool)), this, SIGNAL(pageModified()));
	connect(m_public,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	connect(m_protected,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	connect(m_private,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	//parameter list

	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

OperationPropertiesPage::~OperationPropertiesPage() {}

void OperationPropertiesPage::apply()
{
	saveData();
}

void OperationPropertiesPage::cancel()
{
	loadData();
}

void OperationPropertiesPage::pageContentsModified()
{
	if(m_autoApply)
	{
		saveData();
	}
}


void OperationPropertiesPage::loadData()
{
	disconnect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));

	m_name->setText(m_umlObject->getName());
	m_type->setCurrentText(m_umlObject->getReturnType());
	m_abstract->setChecked(m_umlObject->getAbstract());
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
	QPtrList<UMLAttribute> *params = m_umlObject->getParmList();
	for( UMLAttribute *param = params->first(); param; param = params->next() )
	{
		new QListViewItem( m_parameterList, param->getName() );
	}
	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

void OperationPropertiesPage::saveData()
{
	disconnect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));

	m_umlObject->setName(m_name->text());
	m_umlObject->setReturnType(m_type->currentText());
	m_umlObject->setAbstract(m_abstract->isChecked());
	m_umlObject->setStatic(m_static->isChecked());
	if (m_public->isChecked())
		m_umlObject->setScope(Uml::Public);
	else if (m_protected->isChecked())
		m_umlObject->setScope(Uml::Protected);
	else
		m_umlObject->setScope(Uml::Private);

	//params!
	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
	
}

#include "operationpropertiespage.moc"

