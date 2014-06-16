/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "pinwidget.h"

// app includes
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "listpopupmenu.h"
#include "umlscene.h"
#include "activitywidget.h"

// kde includes
#include <kinputdialog.h>
#include <klocale.h>

// qt includes
#include <QPainter>

DEBUG_REGISTER_DISABLED(PinWidget)

/**
 * Creates a Pin widget.
 *
 * @param scene   The parent of the widget.
 * @param a       The widget to which this pin is attached.
 * @param id      The ID to assign (-1 will prompt a new ID).
 */
PinWidget::PinWidget(UMLScene* scene, UMLWidget* a, Uml::ID::Type id)
  : PinPortBase(scene, WidgetBase::wt_Pin, a, id)
{
    // setParent(a);
    // m_nY = y() < getMinY() ? getMinY() : y();

    m_pName = new FloatingTextWidget(m_scene, Uml::TextRole::Floating, name());
    m_pName->setParentItem(this);
    m_pName->setText(name());  // to get geometry update
    m_pName->activate();
    setVisible(true);
}

/**
 * Destructor.
 */
PinWidget::~PinWidget()
{
}

/**
 * Implement abstract function from PinPortWidget.
 */
void PinWidget::connectOwnerMotion()
{
    ActivityWidget *owner = static_cast<ActivityWidget*>(ownerWidget());
    connect(owner, SIGNAL(sigActMoved(qreal, qreal)), this, SLOT(slotOwnerMoved(qreal, qreal)));
}

/**
 * Captures any popup menu signals for menus it created.
 */
void PinWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            bool ok = false;
            QString name = m_Text;
            name = KInputDialog::getText(i18n("Enter Pin Name"),
                                         i18n("Enter the pin name :"),
                                         m_Text, &ok);
            if (ok) {
                setName(name);
            }
        }
        break;

    default:
        PinPortBase::slotMenuSelection(action);
    }
}

#include "pinwidget.moc"
