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
 ***************************************************************************/

#include <klocale.h>
#include <qlabel.h>
#include "codegenerationpolicypage.h"
#include "../codegenerationpolicy.h"

/** This is the page which comes up IF there is no special options for the 
 * code generator.
 */
CodeGenerationPolicyPage::CodeGenerationPolicyPage( QWidget *parent, const char *name, CodeGenerationPolicy * policy )
	:CodeGenerationPolicyBase(parent,name) 
{
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

