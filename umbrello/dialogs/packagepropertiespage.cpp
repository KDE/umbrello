/***************************************************************************
                          packagepropertiespage.cpp
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

#include "packagepropertiespage.h"
#include "../package.h"

#include <qlineedit.h>
#include <qtextedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>


#include <kdebug.h>

namespace Umbrello{


PackagePropertiesPage::PackagePropertiesPage(UMLPackage *p, QWidget *parent, const char *name)
	: PackagePropertiesBase( parent, name ),
	  DialogPage(parent==0),
	  m_umlObject(p)
{
	loadData();
	connect(m_name,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_package,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_stereotype,SIGNAL(textChanged(const QString&)),this,SIGNAL(pageModified()));
	connect(m_public,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	connect(m_protected,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	connect(m_private,SIGNAL(toggled(bool)),this,SIGNAL(pageModified()));
	connect(m_documentation,SIGNAL(textChanged()),this,SIGNAL(pageModified()));

	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

PackagePropertiesPage::~PackagePropertiesPage() {}

void PackagePropertiesPage::apply()
{
	saveData();
}

void PackagePropertiesPage::cancel()
{
	loadData();
}

void PackagePropertiesPage::pageContentsModified()
{
	if(m_autoApply)
	{
		saveData();
	}
}


void PackagePropertiesPage::loadData()
{
	disconnect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));

	m_name->setText(m_umlObject->getName());
	m_stereotype->setText(m_umlObject->getStereotype());
	m_package->setText(m_umlObject->getPackage());
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
	m_documentation->setText(m_umlObject->getDoc());

	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

void PackagePropertiesPage::saveData()
{
	m_umlObject->blockSignals( true );
	//disconnect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));

	m_umlObject->setName(m_name->text());
	m_umlObject->setStereotype(m_stereotype->text());
	m_umlObject->setPackage(m_package->text());
	if (m_public->isChecked())
		m_umlObject->setScope(Uml::Public);
	else if (m_protected->isChecked())
		m_umlObject->setScope(Uml::Protected);
	else
		m_umlObject->setScope(Uml::Private);
	m_umlObject->setDoc(m_documentation->text());
	
	m_umlObject->blockSignals( false );
	//connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));	
	m_umlObject->emitModified( );

}

}//namespace Umbrello

#include "packagepropertiespage.moc"

