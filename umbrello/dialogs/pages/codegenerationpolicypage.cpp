/*
    SPDX-FileCopyrightText: 2003 Brian Thomas <brian.thomas@gsfc.nasa.gov>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// own header
#include "codegenerationpolicypage.h"

// qt/kde includes
#include <qlabel.h>
#include <KLocalizedString>

// local includes
#include "../codegenerationpolicy.h"


/** This is the page which comes up IF there is no special options for the
 * code generator.
 */
CodeGenerationPolicyPage::CodeGenerationPolicyPage(QWidget *parent, const char *name, CodeGenPolicyExt * policy)
  : DialogPageBase(parent)
{
    setObjectName(QLatin1String(name));
    m_parentPolicy = policy;
}

CodeGenerationPolicyPage::~CodeGenerationPolicyPage()
{
    this->disconnect();
}

void CodeGenerationPolicyPage::apply()
{
    // do nothing in vanilla version
}

void CodeGenerationPolicyPage::setDefaults()
{
}

