/***************************************************************************
                          rubycodegenerationpolicypage.cpp
                          Derived from the Java code generator by thomas

    begin                : Thur Jul 21 2005
    author               : Richard Dale
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "rubycodegenerationpolicypage.h"
// qt/kde includes
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "rubycodegenerationformbase.h"
#include "rubycodegenerator.h"
#include "../uml.h"

RubyCodeGenerationPolicyPage::RubyCodeGenerationPolicyPage( QWidget *parent, const char *name, RubyCodeGenerationPolicy * policy )
        :CodeGenerationPolicyPage(parent,name,(CodeGenerationPolicy*)policy)
{
    form = new RubyCodeGenerationFormBase(this);
    form->m_SelectCommentStyle->setCurrentItem(commentTypeToInteger(policy->getCommentStyle()));
    form->m_generateConstructors->setChecked(policy->getAutoGenerateConstructors());
    form->m_generateAttribAccessors->setChecked(policy->getAutoGenerateAttribAccessors());
    form->m_generateAssocAccessors->setChecked(policy->getAutoGenerateAssocAccessors());
    form->m_accessorScopeCB->setCurrentItem((policy->getAttributeAccessorScope() - 200));
    form->m_assocFieldScopeCB->setCurrentItem((policy->getAssociationFieldScope() - 200));
}

RubyCodeGenerationPolicyPage::~RubyCodeGenerationPolicyPage()
{
}

int RubyCodeGenerationPolicyPage::commentTypeToInteger(RubyCodeGenerationPolicy::RubyCommentStyle type) {
    switch (type) {
    case RubyCodeGenerationPolicy::Hash:
        return 1;
    default:
    case RubyCodeGenerationPolicy::BeginEnd:
        return 0;
    }
}

void RubyCodeGenerationPolicyPage::apply()
{

    // now do our ruby-specific configs
    RubyCodeGenerationPolicy * parent = (RubyCodeGenerationPolicy*) m_parentPolicy;

    // block signals so we dont cause too many update content calls to code documents
    parent->blockSignals(true);

    parent->setCommentStyle((RubyCodeGenerationPolicy::RubyCommentStyle ) form->m_SelectCommentStyle->currentItem());
    parent->setAttributeAccessorScope((RubyCodeGenerationPolicy::ScopePolicy) (form->m_accessorScopeCB->currentItem()+200));
    parent->setAssociationFieldScope((RubyCodeGenerationPolicy::ScopePolicy) (form->m_assocFieldScopeCB->currentItem()+200));
    parent->setAutoGenerateConstructors(form->m_generateConstructors->isChecked());
    parent->setAutoGenerateAttribAccessors(form->m_generateAttribAccessors->isChecked());
    parent->setAutoGenerateAssocAccessors(form->m_generateAssocAccessors->isChecked());

    parent->blockSignals(false);

    // now send out modified code content signal
    parent->emitModifiedCodeContentSig();

}


#include "rubycodegenerationpolicypage.moc"
