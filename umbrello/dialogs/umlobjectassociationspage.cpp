/***************************************************************************
                          umlobjectassociationspage.cpp
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
 

#include "umlobjectassociationspage.h"

//FIXME - what is a UMLCanvasObject? shouldn't their methods (assoc) be moved to UMLObject?
#include "../umlcanvasobject.h"
#include "../association.h"

#include <qptrlist.h>
#include <qlistview.h>




namespace Umbrello{


UMLObjectAssociationsPage::UMLObjectAssociationsPage(UMLCanvasObject *c, QWidget *parent, const char *name)
                :UMLObjectAssociationsBase( parent, name ),
		DialogPage( parent == 0 ),
		m_umlObject(c)
{
	loadData();
}
                         
	

UMLObjectAssociationsPage::~UMLObjectAssociationsPage( )
{

}

void UMLObjectAssociationsPage::apply()
{
	saveData();
}

void UMLObjectAssociationsPage::cancel()
{
	loadData();
}

void UMLObjectAssociationsPage::pageContentsModified()
{
	if(m_autoApply)
	{
		saveData();
	}
}


void UMLObjectAssociationsPage::loadData()
{
// 	disconnect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
	m_associationList->clear();
	QPtrList<UMLAssociation> list = m_umlObject->getAssociations();
	list.setAutoDelete(false);
	QListViewItem *item;
	for( UMLAssociation *a = list.first(); a ; a = list.next() )
	{
		item = new QListViewItem(m_associationList,a->getName(),
		                   a->toString(a->getAssocType()));

	}
// 	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

void UMLObjectAssociationsPage::saveData()
{
// 	m_umlObject->blockSignals( true );
	
/*	m_umlObject->blockSignals( false );
	m_umlObject->emitModified( );*/
}

} // namespace Umbrello

 #include "umlobjectassociationspage.moc"


