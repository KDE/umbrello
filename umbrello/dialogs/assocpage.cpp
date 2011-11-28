/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "assocpage.h"

#include "assocpropdlg.h"
#include "debug_utils.h"
#include "umlview.h"

#include <klocale.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QGroupBox>

/**
 *  Constructs an instance of AssocPage.
 *
 *  @param  parent  The parent of the page
 *  @param  v       The view the UMLObject being represented.
 *  @param  o       The UMLObject being represented
 */
AssocPage::AssocPage(QWidget *parent, UMLView * v, UMLObject * o) : QWidget(parent)
{
    m_pObject = o;
    m_pView = v;
    int margin = fontMetrics().height();

    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(10);

    m_pAssocGB = new QGroupBox(i18n("Associations"), this);
    mainLayout->addWidget(m_pAssocGB);

    QHBoxLayout * layout = new QHBoxLayout(m_pAssocGB);
    layout->setSpacing(10);
    layout->setMargin(margin);

    m_pAssocLW = new QListWidget(m_pAssocGB);
    m_pAssocLW->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_pAssocLW);
    setMinimumSize(310, 330);
    fillListBox();
    m_pMenu = 0;

    connect(m_pAssocLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(slotDoubleClick(QListWidgetItem*)));
    connect(m_pAssocLW, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotRightButtonPressed(QPoint)));
}

/**
 *  Standard deconstructor.
 */
AssocPage::~AssocPage()
{
    disconnect(m_pAssocLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
               this, SLOT(slotDoubleClick(QListWidgetItem*)));
    disconnect(m_pAssocLW, SIGNAL(customContextMenuRequested(QPoint)),
               this, SLOT(slotRightButtonPressed(QPoint)));
}

void AssocPage::slotDoubleClick(QListWidgetItem * item)
{
    if (!item) {
        return;
    }

    int row = m_pAssocLW->currentRow();
    if ( row == -1 ) {
        return;
    }

    AssociationWidget * a = m_List.at(row);
    a->showPropertiesDialog();
    fillListBox();
}

/**
 *  Fills the list box with the objects associations.
 */
void AssocPage::fillListBox()
{
    m_List.clear();
    m_pAssocLW->clear();
    m_pView->getWidgetAssocs(m_pObject, m_List);
    int i = 0;
    foreach( AssociationWidget* assocwidget, m_List ) {
        if( assocwidget->associationType() != Uml::AssociationType::Anchor) {
            m_pAssocLW->insertItem(i, assocwidget->toString());
            i++;
        }
    }
}

void AssocPage::slotRightButtonPressed(const QPoint &p)
{
    QListWidgetItem* item = m_pAssocLW->itemAt(p);
    if (!item) {
        return;
    }
    if (m_pMenu) {
        m_pMenu->hide();
        disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
        delete m_pMenu;
        m_pMenu = 0;
    }
    m_pMenu = new ListPopupMenu(this, ListPopupMenu::mt_Association_Selected);
    connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotPopupMenuSel(QAction*)));
    m_pMenu->exec(m_pAssocLW->mapToGlobal(p));
}

void AssocPage::slotPopupMenuSel(QAction* action)
{
    int currentItemIndex = m_pAssocLW->currentRow();
    if ( currentItemIndex == -1 ) {
        return;
    }
    AssociationWidget * a = m_List.at(currentItemIndex);
    ListPopupMenu::MenuType id = m_pMenu->getMenuType(action);
    switch (id) {
    case ListPopupMenu::mt_Delete:
        m_pView->removeAssocInViewAndDoc(a);
        fillListBox();
        break;

    case ListPopupMenu::mt_Line_Color:
        //:TODO:
        uDebug() << "MenuType mt_Line_Color not yet implemented!";
        break;

    case ListPopupMenu::mt_Properties:
        slotDoubleClick(m_pAssocLW->currentItem());
        break;

    default:
        uDebug() << "MenuType " << ListPopupMenu::toString(id) << " not implemented";
    }
}

#include "assocpage.moc"
