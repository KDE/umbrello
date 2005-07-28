/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "assocpage.h"
#include <qlayout.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <klocale.h>
#include "assocpropdlg.h"

AssocPage::AssocPage(QWidget *parent, UMLView * v, UMLObject * o) : QWidget(parent) {
    m_pObject = o;
    m_pView = v;
    int margin = fontMetrics().height();

    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout -> setSpacing(10);

    m_pAssocGB = new Q3GroupBox(i18n("Associations"), this);
    mainLayout -> addWidget(m_pAssocGB);

    QHBoxLayout * layout = new QHBoxLayout(m_pAssocGB);
    layout -> setSpacing(10);
    layout -> setMargin(margin);

    m_pAssocLB = new Q3ListBox(m_pAssocGB);
    layout -> addWidget(m_pAssocLB);
    setMinimumSize(310, 330);
    fillListBox();
    m_pMenu = 0;

    connect(m_pAssocLB, SIGNAL(doubleClicked(Q3ListBoxItem *)),
            this, SLOT(slotDoubleClick(Q3ListBoxItem *)));

    connect(m_pAssocLB, SIGNAL(rightButtonPressed(Q3ListBoxItem *, const QPoint &)),
            this, SLOT(slotRightButtonPressed(Q3ListBoxItem *, const QPoint &)));

    connect(m_pAssocLB, SIGNAL(rightButtonClicked(Q3ListBoxItem *, const QPoint &)),
            this, SLOT(slotRightButtonClicked(Q3ListBoxItem *, const QPoint &)));
}

AssocPage::~AssocPage() {
    disconnect(m_pAssocLB, SIGNAL(doubleClicked(Q3ListBoxItem *)),
               this, SLOT(slotDoubleClick(Q3ListBoxItem *)));

    disconnect(m_pAssocLB, SIGNAL(rightButtonPressed(Q3ListBoxItem *, const QPoint &)),
               this, SLOT(slotRightButtonPressed(Q3ListBoxItem *, const QPoint &)));

    disconnect(m_pAssocLB, SIGNAL(rightButtonClicked(Q3ListBoxItem *, const QPoint &)),
               this, SLOT(slotRightButtonClicked(Q3ListBoxItem *, const QPoint &)));
}

void AssocPage::slotDoubleClick(Q3ListBoxItem * i) {

    if(!i)
        return;

    int item = m_pAssocLB -> currentItem();

    AssociationWidget * a = m_List.at(item);

    if (a->showDialog())
        fillListBox();
}

void AssocPage::fillListBox() {
    m_List.clear();
    m_pAssocLB->clear();
    m_pView->getWidgetAssocs(m_pObject, m_List);
    AssociationWidgetListIt assoc_it(m_List);
    AssociationWidget* assocwidget = 0;
    int i = 0;
    while((assocwidget = assoc_it.current())) {
        if( assocwidget->getAssocType() != Uml::at_Anchor) {
            m_pAssocLB -> insertItem(assocwidget->toString(), i++);
        }
        ++assoc_it;
    }
}

void AssocPage::slotRightButtonClicked(Q3ListBoxItem */* item*/, const QPoint &/* p*/) {
    if(m_pMenu) {
        m_pMenu -> hide();
        disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

void AssocPage::slotRightButtonPressed(Q3ListBoxItem * item, const QPoint & p) {
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

void AssocPage::slotPopupMenuSel(int id) {
    AssociationWidget * a = m_List.at(m_pAssocLB -> currentItem());
    switch(id) {
    case ListPopupMenu::mt_Delete:
        m_pView->removeAssocInViewAndDoc(a);
        fillListBox();
        break;

    case ListPopupMenu::mt_Properties:
        slotDoubleClick(m_pAssocLB -> item(m_pAssocLB -> currentItem()));
        break;
    }
}




#include "assocpage.moc"
