/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include "classpropertiespage.h"
#include <qlineedit.h>
#include <qtextedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>


#include "../concept.h"

///////////////////
#include <kdebug.h>


ClassPropertiesPage::ClassPropertiesPage(UMLClass *c, QWidget *parent, const char *name)
		: ClassPropertiesBase(parent, name,0), m_umlObject(c)
{
	m_className->setText(m_umlObject->getName());

}
	


void ClassPropertiesPage::apply()
{
	kdDebug()<<"apply changes to class"<<endl;
	
}



void ClassPropertiesPage::cancel()
{
kdDebug()<<"reload data from umlclass"<<endl;

}

void ClassPropertiesPage::loadData()
{
	m_className->setText(m_umlObject->getName());
	m_stereotype->setText(m_umlObject->getStereotype());
	m_packageName->setText(m_umlObject->getPackage());
	m_abstract->setChecked(m_umlObject->getAbstract());
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

}

void ClassPropertiesPage::saveData()
{
	m_umlObject->setName(m_className->text());
	m_umlObject->setStereotype(m_stereotype->text());
	m_umlObject->setPackage(m_packageName->text());
	
	
	m_umlObject->setDoc(m_documentation->text());
}
