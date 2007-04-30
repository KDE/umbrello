
/***************************************************************************
                          codegenerationpolicypage.cpp  -  description
                             -------------------
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
#include "defaultcodegenpolicypage.h"

// qt/kde includes
#include <qlabel.h>
#include <klocale.h>


/** This is the page which comes up IF there is no special options for the
 * code generator.
 */
DefaultCodeGenPolicyPage::DefaultCodeGenPolicyPage ( QWidget *parent, const char *name, CodeGenPolicyExt * policy )
        :CodeGenerationPolicyPage(parent,name,policy)
{
    textLabel = new QLabel(parent,"textLabel");
    textLabel->setText(tr2i18n("<p align=\"center\">No Options Available.</p>"));
}

DefaultCodeGenPolicyPage::~DefaultCodeGenPolicyPage() { }

#include "defaultcodegenpolicypage.moc"
