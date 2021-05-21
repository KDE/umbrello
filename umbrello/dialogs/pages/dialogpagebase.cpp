/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dialogpagebase.h"

// qt includes
#include <QKeyEvent>

/**
 * Constructor
 */
DialogPageBase::DialogPageBase(QWidget *parent)
  : QWidget(parent),
    m_isModified(false)
{
}

DialogPageBase::~DialogPageBase()
{
}

/**
 * Return state if page has been modified by user.
 *
 * The state will be used to determine changed pages.
 *
 * @return true page has been modified
 */
bool DialogPageBase::isModified()
{
    return m_isModified;
}

/**
 * Handle key press event.
 * @param event key press event
 */
void DialogPageBase::keyPressEvent(QKeyEvent *event)
{
    // Set modified state if any text has been typed in
    if (event->key() >= Qt::Key_Space
            && event->key() < Qt::Key_Multi_key)
        m_isModified = true;

    QWidget::keyPressEvent(event);
}
