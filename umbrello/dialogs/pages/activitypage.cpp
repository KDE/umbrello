/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "activitypage.h"

#include "debug_utils.h"
#include "dialog_utils.h"
#include "dialogspopupmenu.h"
#include "statewidget.h"
#include "uml.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLayout>
#include <QPushButton>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>

DEBUG_REGISTER(ActivityPage)

/**
 * Constructor.
 */
ActivityPage::ActivityPage(QWidget * pParent, StateWidget * pWidget)
  : DialogPageBase(pParent)
{
    m_pStateWidget = pWidget;
    setupPage();
}

/**
 * Destructor.
 */
ActivityPage::~ActivityPage()
{
}

/**
 * Sets up the page.
 */
void ActivityPage::setupPage()
{
    int margin = fontMetrics().height();

    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    m_pActivityGB = new QGroupBox(i18n("Activities"), this);

    // vertical box layout for the activity lists, arrow buttons and the button box
    QVBoxLayout* listVBoxLayout = new QVBoxLayout(m_pActivityGB);
    listVBoxLayout->setMargin(margin);
    listVBoxLayout->setSpacing(10);

    //horizontal box contains the list box and the move up/down buttons
    QHBoxLayout* listHBoxLayout = new QHBoxLayout();
    listHBoxLayout->setSpacing(10);
    listVBoxLayout->addItem(listHBoxLayout);

    m_pActivityLW = new QListWidget(m_pActivityGB);
    m_pActivityLW->setContextMenuPolicy(Qt::CustomContextMenu);
    listHBoxLayout->addWidget(m_pActivityLW);

    QVBoxLayout * buttonLayout = new QVBoxLayout();
    listHBoxLayout->addItem(buttonLayout);

    m_pTopArrowB = new QToolButton(m_pActivityGB);
    m_pTopArrowB->setArrowType(Qt::UpArrow);
    m_pTopArrowB->setEnabled(false);
    m_pTopArrowB->setToolTip(i18n("Move selected item to the top"));
    buttonLayout->addWidget(m_pTopArrowB);

    m_pUpArrowB = new QToolButton(m_pActivityGB);
    m_pUpArrowB->setArrowType(Qt::UpArrow);
    m_pUpArrowB->setEnabled(false);
    m_pUpArrowB->setToolTip(i18n("Move selected item up"));
    buttonLayout->addWidget(m_pUpArrowB);

    m_pDownArrowB = new QToolButton(m_pActivityGB);
    m_pDownArrowB->setArrowType(Qt::DownArrow);
    m_pDownArrowB->setEnabled(false);
    m_pDownArrowB->setToolTip(i18n("Move selected item down"));
    buttonLayout->addWidget(m_pDownArrowB);

    m_pBottomArrowB = new QToolButton(m_pActivityGB);
    m_pBottomArrowB->setArrowType(Qt::DownArrow);
    m_pBottomArrowB->setEnabled(false);
    m_pBottomArrowB->setToolTip(i18n("Move selected item to the bottom"));
    buttonLayout->addWidget(m_pBottomArrowB);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(m_pActivityGB);
    QPushButton* newActivity = buttonBox->addButton(i18n("New Activity..."), QDialogButtonBox::ActionRole);
    connect(newActivity, SIGNAL(clicked()), this, SLOT(slotNewActivity()));
    m_pDeleteActivityButton = buttonBox->addButton(i18n("Delete"), QDialogButtonBox::ActionRole);
    connect(m_pDeleteActivityButton, SIGNAL(clicked()), this, SLOT(slotDelete()));
    m_pRenameButton = buttonBox->addButton(i18n("Rename"), QDialogButtonBox::ActionRole);
    connect(m_pRenameButton, SIGNAL(clicked()), this, SLOT(slotRename()));
    listVBoxLayout->addWidget(buttonBox);

    mainLayout->addWidget(m_pActivityGB);

    //now fill activity list box
    QStringList list = m_pStateWidget->activities();
    QStringList::ConstIterator end(list.end());

    for(QStringList::ConstIterator it(list.begin()); it != end; ++it) {
        m_pActivityLW->addItem(*it);
    }

    //now setup the signals
    connect(m_pActivityLW, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotClicked(QListWidgetItem*)));
    connect(m_pActivityLW, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotRightButtonPressed(QPoint)));

    connect(m_pTopArrowB, SIGNAL(clicked()), this, SLOT(slotTopClicked()));
    connect(m_pUpArrowB, SIGNAL(clicked()), this, SLOT(slotUpClicked()));
    connect(m_pDownArrowB, SIGNAL(clicked()), this, SLOT(slotDownClicked()));
    connect(m_pBottomArrowB, SIGNAL(clicked()), this, SLOT(slotBottomClicked()));

    connect(m_pActivityLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotDoubleClicked(QListWidgetItem*)));

    enableWidgets(false);
}

/**
 * Sets the activities of the widget.
 */
void ActivityPage::updateActivities()
{
    QStringList list;
    int count = m_pActivityLW->count();
    for (int i = 0; i < count; ++i) {
        list.append(m_pActivityLW->item(i)->text());
    }
    m_pStateWidget->setActivities(list);
}

/**
 * Popup menu item selected.
 */
void ActivityPage::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_New_Activity:
        slotNewActivity();
        break;

    case ListPopupMenu::mt_Delete:
        slotDelete();
        break;

    case ListPopupMenu::mt_Rename:
        slotRename();
        break;

    default:
        logDebug1("ActivityPage::slotMenuSelection: MenuType %1 not implemented",
                  ListPopupMenu::toString(sel));
    }
}

void ActivityPage::slotNewActivity()
{
    QString name;
    if (Dialog_Utils::askDefaultNewName(WidgetBase::wt_Activity, name) && name.length() > 0) {
        m_pActivityLW->addItem(name);
        m_pActivityLW->setCurrentRow(m_pActivityLW->count() - 1);
        m_pStateWidget->addActivity(name);
        slotClicked(m_pActivityLW->item(m_pActivityLW->count() - 1));
    }
}

void ActivityPage::slotDelete()
{
    QString name = m_pActivityLW->currentItem()->text();
    m_pStateWidget->removeActivity(name);
    m_pActivityLW->takeItem(m_pActivityLW->currentRow());
    slotClicked(nullptr);
}

void ActivityPage::slotRename()
{
    QString name = m_pActivityLW->currentItem()->text();
    QString oldName = name;
    bool ok = Dialog_Utils::askRenameName(WidgetBase::wt_Activity, name);
    if (ok && name.length() > 0) {
        QListWidgetItem* item = m_pActivityLW->currentItem();
        item->setText(name);
        m_pStateWidget->renameActivity(oldName, name);
        slotClicked(item);
    }
}

void ActivityPage::slotRightButtonPressed(const QPoint & p)
{
    DialogsPopupMenu::TriggerType type = DialogsPopupMenu::tt_Undefined;
    QListWidgetItem* item = m_pActivityLW->itemAt(p);
    if (item) { //pressed on an item
        type = DialogsPopupMenu::tt_Activity_Selected;
    } else { //pressed into fresh air
        type = DialogsPopupMenu::tt_New_Activity;
    }

    DialogsPopupMenu popup(this, type);
    QAction *triggered = popup.exec(m_pActivityLW->mapToGlobal(p));
    slotMenuSelection(triggered);
}

void ActivityPage::slotTopClicked()
{
    int count = m_pActivityLW->count();
    int index = m_pActivityLW->currentRow();
    //shouldn't occur, but just in case
    if (count <= 1 || index <= 0)
        return;

    //swap the text around in the ListBox
    QListWidgetItem* item = m_pActivityLW->takeItem(index);
    m_pActivityLW->insertItem(0, item);
    //set the moved item selected
    m_pActivityLW->setCurrentRow(0);

    slotClicked(m_pActivityLW->currentItem());
}

void ActivityPage::slotUpClicked()
{
    int count = m_pActivityLW->count();
    int index = m_pActivityLW->currentRow();
    //shouldn't occur, but just in case
    if (count <= 1 || index <= 0) {
        return;
    }

    QListWidgetItem* item = m_pActivityLW->takeItem(index);
    m_pActivityLW->insertItem(index - 1, item);
    //set the moved attribute selected
    m_pActivityLW->setCurrentRow(index - 1);

    slotClicked(m_pActivityLW->currentItem());
}

void ActivityPage::slotDownClicked()
{
    int count = m_pActivityLW->count();
    int index = m_pActivityLW->currentRow();
    //shouldn't occur, but just in case
    if (count <= 1 || index >= count - 1) {
        return;
    }

    QListWidgetItem* item = m_pActivityLW->takeItem(index);
    m_pActivityLW->insertItem(index + 1, item);
    //set the moved attribute selected
    m_pActivityLW->setCurrentRow(index + 1);

    slotClicked(m_pActivityLW->currentItem());
}

void ActivityPage::slotBottomClicked()
{
    int count = m_pActivityLW->count();
    int index = m_pActivityLW->currentRow();
    //shouldn't occur, but just in case
    if (count <= 1 || index >= count - 1)
        return;

    QListWidgetItem* item = m_pActivityLW->takeItem(index);
    m_pActivityLW->insertItem(m_pActivityLW->count(), item);
    //set the moved item selected
    m_pActivityLW->setCurrentRow(m_pActivityLW->count() - 1);

   slotClicked(m_pActivityLW->currentItem());
}

void ActivityPage::slotClicked(QListWidgetItem *item)
{
    //make sure clicked on an item
    if (!item) {
        enableWidgets(false);
        m_pActivityLW->clearSelection();
    } else {
        enableWidgets(true);
    }
}

void ActivityPage::slotDoubleClicked(QListWidgetItem* item)
{
    if (item) {
        slotRename();
    }
}

/**
 * Set the state of the widgets on the page with the given value.
 * @param state The state to set the widgets as.
 */
void ActivityPage::enableWidgets(bool state)
{
    if (!state) {
        m_pTopArrowB->setEnabled(false);
        m_pUpArrowB->setEnabled(false);
        m_pDownArrowB->setEnabled(false);
        m_pBottomArrowB->setEnabled(false);
        m_pDeleteActivityButton->setEnabled(false);
        m_pRenameButton->setEnabled(false);
        return;
    }
    /*now check the order buttons.
        Double check an item is selected
       If only one att. in list make sure there disabled.
        If at top item, only allow down arrow to be enabled.
        If at bottom item. only allow up arrow to be enabled.
    */
    int index = m_pActivityLW->currentRow();
    if (m_pActivityLW->count() == 1 || index == -1) {
        m_pTopArrowB->setEnabled(false);
        m_pUpArrowB->setEnabled(false);
        m_pDownArrowB->setEnabled(false);
        m_pBottomArrowB->setEnabled(false);
    } else if (index == 0) {
        m_pTopArrowB->setEnabled(false);
        m_pUpArrowB->setEnabled(false);
        m_pDownArrowB->setEnabled(true);
        m_pBottomArrowB->setEnabled(true);
    } else if(index == (int)m_pActivityLW->count() - 1) {
        m_pTopArrowB->setEnabled(true);
        m_pUpArrowB->setEnabled(true);
        m_pDownArrowB->setEnabled(false);
        m_pBottomArrowB->setEnabled(false);
   } else {
        m_pTopArrowB->setEnabled(true);
        m_pUpArrowB->setEnabled(true);
        m_pDownArrowB->setEnabled(true);
        m_pBottomArrowB->setEnabled(true);
    }
    m_pDeleteActivityButton->setEnabled(true);
    m_pRenameButton->setEnabled(true);
}

