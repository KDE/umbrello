/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "portwidget.h"

// app includes
#include "port.h"
#include "package.h"
#include "debug_utils.h"
#include "umldoc.h"
#include "umlscene.h"
#include "componentwidget.h"
#include "floatingtextwidget.h"

// kde includes
#include <kinputdialog.h>
#include <KLocalizedString>

// qt includes
#include <QPainter>
#include <QToolTip>

/**
 * Constructs a PortWidget.
 *
 * @param scene   The parent of this PortWidget.
 * @param d       The UMLPort this will be representing.
 */
PortWidget::PortWidget(UMLScene *scene, UMLPort *d) 
  : PinPortBase(scene, WidgetBase::wt_Port, d)
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
UMLWidget* PortWidget::ownerWidget()
{
    if (m_pOw == NULL) {
        const Uml::ID::Type compWidgetId = m_umlObject->umlPackage()->id();
        m_pOw = m_scene->widgetOnDiagram(compWidgetId);
    }
    return m_pOw;
}

/**
 * Implement abstract function from PinPortWidget.
 */
void PortWidget::connectOwnerMotion()
{
    ComponentWidget *owner = static_cast<ComponentWidget*>(ownerWidget());
    connect(owner, SIGNAL(sigCompMoved(qreal,qreal)), this, SLOT(slotOwnerMoved(qreal,qreal)));
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
            bool ok = false;
            QString newName;
            newName = KInputDialog::getText(i18n("Enter Port Name"), i18n("Enter the port name :"),
                                            name(), &ok);
            if (ok) {
                setName(newName);
            }
        }
        break;

    default:
        PinPortBase::slotMenuSelection(action);
    }
}

