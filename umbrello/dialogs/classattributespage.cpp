/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classattributespage.h"
#include "attributepropertiespage.h"
#include "umbrellodialog.h"
#include "../class.h"
#include "../umldoc.h"
#include "../attribute.h"

#include <qtextedit.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

namespace Umbrello{



ClassAttributesPage::ClassAttributesPage(UMLClass *c, UMLDoc *doc, QWidget *parent, const char *name)
	: ClassAttributesBase( parent, name ),
	  DialogPage(parent==0),
	  m_umlObject(c),m_doc(doc)
{
	m_attributesList->setSorting( -1 ); //no sorting
	m_attributesList->setColumnWidthMode(1,QListView::Maximum);
	m_attributesList->setResizeMode(QListView::LastColumn);
	m_attributesList->setAllColumnsShowFocus( true );
	m_attList.setAutoDelete(true);
	loadPixmaps();
	loadData();
// 	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
// 	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

ClassAttributesPage::~ClassAttributesPage() {}

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
// 	if(m_autoApply) saveData();
}
void ClassAttributesPage::loadData()
{
// disconnect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
	/*FIXME sorry, too busy to fix this
	m_attList.clear();
	UMLAttributeList *list = m_umlObject->getAttList();
	QListViewItem *item;
	UMLAttribute *copy;
	// create list view and working-copy of attributes
	for( UMLAttribute *att = list->last(); att; att = list->prev() )
	{
		copy = new UMLAttribute(m_doc,att->getName(),att->getID(),att->getTypeName(), att->getScope(),att->getInitialValue());
		m_attList.prepend(copy);
		item = new QListViewItem( m_attributesList, copy->getName() );
		item->setPixmap(0, (copy->getScope() == Uml::Public ? m_pixmaps.Public :
				    (copy->getScope() == Uml::Protected ? m_pixmaps.Protected :
				    m_pixmaps.Private)));
		m_attMap[item] = copy;
	}
	*/
// connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

void ClassAttributesPage::saveData()
{
	/*FIXME sorry too busy to fix this
	m_umlObject->blockSignals( true );
	{
	 //remove deleted attributes
	UMLAttributeList list;
	list = *(m_umlObject->getAttList());
	list.setAutoDelete(false);
	kdDebug()<<"removing deleted atts"<<endl;
	for( UMLAttribute *att = list.first(); att ; att = list.next() )
	{kdDebug()<<"checking if "<<att->getName()<<" still exists."<<endl;
		 UMLAttribute *old(0);
		 for( old = m_attList.first(); old ; old = m_attList.next() )
		{
			if( old->getID() == att->getID() )
				 break;
		}
		if(!old)
		{
			 m_umlObject->removeAttribute( att );
			 delete att;
		}
	}
	}

	{
	// add/update attributes
	UMLAttributeList *pList = m_umlObject->getAttList();
	UMLAttribute *att;
	int index,old_index;
	for( att = m_attList.first(), index = 0; att ; att = m_attList.next(), ++index )
	{
		UMLAttribute *old(0);
		for( old = pList->first(), old_index = 0; old ; old = pList->next(), ++old_index )
		{kdDebug()<<"testing if modified:"<<old->getName()<<endl;
			if( old->getID() == att->getID() )
				break;
		}
		if( !old )
		{//add new attribute
		kdDebug()<<"new attribute!"<<endl;
			UMLAttribute *a = new UMLAttribute( m_doc, att->getName(),m_doc->getUniqueID(),
                                                            att->getTypeName(),att->getScope(),att->getInitialValue());
			a->setDoc( att->getDoc() );
			m_umlObject->addAttribute(a,index);
		}
		else
		{//update attribute
		kdDebug()<<"updating attribute!"<<endl;
			old->setName(att->getName());
			old->setTypeName(att->getTypeName());
			old->setScope(att->getScope());
			old->setInitialValue(att->getInitialValue());
			if( old_index != index )
			{kdDebug()<<"reordering"<<endl;
				m_umlObject->removeAttribute(old);
				m_umlObject->addAttribute(old,index);
			}
		}
	}
	}
	m_umlObject->blockSignals( false );
	m_umlObject->emitModified();
	//connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
	*/
}

void ClassAttributesPage::moveUp( )
{
	QListViewItem *item = m_attributesList->currentItem();
	int index = m_attList.findRef(m_attMap[item]);
	if( !item || index < 0 )
	{
		return;
	}
	QListViewItem *above = item->itemAbove( );
	if( !above )
	{
		return;
	}
	above = above->itemAbove( );
	if( above )
	{
		item->moveItem( above );
	}
	else
	{//we are already the second, and cannot move further up just like that, so we move the first child down instead
		m_attributesList->firstChild()->moveItem( item );
	}
	UMLAttribute *a = m_attList.take( index );
	m_attList.insert( --index, a );
	emit pageModified( );
}
void ClassAttributesPage::moveDown( )
{
	QListViewItem *item = m_attributesList->currentItem();
	int index = m_attList.findRef(m_attMap[item]);
	if( !item || index < 0 )
	{
		return;
	}
	QListViewItem *below = item->itemBelow();
	if( !below )
	{
		return;
	}
	item->moveItem( below );
	UMLAttribute *a = m_attList.take( index );
	m_attList.insert( ++index, a );
	emit pageModified( );
}

void ClassAttributesPage::createAttribute( )
{
	UMLAttribute *a = new UMLAttribute(m_doc,"new_att",-1);
	UmbrelloDialog dialog(this, UmbrelloDialog::Swallow, "edit_attribute", true, i18n("Attribute properties"),
	                       UmbrelloDialog::Ok | UmbrelloDialog::Cancel );
	AttributePropertiesPage *page = new AttributePropertiesPage(a,&dialog,0);
	dialog.setMainWidget(page);
// 	dialog.addPage(page,i18n("Attribute Properties"));
	if( dialog.exec() )
	{
		m_attList.append(a);
		QListViewItem *item = new QListViewItem( m_attributesList, a->getName() );
		item->setPixmap(0, (a->getScope() == Uml::Public ? m_pixmaps.Public :
				    (a->getScope() == Uml::Protected ? m_pixmaps.Protected :
			    	m_pixmaps.Private)));
		m_attMap[item] = a;
		emit pageModified( );
	}
	else
	{
		delete a;
	}
}

void ClassAttributesPage::editSelected( )
{
	QListViewItem *item = m_attributesList->currentItem();
	UMLAttribute *a = m_attMap[item];
	if(!a)
		return;
	UmbrelloDialog dialog(this, UmbrelloDialog::Swallow, "edit_attribute", true, i18n("Attribute properties"),
	                       UmbrelloDialog::Ok | UmbrelloDialog::Cancel );
	AttributePropertiesPage *page = new AttributePropertiesPage(a,&dialog,0);
// 	dialog.addPage(page,i18n("Attribute Properties"));
	dialog.setMainWidget(page);
	if(dialog.exec())
	{
		item->setText(0,a->getName());
		item->setPixmap(0, (a->getScope() == Uml::Public ? m_pixmaps.Public :
				    (a->getScope() == Uml::Protected ? m_pixmaps.Protected :
			    	m_pixmaps.Private)));
		emit pageModified( );
	}

}

void ClassAttributesPage::deleteSelected( )
{
	QListViewItem *item = m_attributesList->currentItem();
	UMLAttribute *a = m_attMap[item];
	if(!a)
		return;
	m_attList.removeRef( a );
	m_attMap.remove(item);
	delete a;
	delete item;
	emit pageModified( );
}

void ClassAttributesPage::itemSelected(QListViewItem *item )
{
	UMLAttribute *att = m_attMap[item];
	if( !att )
		return;

	//set doc
	m_documentation->setText( att->getDoc( ) );
	//enable/disable buttons
	m_upButton->setEnabled( item->itemAbove()?true:false);
	m_downButton->setEnabled( item->itemBelow()?true:false);
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

} //namespace Umbrello
#include "classattributespage.moc"

