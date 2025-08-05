/*
    SPDX-FileCopyrightText: 2025 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KGuiItem>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>

#include <QString>
#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

namespace UmlMessageBox {

const int Yes = KMessageBox::PrimaryAction;
const int No = KMessageBox::SecondaryAction;
const int Cancel = KMessageBox::Cancel;

inline KGuiItem yes()
{
    return KGuiItem(i18nc("@action:button", "Yes"), QStringLiteral("dialog-ok"));
}

inline KGuiItem no()
{
    return KGuiItem(i18nc("@action:button", "No"), QStringLiteral("dialog-no"));
}

inline KGuiItem cancel()
{
    return KStandardGuiItem::cancel();
}

inline int questionYesNo(QWidget *parent,
                         const QString &text,
                         const QString &caption = QString(),
                         const KGuiItem &yes = UmlMessageBox::yes(),
                         const KGuiItem &no = UmlMessageBox::no())
{
    auto result = KMessageBox::questionTwoActions(parent, text, caption, yes, no);
    return (result == KMessageBox::PrimaryAction) ? Yes : No;
}

inline int warningYesNo(QWidget *parent,
                         const QString &text,
                         const QString &caption = QString(),
                         const KGuiItem &yes = UmlMessageBox::yes(),
                         const KGuiItem &no = UmlMessageBox::no())
{
    auto result = KMessageBox::warningTwoActions(parent, text, caption, yes, no);
    return (result == KMessageBox::PrimaryAction) ? Yes : No;
}

inline int warningYesNoCancel(QWidget *parent,
                         const QString &text,
                         const QString &caption = QString(),
                         const KGuiItem &yes = UmlMessageBox::yes(),
                         const KGuiItem &no = UmlMessageBox::no(),
                         const KGuiItem &cancel = UmlMessageBox::cancel())
{
    auto result = KMessageBox::warningTwoActionsCancel(parent, text, caption, yes, no, cancel);
    return (result == KMessageBox::PrimaryAction) ? Yes : No;
}

} // namespace UmlMessageBox
