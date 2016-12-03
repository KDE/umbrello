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
#if QT_VERSION < 0x050000
#include <kinputdialog.h>
#endif
#include <KLocalizedString>

// qt includes
#if QT_VERSION >= 0x050000
#include <QInputDialog>
#endif
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
    const Uml::ID::Type compWidgetId = m_umlObject->umlPackage()->id();
    setParentItem(scene->widgetOnDiagram(compWidgetId));
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
            bool ok = false;
            QString newName;
#if QT_VERSION >= 0x050000
            newName = QInputDialog::getText(Q_NULLPTR,
                                            i18n("Enter Port Name"), i18n("Enter the port name :"),
                                            QLineEdit::Normal,
                                            name(), &ok);
#else
            newName = KInputDialog::getText(i18n("Enter Port Name"), i18n("Enter the port name :"),
                                            name(), &ok);
#endif
            if (ok) {
                setName(newName);
            }
        }
        break;

    default:
        PinPortBase::slotMenuSelection(action);
    }
}

