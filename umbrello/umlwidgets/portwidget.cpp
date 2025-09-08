/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "portwidget.h"

// app includes
#include "port.h"
#include "umlpackage.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "listpopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlscene.h"
#include "componentwidget.h"
#include "floatingtextwidget.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QPainter>
#include <QToolTip>

DEBUG_REGISTER_DISABLED(PortWidget)

/**
 * Constructs a PortWidget.
 *
 * @param scene   The parent scene of this PortWidget.
 * @param d       The UMLPort this will be representing.
 * @param owner   The owning widget to which this PortWidget is attached.
 */
PortWidget::PortWidget(UMLScene *scene, UMLPort *d, UMLWidget *owner)
  : PinPortBase(scene, WidgetBase::wt_Port, owner, d)
{
    setToolTip(d->name());
}

/**
 * Standard deconstructor.
 */
PortWidget::~PortWidget()
{
}

/**
 * Override function from PinPortWidget.
 */
UMLWidget* PortWidget::ownerWidget() const
{
    return PinPortBase::ownerWidget();
}

/**
 * Captures any popup menu signals for menus it created.
 */
void PortWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            QString newName = name();
            if (Dialog_Utils::askNewName(WidgetBase::wt_Port, newName))
                setName(newName);
        }
        break;

    default:
        PinPortBase::slotMenuSelection(action);
    }
}

