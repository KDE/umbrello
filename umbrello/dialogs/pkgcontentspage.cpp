/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "pkgcontentspage.h"
#include <qlayout.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <klocale.h>
#include <kdebug.h>
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
    if ( item == -1 )
        return;
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
    while (objList_it.hasNext()) {
        umlo = objList_it.next();
        m_pContentLB->insertItem(umlo->getName(), i++);
    }
}

void PkgContentsPage::slotRightButtonClicked(Q3ListBoxItem */* item*/, const QPoint &/* p*/) {
    if(m_pMenu) {
        m_pMenu -> hide();
        disconnect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotPopupMenuSel(QAction*)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

void PkgContentsPage::slotRightButtonPressed(Q3ListBoxItem * item, const QPoint & p) {
    if(!item)
        return;
    if(m_pMenu) {
        m_pMenu -> hide();
        disconnect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotPopupMenuSel(QAction*)));
        delete m_pMenu;
        m_pMenu = 0;
    }
    m_pMenu = new ListPopupMenu(this, ListPopupMenu::mt_Association_Selected);
    m_pMenu->popup(p);
    connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotPopupMenuSel(QAction*)));
}

void PkgContentsPage::slotPopupMenuSel(QAction* action) {
    ListPopupMenu::Menu_Type id = m_pMenu->getMenuType(action);
    switch(id) {
    case ListPopupMenu::mt_Delete:
        {
            UMLObjectList contents = m_pPackage->containedObjects();
            if ( m_pContentLB->currentItem() == -1 )
                break;
            UMLObject *o = contents.at( m_pContentLB->currentItem() );
            UMLApp::app()->getDocument()->removeUMLObject(o);
            fillListBox();
        }
        break;

    case ListPopupMenu::mt_Properties:
        slotDoubleClick(m_pContentLB->item(m_pContentLB->currentItem()));
        break;

    default:
        uDebug() << "Menu_Type " << id << " not implemented" << endl;
    }
}



#include "pkgcontentspage.moc"
