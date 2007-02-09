/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "pkgcontentspage.h"
#include <qlayout.h>
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

    m_pContentGB = new QGroupBox(i18n("Contained Items"), this);
    mainLayout -> addWidget(m_pContentGB);

    QHBoxLayout * layout = new QHBoxLayout(m_pContentGB);
    layout -> setSpacing(10);
    layout -> setMargin(margin);

    m_pContentLB = new QListBox(m_pContentGB);
    layout -> addWidget(m_pContentLB);
    setMinimumSize(310, 330);
    fillListBox();
    m_pMenu = 0;

    connect(m_pContentLB, SIGNAL(doubleClicked(QListBoxItem *)),
            this, SLOT(slotDoubleClick(QListBoxItem *)));

    connect(m_pContentLB, SIGNAL(rightButtonPressed(QListBoxItem *, const QPoint &)),
            this, SLOT(slotRightButtonPressed(QListBoxItem *, const QPoint &)));

    connect(m_pContentLB, SIGNAL(rightButtonClicked(QListBoxItem *, const QPoint &)),
            this, SLOT(slotRightButtonClicked(QListBoxItem *, const QPoint &)));
}

PkgContentsPage::~PkgContentsPage() {
    disconnect(m_pContentLB, SIGNAL(doubleClicked(QListBoxItem *)),
               this, SLOT(slotDoubleClick(QListBoxItem *)));

    disconnect(m_pContentLB, SIGNAL(rightButtonPressed(QListBoxItem *, const QPoint &)),
               this, SLOT(slotRightButtonPressed(QListBoxItem *, const QPoint &)));

    disconnect(m_pContentLB, SIGNAL(rightButtonClicked(QListBoxItem *, const QPoint &)),
               this, SLOT(slotRightButtonClicked(QListBoxItem *, const QPoint &)));
}

void PkgContentsPage::slotDoubleClick(QListBoxItem * i) {
    if (!i)
        return;
    int item = m_pContentLB -> currentItem();
    UMLObjectList contents = m_pPackage->containedObjects();
    UMLObject *o = contents.at(item);
    ClassPropDlg dlg(this, o, item, true);
    dlg.exec();
}

void PkgContentsPage::fillListBox() {
    m_pContentLB->clear();
    UMLObjectList contents = m_pPackage->containedObjects();
    UMLObjectListIt objList_it(contents);
    UMLObject* umlo = NULL;
    int i = 0;
    while ((umlo = objList_it.current()) != NULL) {
        m_pContentLB->insertItem(umlo->getName(), i++);
        ++objList_it;
    }
}

void PkgContentsPage::slotRightButtonClicked(QListBoxItem */* item*/, const QPoint &/* p*/) {
    if(m_pMenu) {
        m_pMenu -> hide();
        disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

void PkgContentsPage::slotRightButtonPressed(QListBoxItem * item, const QPoint & p) {
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
            UMLObjectList contents = m_pPackage->containedObjects();
            UMLObject *o = contents.at( m_pContentLB->currentItem() );
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
