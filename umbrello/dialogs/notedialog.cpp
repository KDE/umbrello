/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "notedialog.h"
#include "notewidget.h"

// KDE includes
#include <KLocalizedString>

/**
 * Constructs an NoteDialog.
 */
NoteDialog::NoteDialog(QWidget * parent, NoteWidget * widget)
  : MultiPageDialogBase(parent),
    m_widget(widget)
{
    setCaption(i18n("Note Properties"));
    setupPages();
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

void NoteDialog::setupPages()
{
    setupGeneralPage(m_widget);
    setupStylePage(m_widget);
    setupFontPage(m_widget);
}

void NoteDialog::slotOk()
{
    slotApply();
}

void NoteDialog::slotApply()
{
    if (!apply())
        reject();
    else
        accept();
}

bool NoteDialog::apply()
{
    MultiPageDialogBase::apply();
    if (m_widget) {
        applyFontPage(m_widget);
        QString key = QStringLiteral("Diagram:");
        QString str = m_widget->documentation();
        if (!str.startsWith(key)) {
            m_widget->setDiagramLink(QString());
            return false;
        }
        QString diagramName = str.remove(key).trimmed();
        m_widget->setDiagramLink(diagramName);
    }
    return true;
}
