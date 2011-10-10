/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <brian.thomas@gsfc.nasa.gov>     *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "defaultcodegenpolicypage.h"

// qt/kde includes
#include <QtGui/QLabel>
#include <klocale.h>

/**
 * This is the page which comes up IF there is no special options for the
 * code generator.
 */
DefaultCodeGenPolicyPage::DefaultCodeGenPolicyPage(QWidget *parent, const char *name, CodeGenPolicyExt * policy)
        :CodeGenerationPolicyPage(parent,name,policy)
{
    m_textLabel = new QLabel(this);
    m_textLabel->setObjectName("textLabel");
    m_textLabel->setText(tr2i18n("<p align=\"center\">No Options Available.</p>"));
}

DefaultCodeGenPolicyPage::~DefaultCodeGenPolicyPage()
{
}

#include "defaultcodegenpolicypage.moc"
