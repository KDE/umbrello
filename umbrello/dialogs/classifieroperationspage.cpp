/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classifieroperationspage.h"
#include "operationpropertiespage.h"

#include "umbrellodialog.h"
#include "../classifier.h"
#include "../umldoc.h"
#include "../operation.h"
#include "../attribute.h"

#include <qtextedit.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

namespace Umbrello{


ClassifierOperationsPage::ClassifierOperationsPage(UMLClassifier *c, UMLDoc *doc, QWidget *parent, const char *name)
	: ClassifierOperationsBase( parent, name ),
	  DialogPage(parent==0),
	  m_umlObject(c),m_doc(doc)
{
	m_operationsList->setSorting( -1 ); //no sorting
	m_operationsList->setColumnWidthMode(0,QListView::Maximum);
	m_operationsList->setResizeMode(QListView::LastColumn);
	m_operationsList->setAllColumnsShowFocus( true );
	m_opsList.setAutoDelete(true);
	loadPixmaps();
	loadData();
// 	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
// 	connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

ClassifierOperationsPage::~ClassifierOperationsPage() {}

void ClassifierOperationsPage::apply()
{
	saveData();
}

void ClassifierOperationsPage::cancel()
{
	loadData();
}

void ClassifierOperationsPage::pageContentsModified()
{
	if(m_autoApply) saveData();
}
void ClassifierOperationsPage::loadData()
{
// disconnect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
	/*FIXME, sorry don't have time to fix this just now
	m_opsList.clear();
	QPtrList<UMLClassifierListItem>* list = m_umlObject->getOpList();
	QListViewItem* item;
	UMLOperation* copy;
	// create list view and working-copy of operations
	for( UMLOperation* op = list->last(); op; op = list->prev() )
	{
		copy = new UMLOperation(m_umlObject, op->getName(),-1, op->getScope(), op->getReturnType());
		m_opsList.prepend(copy);
		item = new QListViewItem( m_operationsList, copy->getName() );
		item->setPixmap(0, (copy->getScope() == Uml::Public ? m_pixmaps.Public :
				    (copy->getScope() == Uml::Protected ? m_pixmaps.Protected :
				    m_pixmaps.Private)));
		m_opsMap[item] = copy;

		QPtrList<UMLAttribute> *prmList = op->getParmList();
		for( UMLAttribute *p = prmList->first(); p ; p = prmList->next() )
		{
			copy->addParm( new UMLAttribute(this,p->getName(),p->getID(),p->getTypeName(), p->getScope(),p->getInitialValue()));
		}
	}
	*/
// connect(this,SIGNAL(pageModified()),this,SLOT(pageContentsModified()));
}

void ClassifierOperationsPage::saveData()
{
	/*FIXME
	m_umlObject->blockSignals( true );
	{
	 //remove deleted operations
	QPtrList<UMLOperation> list;
	list = *(m_umlObject->getOpList());
	list.setAutoDelete(false);
	for( UMLOperation *op = list.first(); op ; op = list.next() )
	{
		 UMLOperation *old(0);
		 for( old = m_opsList.first(); old ; old = m_opsList.next() )
		{
			if( old->getID() == op->getID() )
				 break;
		}
		if(!old)
		{
			 m_umlObject->removeOperation( op );
			 delete op;
		}
	}
	}

	{
	// add/update operations
	QPtrList<UMLOperation> *pList = m_umlObject->getOpList();
	UMLOperation *op;
	int index,old_index;
	for( op = m_opsList.first(), index = 0; op ; op = m_opsList.next(), ++index )
	{
		UMLOperation *old(0);
		for( old = pList->first(), old_index = 0; old ; old = pList->next(), ++old_index )
		{kdDebug()<<"testing if modified:"<<old->getName()<<endl;
			if( old->getID() == op->getID() )
				break;
		}
		if( !old )
		{//add new operation
			UMLOperation *o = new UMLOperation( m_umlObject, op->getName(),m_doc->getUniqueID(),
                                                            op->getScope(),op->getReturnType	());
			o->setDoc( op->getDoc() );
			m_umlObject->addOperation(o,index);
			syncParams( op, o );
		}
		else
		{//update operation
			old->setName(op->getName());
			old->setScope(op->getScope());
			old->setReturnType(op->getReturnType());
			if( old_index != index )
			{
				m_umlObject->removeOperation(old);
				m_umlObject->addOperation(old,index);
			}
			syncParams( op, old );
		}
	}
	}
	m_umlObject->blockSignals( false );
	m_umlObject->emitModified();
	//connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
	*/
}

void ClassifierOperationsPage::moveUp( )
{
	QListViewItem *item = m_operationsList->currentItem();
	int index = m_opsList.findRef(m_opsMap[item]);
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
		m_operationsList->firstChild()->moveItem( item );
	}
	UMLOperation *o = m_opsList.take( index );
	m_opsList.insert( --index, o );
	emit pageModified( );
}
void ClassifierOperationsPage::moveDown( )
{
	QListViewItem *item = m_operationsList->currentItem();
	int index = m_opsList.findRef(m_opsMap[item]);
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
	UMLOperation *o = m_opsList.take( index );
	m_opsList.insert( ++index, o );
	emit pageModified( );
}

void ClassifierOperationsPage::createOperation( )
{
	UMLOperation *o = new UMLOperation(m_umlObject,"new_operation",-1);
	UmbrelloDialog dialog(this, UmbrelloDialog::Swallow, "edit_operation", true, i18n("Operation properties"), 
	                       UmbrelloDialog::Ok | UmbrelloDialog::Cancel );
	OperationPropertiesPage *page = new OperationPropertiesPage(o,m_doc,&dialog,0);
	dialog.setMainWidget(page);
// 	dialog.addPage(page,i18n("Attribute Properties"));
	if( dialog.exec() )
	{
		m_opsList.append(o);
		QListViewItem *item = new QListViewItem( m_operationsList, o->getName() );
		item->setPixmap(0, (o->getScope() == Uml::Public ? m_pixmaps.Public :
				    (o->getScope() == Uml::Protected ? m_pixmaps.Protected :
			    	m_pixmaps.Private)));
		m_opsMap[item] = o;
		emit pageModified( );
	}
	else
	{
		delete o;
	}
}

void ClassifierOperationsPage::editSelected( )
{
	QListViewItem *item = m_operationsList->currentItem();
	UMLOperation *o = m_opsMap[item];
	if(!o)
		return;
	UmbrelloDialog dialog(this, UmbrelloDialog::Swallow, "edit_operation", true, i18n("Operation properties"),
	                       UmbrelloDialog::Ok | UmbrelloDialog::Cancel );
	OperationPropertiesPage *page = new OperationPropertiesPage(o,m_doc,&dialog,0);
// 	dialog.addPage(page,i18n("Attribute Properties"));
	dialog.setMainWidget(page);
	if(dialog.exec())
	{
		item->setText(0,o->getName());
		item->setPixmap(0, (o->getScope() == Uml::Public ? m_pixmaps.Public :
				    (o->getScope() == Uml::Protected ? m_pixmaps.Protected :
			    	m_pixmaps.Private)));
		emit pageModified( );
	}

}

void ClassifierOperationsPage::deleteSelected( )
{
	QListViewItem *item = m_operationsList->currentItem();
	UMLOperation *o = m_opsMap[item];
	if(!o)
		return;
	m_opsList.removeRef( o );
	m_opsMap.remove(item);
	delete o;
	delete item;
	emit pageModified( );
}

////////////////////////////////////
void ClassifierOperationsPage::syncParams( UMLOperation *src, UMLOperation *dest )
{
	QPtrList<UMLAttribute> *srcList = src->getParmList();
	{
	QPtrList<UMLAttribute> list = *(dest->getParmList());
	list.setAutoDelete(false);

	kdDebug()<<"removing deleted params"<<endl;
	for( UMLAttribute *att = list.first(); att ; att = list.next() )
	{
		UMLAttribute *old(0);
		for( old = srcList->first(); old ; old = srcList->next() )
		{
			if( old->getID() == att->getID() )
				 break;
		}
		if(!old)
		{
		 	dest->removeParm( att );
		 	delete att;
		}
	}
	}
	{
	// add/update attributes
	QPtrList<UMLAttribute> *destList = dest->getParmList();
	UMLAttribute *att;
	int index,old_index;
	for( att = srcList->first(), index = 0; att ; att = srcList->next(), ++index )
	{
		UMLAttribute *old(0);
		for( old = destList->first(), old_index = 0; old ; old = destList->next(), ++old_index )
		{kdDebug()<<"testing if modified:"<<old->getName()<<endl;
			if( old->getID() == att->getID() )
				break;
		}
		if( !old )
		{//add new attribute
		kdDebug()<<"new attribute!"<<endl;
			UMLAttribute *a = new UMLAttribute( m_umlObject, att->getName(),m_doc->getUniqueID(),
                                                            att->getTypeName(),att->getScope(),att->getInitialValue());
			a->setDoc( att->getDoc() );
			dest->addParm(a,index);
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
				dest->removeParm(old);
				dest->addParm(old,index);
			}
		}
	}
	}
}

////////////////////////////////

void ClassifierOperationsPage::itemSelected(QListViewItem *item )
{
	UMLOperation *op = m_opsMap[item];
	if( !op )
		return;

	//set doc
	m_documentation->setText( op->getDoc( ) );
	//enable/disable buttons
	m_upButton->setEnabled( item->itemAbove()?true:false);
	m_downButton->setEnabled( item->itemBelow()?true:false);
}

void ClassifierOperationsPage::loadPixmaps()
{
	KStandardDirs *dirs = KGlobal::dirs();
	QString dataDir = dirs -> findResourceDir( "data", "umbrello/pics/public.png" );
	dataDir += "/umbrello/pics/";

	m_pixmaps.Public.load( dataDir + "public.png" );
	m_pixmaps.Protected.load( dataDir + "protected.png" );
	m_pixmaps.Private.load( dataDir + "private.png" );

}

} //namespace Umbrello

#include "classifieroperationspage.moc"

