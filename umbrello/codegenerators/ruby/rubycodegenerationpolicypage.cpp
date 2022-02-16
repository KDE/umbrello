/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "rubycodegenerationpolicypage.h"

// app includes
#include "codegenerationpolicy.h"
#include "uml.h"

// kde includes
#include <KLocalizedString>

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
    common->setAttributeAccessorScope(Uml::Visibility::fromInt(form.m_accessorScopeCB->currentIndex()));
    common->setAssociationFieldScope(Uml::Visibility::fromInt(form.m_assocFieldScopeCB->currentIndex()));
    common->setAutoGenerateConstructors(form.m_generateConstructors->isChecked());
    parent->setAutoGenerateAttribAccessors(form.m_generateAttribAccessors->isChecked());
    parent->setAutoGenerateAssocAccessors(form.m_generateAssocAccessors->isChecked());

    parent->blockSignals(false);

    // now send out modified code content signal
    common->emitModifiedCodeContentSig();
}

