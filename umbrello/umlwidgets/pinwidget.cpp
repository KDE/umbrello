/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "pinwidget.h"

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "floatingtextwidget.h"
#include "listpopupmenu.h"
#include "umlscene.h"
#include "activitywidget.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QPainter>

DEBUG_REGISTER_DISABLED(PinWidget)

/**
 * Creates a Pin widget.
 *
 * @param scene   The parent of the widget.
 * @param owner   The widget to which this pin is attached.
 * @param id      The ID to assign (-1 will prompt a new ID).
 */
PinWidget::PinWidget(UMLScene* scene, UMLWidget* owner, Uml::ID::Type id)
  : PinPortBase(scene, WidgetBase::wt_Pin, owner, id)
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
 * Captures any popup menu signals for menus it created.
 */
void PinWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            QString name = m_Text;
            bool ok = Dialog_Utils::askNewName(WidgetBase::wt_Pin, name);
            if (ok) {
                setName(name);
            }
        }
        break;

    default:
        PinPortBase::slotMenuSelection(action);
    }
}

