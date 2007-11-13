/***************************************************************************
    begin                : Tue Jul 29 2003
    copyright            : (C) 2003 by Brian Thomas
    email                : brian.thomas@gsfc.nasa.gov
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codegenerationpolicypage.h"

// qt/kde includes
#include <qlabel.h>
#include <klocale.h>

// local includes
#include "../codegenerationpolicy.h"


/** This is the page which comes up IF there is no special options for the
 * code generator.
 */
CodeGenerationPolicyPage::CodeGenerationPolicyPage( QWidget *parent, const char *name, CodeGenPolicyExt * policy )
        :QWidget( parent )
{
    setObjectName(name);
    m_parentPolicy = policy;
}

CodeGenerationPolicyPage::~CodeGenerationPolicyPage()
{
    this->disconnect();
}

void CodeGenerationPolicyPage::apply() {
    // do nothing in vanilla version
}

void CodeGenerationPolicyPage::setDefaults() { }

#include "codegenerationpolicypage.moc"

