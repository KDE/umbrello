/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "pkgcontentspage.h"

#include "classpropdlg.h"
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobjectlist.h"

#include <klocale.h>

#include <QtCore/QPointer>
#include <QtGui/QLayout>
#include <QtGui/QHBoxLayout>


/**
 * Constructs an instance of PkgContentsPage.
 * @param parent    The parent of the page.
 * @param pkg       The UMLPackage being represented.
 */
PkgContentsPage::PkgContentsPage(QWidget *parent, UMLPackage *pkg)
  : QWidget(parent)
{
    m_package = pkg;
    int margin = fontMetrics().height();

    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(10);

    m_contentGB = new QGroupBox(i18n("Contained Items"), this);
    mainLayout->addWidget(m_contentGB);

    QHBoxLayout * layout = new QHBoxLayout(m_contentGB);
    layout->setSpacing(10);
    layout->setMargin(margin);

    m_contentLW = new QListWidget(m_contentGB);
    m_contentLW->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_contentLW);
    setMinimumSize(310, 330);
    fillListBox();
    m_menu = 0;

    connect(m_contentLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(slotDoubleClick(QListWidgetItem*)));
    connect(m_contentLW, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotShowContextMenu(QPoint)));
}

/**
 * Standard destructor.
 */
PkgContentsPage::~PkgContentsPage()
{
    disconnect(m_contentLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
               this, SLOT(slotDoubleClick(QListWidgetItem*)));
    disconnect(m_contentLW, SIGNAL(customContextMenuRequested(QPoint)),
               this, SLOT(slotShowContextMenu(QPoint)));
}

void PkgContentsPage::slotDoubleClick(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    int index = m_contentLW->currentRow();
    if ( index == -1 ) {
        return;
    }
    UMLObjectList contents = m_package->containedObjects();
    UMLObject *o = contents.at(index);
    QPointer<ClassPropDlg> dlg = new ClassPropDlg(this, o, true);
    dlg->exec();
    delete dlg;
}

/**
 * Fills the list box with the package's contents.
 */
void PkgContentsPage::fillListBox()
{
    m_contentLW->clear();
    UMLObjectList contents = m_package->containedObjects();
    UMLObjectListIt objList_it(contents);
    UMLObject* umlo = NULL;
    while (objList_it.hasNext()) {
        umlo = objList_it.next();
        m_contentLW->addItem(umlo->name());
    }
}

/**
 * Slot for the context menu by right clicking in the list widget.
 * @param p   point of the right click inside the list widget
 */
void PkgContentsPage::slotShowContextMenu(const QPoint &p)
{
    QListWidgetItem *item = m_contentLW->itemAt(p);
    if (item) {
        if (m_menu) {
            m_menu->hide();
            disconnect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(slotPopupMenuSel(QAction*)));
            delete m_menu;
            m_menu = 0;
        }
        m_menu = new ListPopupMenu(this, ListPopupMenu::mt_Association_Selected);
        connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(slotPopupMenuSel(QAction*)));
        m_menu->exec(mapToGlobal(p) + QPoint(0, 20));
    }
}

void PkgContentsPage::slotPopupMenuSel(QAction* action)
{
    ListPopupMenu::MenuType id = m_menu->getMenuType(action);
    switch(id) {
    case ListPopupMenu::mt_Delete:
        {
            UMLObjectList contents = m_package->containedObjects();
            if ( m_contentLW->currentRow() == -1 )
                break;
            UMLObject *o = contents.at( m_contentLW->currentRow() );
            UMLApp::app()->document()->removeUMLObject(o);
            fillListBox();
        }
        break;

    case ListPopupMenu::mt_Properties:
        slotDoubleClick(m_contentLW->item(m_contentLW->currentRow()));
        break;

    default:
        uDebug() << "MenuType " << ListPopupMenu::toString(id) << " not implemented";
    }
}

#include "pkgcontentspage.moc"
