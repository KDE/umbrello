/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "assocpage.h"

#include <qgroupbox.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <klocale.h>
#include <kdebug.h>

#include "../umlcanvasobject.h"
#include "../association.h"
#include "assocpropdlg.h"

AssocPage::AssocPage(QWidget *parent, UMLView * v, UMLObject * o) : QWidget(parent) {
	m_pObject = o;
	m_pView = v;
	int margin = fontMetrics().height();

	QHBoxLayout * mainLayout = new QHBoxLayout(this);
	mainLayout -> setSpacing(10);

	m_pAssocGB = new QGroupBox(i18n("Associations"), this);
	mainLayout -> addWidget(m_pAssocGB);

	QHBoxLayout * layout = new QHBoxLayout(m_pAssocGB);
	layout -> setSpacing(10);
	layout -> setMargin(margin);

	m_pAssocLB = new QListBox(m_pAssocGB);
	layout -> addWidget(m_pAssocLB);
	setMinimumSize(310, 330);
	fillListBox();
	m_pMenu = 0;

	connect(m_pAssocLB, SIGNAL(doubleClicked(QListBoxItem *)),
	        this, SLOT(slotDoubleClick(QListBoxItem *)));

	connect(m_pAssocLB, SIGNAL(rightButtonPressed(QListBoxItem *, const QPoint &)),
	        this, SLOT(slotRightButtonPressed(QListBoxItem *, const QPoint &)));

	connect(m_pAssocLB, SIGNAL(rightButtonClicked(QListBoxItem *, const QPoint &)),
	        this, SLOT(slotRightButtonClicked(QListBoxItem *, const QPoint &)));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
AssocPage::~AssocPage() {
	disconnect(m_pAssocLB, SIGNAL(doubleClicked(QListBoxItem *)),
	           this, SLOT(slotDoubleClick(QListBoxItem *)));

	disconnect(m_pAssocLB, SIGNAL(rightButtonPressed(QListBoxItem *, const QPoint &)),
	           this, SLOT(slotRightButtonPressed(QListBoxItem *, const QPoint &)));

	disconnect(m_pAssocLB, SIGNAL(rightButtonClicked(QListBoxItem *, const QPoint &)),
	           this, SLOT(slotRightButtonClicked(QListBoxItem *, const QPoint &)));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocPage::slotDoubleClick(QListBoxItem * i) {

	int item = m_pAssocLB -> currentItem();
	UMLAssociation *a = m_associations.at(item);
	if(!i || !a)
		return;

	AssocPropDlg dlg(this, a);
	int result = dlg.exec();
	if(result) {
		a -> setName(dlg.getName());
		a -> setRoleNameA(dlg.getRoleAName());
		a -> setRoleNameB(dlg.getRoleBName());
		a -> setMultiA(dlg.getMultiA());
		a -> setMultiB(dlg.getMultiB());
		fillListBox();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocPage::fillListBox() {

	m_pAssocLB->clear();
	m_associations.clear();

	UMLAssociation *a;
	UMLAssociationList list;
	UMLCanvasObject *c = dynamic_cast<UMLCanvasObject*>(m_pObject);
	if(!c)
	{//what is a CanvasObject!?!?!?!
		kdWarning()<<"AssocPage created for an object which is not a UMLCanvasObject!"<<endl;
		return;
	}
	int i = 0;
	list = c->getGeneralizations();
	for( a = list.first(); a; a = list.next() )
	{
		m_pAssocLB -> insertItem(a->toString(), i++);
		m_associations.append(a);
	}
	list = c->getRealizations();
	for( a = list.first(); a; a = list.next() )
	{
		m_pAssocLB -> insertItem(a->toString(), i++);
		m_associations.append(a);
	}
	list = c->getAggregations();
	for( a = list.first(); a; a = list.next() )
	{
		m_pAssocLB -> insertItem(a->toString(), i++);
		m_associations.append(a);
	}
	list = c->getCompositions();
	for( a = list.first(); a; a = list.next() )
	{
		m_pAssocLB -> insertItem(a->toString(), i++);
		m_associations.append(a);
	}
	list = c->getSpecificAssocs(Uml::at_Association);
	for( a = list.first(); a; a = list.next() )
	{
		m_pAssocLB -> insertItem(a->toString(), i++);
		m_associations.append(a);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocPage::slotRightButtonClicked(QListBoxItem */* item*/, const QPoint &/* p*/) {
	if(m_pMenu) {
		m_pMenu -> hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocPage::slotRightButtonPressed(QListBoxItem * item, const QPoint & p) {
	if(!item)
		return;
	if(m_pMenu) {
		m_pMenu -> hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
	m_pMenu = new ListPopupMenu(this, ListPopupMenu::mt_Association_Selected);
	m_pMenu->popup(p);
	connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocPage::slotPopupMenuSel(int id) {
//	UMLAssociation *a = m_associations.at(m_pAssocLB -> currentItem());
	switch(id) {
		case ListPopupMenu::mt_Delete:
			kdWarning()<<"FIXME - change m_pView->removeAssocInViewAndDoc(a) to use a UMLAssociation"<<endl;
			//m_pView->removeAssocInViewAndDoc(a);
			fillListBox();
			break;

		case ListPopupMenu::mt_Properties:
			slotDoubleClick(m_pAssocLB -> item(m_pAssocLB -> currentItem()));
			break;
	}
}





#include "assocpage.moc"
