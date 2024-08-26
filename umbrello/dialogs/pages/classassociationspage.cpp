/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "classassociationspage.h"

#include "associationwidget.h"
#include "associationpropertiesdialog.h"
#include "debug_utils.h"
#include "dialogpagebase.h"
#include "dialogspopupmenu.h"
#include "umlobject.h"
#include "umlscene.h"
#include "uml.h"

#include <KLocalizedString>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QListWidgetItem>

DEBUG_REGISTER_DISABLED(ClassAssociationsPage)

/**
 *  Constructs an instance of AssocPage.
 *
 *  @param  parent  The parent of the page
 *  @param  s       The scene on which the UMLObject is being represented
 *  @param  o       The UMLObject being represented
 */
ClassAssociationsPage::ClassAssociationsPage(QWidget *parent, UMLScene *s, UMLObject *o)
  : DialogPageBase(parent),
    m_pObject(o),
    m_pScene(s)
{
    int margin = fontMetrics().height();

    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(10);

    m_pAssocGB = new QGroupBox(i18n("Associations"), this);
    mainLayout->addWidget(m_pAssocGB);

    QHBoxLayout * layout = new QHBoxLayout(m_pAssocGB);
    layout->setSpacing(10);
    layout->setContentsMargins(margin, margin, margin, margin);

    m_pAssocLW = new QListWidget(m_pAssocGB);
    m_pAssocLW->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_pAssocLW);
    setMinimumSize(310, 330);
    fillListBox();

    connect(m_pAssocLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(slotDoubleClick(QListWidgetItem*)));
    connect(m_pAssocLW, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotRightButtonPressed(QPoint)));
}

/**
 *  Standard destructor.
 */
ClassAssociationsPage::~ClassAssociationsPage()
{
    disconnect(m_pAssocLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
               this, SLOT(slotDoubleClick(QListWidgetItem*)));
    disconnect(m_pAssocLW, SIGNAL(customContextMenuRequested(QPoint)),
               this, SLOT(slotRightButtonPressed(QPoint)));
}

void ClassAssociationsPage::slotDoubleClick(QListWidgetItem * item)
{
    if (!item) {
        return;
    }

    int row = m_pAssocLW->currentRow();
    if (row == -1) {
        return;
    }

    AssociationWidget * a = m_List.at(row);
    a->showPropertiesDialog();
    fillListBox();
}

/**
 *  Fills the list box with the objects associations.
 */
void ClassAssociationsPage::fillListBox()
{
    m_List.clear();
    m_pAssocLW->clear();
    m_pScene->getWidgetAssocs(m_pObject, m_List);
    int i = 0;
    for(AssociationWidget *assocwidget : m_List) {
        if(assocwidget->associationType() != Uml::AssociationType::Anchor) {
            m_pAssocLW->insertItem(i, assocwidget->toString());
            i++;
        }
    }
}

void ClassAssociationsPage::slotRightButtonPressed(const QPoint &p)
{
    DialogsPopupMenu popup(this, DialogsPopupMenu::tt_Association_Selected);
    QAction *triggered = popup.exec(m_pAssocLW->mapToGlobal(p));
    slotMenuSelection(triggered);
}

void ClassAssociationsPage::slotMenuSelection(QAction* action)
{
    int currentItemIndex = m_pAssocLW->currentRow();
    if (currentItemIndex == -1) {
        return;
    }
    AssociationWidget * a = m_List.at(currentItemIndex);
    ListPopupMenu::MenuType id = ListPopupMenu::typeFromAction(action);
    switch (id) {
    case ListPopupMenu::mt_Delete:
        m_pScene->removeAssocInViewAndDoc(a);
        fillListBox();
        break;

    case ListPopupMenu::mt_Line_Color:
        //:TODO:
        logDebug0("ClassAssociationsPage::slotMenuSelection: MenuType mt_Line_Color not yet implemented!");
        break;

    case ListPopupMenu::mt_Properties:
        slotDoubleClick(m_pAssocLW->currentItem());
        break;

    default:
        logDebug1("ClassAssociationsPage::slotMenuSelection: MenuType %1 not implemented",
                  ListPopupMenu::toString(id));
    }
}

