/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classattributespage.h"
#include "../class.h"
#include "../umldoc.h"
#include "../attribute.h"

#include <qtextedit.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <kstddirs.h>
#include <kdebug.h>


ClassAttributesPage::ClassAttributesPage(UMLClass *c, UMLDoc *doc, QWidget *parent, const char *name)
	: ClassAttributesBase( parent, name ),
	  m_umlObject(c),m_doc(doc)
{
	m_attributesList->setSorting( -1 ); //no sorting
	m_attributesList->setColumnWidthMode(1,QListView::Maximum);
	m_attributesList->setResizeMode(QListView::LastColumn);
	m_attributesList->setAllColumnsShowFocus( true );
	loadPixmaps();
	loadData();
	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
}



void ClassAttributesPage::apply()
{
	saveData();
}

void ClassAttributesPage::cancel()
{
	loadData();
}

void ClassAttributesPage::pageContentsModified()
{
	if(m_autoApply) saveData();
}
void ClassAttributesPage::loadData()
{
	QPtrList<UMLAttribute> *attList = m_umlObject->getAttList();
	QListViewItem *item;
	for( UMLAttribute *att = attList->last(); att; att = attList->prev() )
	{
		item = new QListViewItem( m_attributesList, "", att->getName( ) );
		item->setPixmap(1, (att->getScope() == Uml::Public ? m_pixmaps.Public :
				    (att->getScope() == Uml::Protected ? m_pixmaps.Protected :
				    m_pixmaps.Private)));
	}
}

void ClassAttributesPage::saveData()
{

	UMLAttribute *att;
	for( att = m_deletedAtts.first() ; att; att = m_deletedAtts.next() )
	{kdDebug()<<"deleting attribute..."<<endl;
		m_doc->removeUMLObject( att );
	}
	m_deletedAtts.clear();
//	blockSignals(true);

	/*m_umlObject->setName(m_className->text());
	m_umlObject->setStereotype(m_stereotype->text());
	m_umlObject->setPackage(m_packageName->text());
	if (m_public->isChecked())
		m_umlObject->setScope(Uml::Public);
	else if (m_protected->isChecked())
		m_umlObject->setScope(Uml::Protected);
	else
		m_umlObject->setScope(Uml::Private);
	m_umlObject->setDoc(m_documentation->text());*/

//	blockSignals(false);
}

void ClassAttributesPage::moveUp( )
{
	QListViewItem *item = m_attributesList->currentItem();
	if( !item)
	{
		return;
	}
	QListViewItem *above;
	(above = item->itemAbove( )) && (above = above->itemAbove( ));
	if( above )
	{
		item->moveItem( above );
	}
	else
	{//we are already the second, and cannot move further up, so we move the first child down instead
		m_attributesList->firstChild()->moveItem( item );
	}
	emit pageModified( );
}
void ClassAttributesPage::moveDown( )
{
	QListViewItem *item = m_attributesList->currentItem();
	if( !item)
	{
		return;
	}
	QListViewItem *below;
	if( (below = item->itemBelow()) && below )
	{
		item->moveItem( below );
		emit pageModified( );
	}


}
void ClassAttributesPage::createAttribute( )
{kdDebug()<<"create att"<<endl;
emit pageModified( );
}
void ClassAttributesPage::editSelected( )
{kdDebug()<<"edit selected"<<endl;
emit pageModified( );
}
void ClassAttributesPage::deleteSelected( )
{
	QListViewItem *item = m_attributesList->currentItem();
	QPtrList<UMLAttribute> *attList = m_umlObject->getAttList();
	UMLAttribute *att;
	for( att = attList->first(); att; att = attList->next() )
	{
		if(att->getName( ) == item->text(1))
			break;
	}
	if(att)
	{
		m_deletedAtts.append(att);
		delete item;

	}
emit pageModified( );
}
void ClassAttributesPage::itemSelected(QListViewItem *item )
{
	QPtrList<UMLAttribute> *attList = m_umlObject->getAttList();
	UMLAttribute *att;
	for( att = attList->first(); att; att = attList->next() )
	{
		if(att->getName( ) == item->text(1))
			break;
	}
	//set doc
	if(att)
	{
		m_documentation->setText( att->getDoc( ) );
	}
	//enable/disable buttons
	if(! item->itemAbove() )
	{
		m_upButton->setEnabled(false);
	}
	else
	{
		m_upButton->setEnabled(true);
	}
	if(! item->itemBelow() )
	{
		m_downButton->setEnabled(false);
	}
	else
	{
		m_downButton->setEnabled(true);
	}
}

void ClassAttributesPage::loadPixmaps()
{
	KStandardDirs *dirs = KGlobal::dirs();
	QString dataDir = dirs -> findResourceDir( "data", "umbrello/pics/public.png" );
	dataDir += "/umbrello/pics/";

	m_pixmaps.Public.load( dataDir + "public.png" );
	m_pixmaps.Protected.load( dataDir + "protected.png" );
	m_pixmaps.Private.load( dataDir + "private.png" );

}
#include "classattributespage.moc"

