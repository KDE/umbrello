/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classtemplatespage.h"
#include "templatepropertiespage.h"
#include "umbrellodialog.h"
#include "../class.h"
#include "../umldoc.h"
#include "../template.h"

#include <qtextedit.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>


namespace Umbrello{


ClassTemplatesPage::ClassTemplatesPage(UMLClass *c, UMLDoc *doc, QWidget *parent, const char *name)
	: ClassTemplatesBase( parent, name ),
	  DialogPage(parent==0),
	  m_umlObject(c),m_doc(doc)
{
	m_templatesList->setSorting( -1 ); //no sorting
	m_templatesList->setColumnWidthMode(0,QListView::Maximum);
	m_templatesList->setResizeMode(QListView::LastColumn);
	m_templatesList->setAllColumnsShowFocus( true );
	m_tempsList.setAutoDelete(true);
	loadData();
// 	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
// 	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

ClassTemplatesPage::~ClassTemplatesPage() {}

void ClassTemplatesPage::apply()
{
	saveData();
}

void ClassTemplatesPage::cancel()
{
	loadData();
}

void ClassTemplatesPage::pageContentsModified()
{
// 	if(m_autoApply) saveData();
}
void ClassTemplatesPage::loadData()
{
// disconnect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
	m_tempsList.clear();
	QPtrList<UMLTemplate> *list = m_umlObject->getTemplateList();
	QListViewItem *item;
	UMLTemplate *copy;
	// create list view and working-copy of templates
	for( UMLTemplate *temp = list->last(); temp; temp = list->prev() )
	{
		copy = new UMLTemplate(this,temp->getName(),temp->getID(),temp->getTypeName());
		m_tempsList.prepend(copy);
		item = new QListViewItem( m_templatesList, copy->getName() );
		m_tempMap[item] = copy;
	}
// connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

void ClassTemplatesPage::saveData()
{
	m_umlObject->blockSignals( true );
	{
	 //remove deleted templates
	QPtrList<UMLTemplate> list;
	list = *(m_umlObject->getTemplateList());
	list.setAutoDelete(false);
	for( UMLTemplate *temp = list.first(); temp ; temp = list.next() )
	{kdDebug()<<"checking if "<<temp->getName()<<" still exists."<<endl;
		 UMLTemplate *old(0);
		 for( old = m_tempsList.first(); old ; old = m_tempsList.next() )
		{
			if( old->getID() == temp->getID() )
				 break;
		}
		if(!old)
		{
			 m_umlObject->removeTemplate( temp );
			 delete temp;
		}
	}
	}
	
	{
	// add/update templates
	QPtrList<UMLTemplate> *pList = m_umlObject->getTemplateList();
	UMLTemplate *temp;
	int index,old_index;
	for( temp = m_tempsList.first(), index = 0; temp ; temp = m_tempsList.next(), ++index )
	{
		UMLTemplate *old(0);
		for( old = pList->first(), old_index = 0; old ; old = pList->next(), ++old_index )
		{kdDebug()<<"testing if modified:"<<old->getName()<<endl;
			if( old->getID() == temp->getID() )
				break;
		}
		if( !old )
		{//add new template
		kdDebug()<<"new template!"<<endl;
			UMLTemplate *t = new UMLTemplate( m_umlObject, temp->getName(),m_doc->getUniqueID(),
                                                            temp->getTypeName());
			m_umlObject->addTemplate(t,index);
		}
		else
		{//update template
		kdDebug()<<"updating template!"<<endl;
			old->setName(temp->getName());
			old->setTypeName(temp->getTypeName());
			if( old_index != index )
			{kdDebug()<<"reordering"<<endl;
				m_umlObject->removeTemplate(old);
				m_umlObject->addTemplate(old,index);
			}
		}
	}
	}
	m_umlObject->blockSignals( false );
	m_umlObject->emitModified();
	//connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
}

void ClassTemplatesPage::moveUp( )
{
	QListViewItem *item = m_templatesList->currentItem();
	int index = m_tempsList.findRef(m_tempMap[item]);
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
		m_templatesList->firstChild()->moveItem( item );
	}
	UMLTemplate *t = m_tempsList.take( index );
	m_tempsList.insert( --index, t );
	emit pageModified( );
}
void ClassTemplatesPage::moveDown( )
{
	QListViewItem *item = m_templatesList->currentItem();
	int index = m_tempsList.findRef(m_tempMap[item]);
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
	UMLTemplate *t = m_tempsList.take( index );
	m_tempsList.insert( ++index, t );
	emit pageModified( );
}

void ClassTemplatesPage::createTemplate( )
{
	UMLTemplate *t = new UMLTemplate(this,"new_template",-1);
	UmbrelloDialog dialog(this, UmbrelloDialog::Swallow, "edit_template", true, i18n("Template properties"), 
	                       UmbrelloDialog::Ok | UmbrelloDialog::Cancel );
	TemplatePropertiesPage *page = new TemplatePropertiesPage(t,&dialog,0);
	dialog.setMainWidget(page);
// 	dialog.addPage(page,i18n("Template Properties"));
	if( dialog.exec() )
	{
		m_tempsList.append(t);
		QListViewItem *item = new QListViewItem( m_templatesList, t->getName() );
		m_tempMap[item] = t;
		emit pageModified( );
	}
	else
	{
		delete t;
	}
}

void ClassTemplatesPage::editSelected( )
{
	QListViewItem *item = m_templatesList->currentItem();
	UMLTemplate *t = m_tempMap[item];
	if(!t)
		return;
	UmbrelloDialog dialog(this, UmbrelloDialog::Swallow, "edit_template", true, i18n("Template properties"), 
	                       UmbrelloDialog::Ok | UmbrelloDialog::Cancel );
	TemplatePropertiesPage *page = new TemplatePropertiesPage(t,&dialog,0);
// 	dialog.addPage(page,i18n("Template Properties"));
	dialog.setMainWidget(page);
	if(dialog.exec())
	{
		item->setText(0,t->getName());
		emit pageModified( );
	}
	
}

void ClassTemplatesPage::deleteSelected( )
{
	QListViewItem *item = m_templatesList->currentItem();
	UMLTemplate *t = m_tempMap[item];
	if(!t)
		return;
	m_tempsList.removeRef( t );
	m_tempMap.remove(item);
	delete t;
	delete item;
	emit pageModified( );
}

void ClassTemplatesPage::itemSelected(QListViewItem *item )
{
	UMLTemplate *temp = m_tempMap[item];
	if( !temp )
		return;
	
	//set doc
	m_documentation->setText( temp->getDoc( ) );
	//enable/disable buttons
	m_upButton->setEnabled( item->itemAbove()?true:false);
	m_downButton->setEnabled( item->itemBelow()?true:false);
}

}

#include "classtemplatespage.moc"

