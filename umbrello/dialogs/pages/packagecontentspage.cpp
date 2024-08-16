/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "packagecontentspage.h"

#include "classpropertiesdialog.h"
#include "debug_utils.h"
#include "dialogspopupmenu.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobjectlist.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QGroupBox>
#include <QLayout>
#include <QListWidget>
#include <QPointer>

DEBUG_REGISTER(PackageContentsPage)

/**
 * Constructs an instance of PackageContentsPage.
 * @param parent    The parent of the page.
 * @param pkg       The UMLPackage being represented.
 */
PackageContentsPage::PackageContentsPage(QWidget *parent, UMLPackage *pkg)
  : DialogPageBase(parent)
{
    m_package = pkg;
    int margin = fontMetrics().height();

    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(10);

    m_contentGB = new QGroupBox(i18n("Contained Items"), this);
    mainLayout->addWidget(m_contentGB);

    QHBoxLayout * layout = new QHBoxLayout(m_contentGB);
    layout->setSpacing(10);
    layout->setContentsMargins(margin, margin, margin, margin);

    m_contentLW = new QListWidget(m_contentGB);
    m_contentLW->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_contentLW);
    setMinimumSize(310, 330);
    fillListBox();

    connect(m_contentLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(slotDoubleClick(QListWidgetItem*)));
    connect(m_contentLW, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotShowContextMenu(QPoint)));
}

/**
 * Standard destructor.
 */
PackageContentsPage::~PackageContentsPage()
{
    disconnect(m_contentLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
               this, SLOT(slotDoubleClick(QListWidgetItem*)));
    disconnect(m_contentLW, SIGNAL(customContextMenuRequested(QPoint)),
               this, SLOT(slotShowContextMenu(QPoint)));
}

void PackageContentsPage::slotDoubleClick(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    int index = m_contentLW->currentRow();
    if (index == -1) {
        return;
    }
    UMLObjectList contents = m_package->containedObjects();
    UMLObject *o = contents.at(index);
    QPointer<ClassPropertiesDialog> dlg = new ClassPropertiesDialog(this, o, true);
    dlg->exec();
    delete dlg;
}

/**
 * Fills the list box with the package's contents.
 */
void PackageContentsPage::fillListBox()
{
    m_contentLW->clear();
    UMLObjectList contents = m_package->containedObjects();
    UMLObjectListIt objList_it(contents);
    UMLObject *umlo = nullptr;
    while (objList_it.hasNext()) {
        umlo = objList_it.next();
        uIgnoreZeroPointer(umlo);
        m_contentLW->addItem(umlo->name());
    }
}

/**
 * Slot for the context menu by right clicking in the list widget.
 * @param p   point of the right click inside the list widget
 */
void PackageContentsPage::slotShowContextMenu(const QPoint &p)
{
    DialogsPopupMenu popup(this, DialogsPopupMenu::tt_Association_Selected);
    QAction *triggered = popup.exec(mapToGlobal(p) + QPoint(0, 20));
    slotMenuSelection(triggered);
}

void PackageContentsPage::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType id = ListPopupMenu::typeFromAction(action);
    switch(id) {
    case ListPopupMenu::mt_Delete:
        {
            UMLObjectList contents = m_package->containedObjects();
            if (m_contentLW->currentRow() == -1)
                break;
            UMLObject *o = contents.at(m_contentLW->currentRow());
            UMLApp::app()->document()->removeUMLObject(o);
            fillListBox();
        }
        break;

    case ListPopupMenu::mt_Properties:
        slotDoubleClick(m_contentLW->item(m_contentLW->currentRow()));
        break;

    default:
        logDebug1("PackageContentsPage::slotMenuSelection: MenuType %1 not implemented",
                  ListPopupMenu::toString(id));
    }
}

