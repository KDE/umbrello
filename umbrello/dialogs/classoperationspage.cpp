/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classoperationspage.h"
#include "../class.h"
#include "../umldoc.h"
#include "../operation.h"

#include <qtextedit.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <kdebug.h>


ClassOperationsPage::ClassOperationsPage(UMLClass *c,  UMLDoc *doc, QWidget *parent, const char *name)
	: ClassOperationsBase( parent, name ),
	  m_umlObject(c), m_doc(doc)
{
	m_operationsList->setSorting( -1 ); //no sorting
	m_operationsList->setColumnWidthMode(1,QListView::Maximum);
	m_operationsList->setResizeMode(QListView::LastColumn);
	m_operationsList->setAllColumnsShowFocus( true );
	loadData();
	connect(m_umlObject,SIGNAL(modified()),this,SLOT(loadData()));
}



void ClassOperationsPage::apply()
{
	saveData();
}

void ClassOperationsPage::cancel()
{
	loadData();
}

void ClassOperationsPage::modified()
{
	//if(m_autoApply) saveData();
}
void ClassOperationsPage::loadData()
{
	QPtrList<UMLOperation> *opList = m_umlObject->getOpList();
	for( UMLOperation *op = opList->last(); op; op = opList->prev() )
	{
		new QListViewItem( m_operationsList, "", op->getName( ) );
	}

}

void ClassOperationsPage::saveData()
{
	blockSignals(true);



	blockSignals(false);
}

void ClassOperationsPage::moveUp( )
{
	QListViewItem *item = m_operationsList->currentItem();
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
		m_operationsList->firstChild()->moveItem( item );
	}
	if( item->itemAbove() )
	{
		m_upButton->setEnabled(true);
	}
	else
	{
		m_upButton->setEnabled(false);
	}
}
void ClassOperationsPage::moveDown( )
{
	QListViewItem *item = m_operationsList->currentItem();
	if( !item)
	{
		return;
	}
	QListViewItem *below;
	if( (below = item->itemBelow()) && below )
	{
		item->moveItem( below );
		if( item->itemBelow() )
		{
			m_downButton->setEnabled(true);
		}
		else
		{
			m_downButton->setEnabled(false);
		}

	}
}

void ClassOperationsPage::createOperation( )
{ kdDebug()<<"create operation"<<endl;
}
void ClassOperationsPage::editSelected( )
{kdDebug()<<"edit selected"<<endl;
}

void ClassOperationsPage::deleteSelected( )
{kdDebug()<<"delete selected"<<endl;
}
void ClassOperationsPage::itemSelected(QListViewItem *item )
{
	QPtrList<UMLOperation> *opList = m_umlObject->getOpList();
	UMLOperation *op;
	for( op = opList->first(); op; op = opList->next() )
	{
		if(op->getName( ) == item->text(1))
			break;
	}
	//set documentation
	if(op)
	{
		m_documentation->setText( op->getDoc( ) );
	}
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

#include "classoperationspage.moc"

