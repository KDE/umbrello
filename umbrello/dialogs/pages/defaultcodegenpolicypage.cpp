/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <brian.thomas@gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "defaultcodegenpolicypage.h"

// qt/kde includes
#include <QLabel>
#include <KLocalizedString>

/**
 * This is the page which comes up IF there is no special options for the
 * code generator.
 */
DefaultCodeGenPolicyPage::DefaultCodeGenPolicyPage(QWidget *parent, const char *name, CodeGenPolicyExt * policy)
        :CodeGenerationPolicyPage(parent, name, policy)
{
    m_textLabel = new QLabel(this);
    m_textLabel->setObjectName(QStringLiteral("textLabel"));
    m_textLabel->setText(i18n("<p align=\"center\">No Options Available.</p>"));
}

DefaultCodeGenPolicyPage::~DefaultCodeGenPolicyPage()
{
}

