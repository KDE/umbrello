/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "rubycodegenerationpolicypage.h"

// app includes
#include "codegenerationpolicy.h"
#include "uml.h"

// kde includes
#include <klocale.h>

RubyCodeGenerationPolicyPage::RubyCodeGenerationPolicyPage(QWidget *parent, const char *name, RubyCodeGenerationPolicy * policy)
  : CodeGenerationPolicyPage(parent, name, policy)
{
    CodeGenerationPolicy *common = UMLApp::app()->commonPolicy();
    form.setupUi(this);
    form.m_SelectCommentStyle->setCurrentIndex((int)(common->getCommentStyle()));
    form.m_generateConstructors->setChecked(common->getAutoGenerateConstructors());
    form.m_generateAttribAccessors->setChecked(policy->getAutoGenerateAttribAccessors());
    form.m_generateAssocAccessors->setChecked(policy->getAutoGenerateAssocAccessors());
    form.m_accessorScopeCB->setCurrentIndex(common->getAttributeAccessorScope());
    form.m_assocFieldScopeCB->setCurrentIndex(common->getAssociationFieldScope());
}

RubyCodeGenerationPolicyPage::~RubyCodeGenerationPolicyPage()
{
}

void RubyCodeGenerationPolicyPage::apply()
{
    CodeGenerationPolicy *common = UMLApp::app()->commonPolicy();

    // now do our ruby-specific configs
    RubyCodeGenerationPolicy * parent = (RubyCodeGenerationPolicy*) m_parentPolicy;

    // block signals so we don't cause too many update content calls to code documents
    parent->blockSignals(true);

    common->setCommentStyle((CodeGenerationPolicy::CommentStyle) form.m_SelectCommentStyle->currentIndex());
    common->setAttributeAccessorScope((Uml::Visibility::Value) form.m_accessorScopeCB->currentIndex());
    common->setAssociationFieldScope((Uml::Visibility::Value) form.m_assocFieldScopeCB->currentIndex());
    common->setAutoGenerateConstructors(form.m_generateConstructors->isChecked());
    parent->setAutoGenerateAttribAccessors(form.m_generateAttribAccessors->isChecked());
    parent->setAutoGenerateAssocAccessors(form.m_generateAssocAccessors->isChecked());

    parent->blockSignals(false);

    // now send out modified code content signal
    common->emitModifiedCodeContentSig();
}

#include "rubycodegenerationpolicypage.moc"
