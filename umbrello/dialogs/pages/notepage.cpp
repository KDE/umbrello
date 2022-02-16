/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "notepage.h"

#include "notewidget.h"
#include "documentationwidget.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QVBoxLayout>

/**
 * Constructs an note page.
 */
NotePage::NotePage(QWidget *parent, NoteWidget *note)
  : DialogPageBase(parent),
    m_noteWidget(note)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_docWidget = new DocumentationWidget(m_noteWidget);
    layout->addWidget(m_docWidget, 10);
    setMinimumSize(600, 250);
    m_docWidget->setFocus();
}

/**
 *  Standard destructor.
 */
NotePage::~NotePage()
{
}

bool NotePage::apply()
{
    m_docWidget->apply();
    return true;
}

