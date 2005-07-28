/*
 *  copyright (C) 2003-2004
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

#include "pkgcontentspage.h"
#include <qlayout.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <klocale.h>
#include "../umlobjectlist.h"
#include "../uml.h"
#include "../umldoc.h"
#include "classpropdlg.h"

PkgContentsPage::PkgContentsPage(QWidget *parent, UMLPackage *pkg)
        : QWidget(parent)
{
    m_pPackage = pkg;
    int margin = fontMetrics().height();

    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout -> setSpacing(10);

    m_pContentGB = new Q3GroupBox(i18n("Contained Items"), this);
    mainLayout -> addWidget(m_pContentGB);

    QHBoxLayout * layout = new QHBoxLayout(m_pContentGB);
    layout -> setSpacing(10);
    layout -> setMargin(margin);

    m_pContentLB = new Q3ListBox(m_pContentGB);
    layout -> addWidget(m_pContentLB);
    setMinimumSize(310, 330);
    fillListBox();
    m_pMenu = 0;

    connect(m_pContentLB, SIGNAL(doubleClicked(Q3ListBoxItem *)),
            this, SLOT(slotDoubleClick(Q3ListBoxItem *)));

    connect(m_pContentLB, SIGNAL(rightButtonPressed(Q3ListBoxItem *, const QPoint &)),
            this, SLOT(slotRightButtonPressed(Q3ListBoxItem *, const QPoint &)));

    connect(m_pContentLB, SIGNAL(rightButtonClicked(Q3ListBoxItem *, const QPoint &)),
            this, SLOT(slotRightButtonClicked(Q3ListBoxItem *, const QPoint &)));
}

PkgContentsPage::~PkgContentsPage() {
    disconnect(m_pContentLB, SIGNAL(doubleClicked(Q3ListBoxItem *)),
               this, SLOT(slotDoubleClick(Q3ListBoxItem *)));

    disconnect(m_pContentLB, SIGNAL(rightButtonPressed(Q3ListBoxItem *, const QPoint &)),
               this, SLOT(slotRightButtonPressed(Q3ListBoxItem *, const QPoint &)));

    disconnect(m_pContentLB, SIGNAL(rightButtonClicked(Q3ListBoxItem *, const QPoint &)),
               this, SLOT(slotRightButtonClicked(Q3ListBoxItem *, const QPoint &)));
}

void PkgContentsPage::slotDoubleClick(Q3ListBoxItem * i) {
    if (!i)
        return;
    int item = m_pContentLB -> currentItem();
    UMLObject *o = m_pPackage->containedObjects().at(item);
    ClassPropDlg dlg(this, o, item, true);
    dlg.exec();
}

void PkgContentsPage::fillListBox() {
    m_pContentLB->clear();
    UMLObjectListIt objList_it(m_pPackage->containedObjects());
    UMLObject* umlo = NULL;
    int i = 0;
    while ((umlo = objList_it.current()) != NULL) {
        m_pContentLB->insertItem(umlo->getName(), i++);
        ++objList_it;
    }
}

void PkgContentsPage::slotRightButtonClicked(Q3ListBoxItem */* item*/, const QPoint &/* p*/) {
    if(m_pMenu) {
        m_pMenu -> hide();
        disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

void PkgContentsPage::slotRightButtonPressed(Q3ListBoxItem * item, const QPoint & p) {
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

void PkgContentsPage::slotPopupMenuSel(int id) {
    switch(id) {
    case ListPopupMenu::mt_Delete:
        {
            UMLObject *o = m_pPackage->containedObjects().at(
                               m_pContentLB->currentItem() );
            UMLApp::app()->getDocument()->removeUMLObject(o);
            fillListBox();
        }
        break;

    case ListPopupMenu::mt_Properties:
        slotDoubleClick(m_pContentLB->item(m_pContentLB->currentItem()));
        break;
    }
}



#include "pkgcontentspage.moc"
