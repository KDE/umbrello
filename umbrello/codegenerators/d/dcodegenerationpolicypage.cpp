/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dcodegenerationpolicypage.h"

// app includes
#include "umlapp.h"

// kde includes
#include <KLocalizedString>

DCodeGenerationPolicyPage::DCodeGenerationPolicyPage(QWidget *parent, const char *name, DCodeGenerationPolicy * policy)
  : CodeGenerationPolicyPage(parent, name, policy)
{
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    form = new DCodeGenerationFormBase(this);
    form->m_SelectCommentStyle->setCurrentIndex((int)(commonPolicy->getCommentStyle()));
    form->m_generateConstructors->setChecked(commonPolicy->getAutoGenerateConstructors());
    form->m_generateAttribAccessors->setChecked(policy->getAutoGenerateAttribAccessors());
    form->m_generateAssocAccessors->setChecked(policy->getAutoGenerateAssocAccessors());
    form->m_accessorScopeCB->setCurrentIndex(commonPolicy->getAttributeAccessorScope());
    form->m_assocFieldScopeCB->setCurrentIndex(commonPolicy->getAssociationFieldScope());

    /**
     * @todo unclean - CreateANTBuildFile attribute should be in d policy
    CodeGenerator *codegen = UMLApp::app()->getGenerator();
    DCodeGenerator *dcodegen = dynamic_cast<DCodeGenerator*>(codegen);
    if (dcodegen)
        form->m_makeANTDocumentCheckBox->setChecked(dcodegen->getCreateANTBuildFile());
     */
}

DCodeGenerationPolicyPage::~DCodeGenerationPolicyPage()
{
}

void DCodeGenerationPolicyPage::apply()
{
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    DCodeGenerationPolicy * parent = (DCodeGenerationPolicy*) m_parentPolicy;

    // block signals so we don't cause too many update content calls to code documents
    commonPolicy->blockSignals(true);

    commonPolicy->setCommentStyle((CodeGenerationPolicy::CommentStyle) form->m_SelectCommentStyle->currentIndex());
    commonPolicy->setAttributeAccessorScope(Uml::Visibility::fromInt(form->m_accessorScopeCB->currentIndex()));
    commonPolicy->setAssociationFieldScope(Uml::Visibility::fromInt(form->m_assocFieldScopeCB->currentIndex()));
    commonPolicy->setAutoGenerateConstructors(form->m_generateConstructors->isChecked());
    parent->setAutoGenerateAttribAccessors(form->m_generateAttribAccessors->isChecked());
    parent->setAutoGenerateAssocAccessors(form->m_generateAssocAccessors->isChecked());

    /**
     * @todo unclean - CreateANTBuildFile attribute should be in d policy
    CodeGenerator *codegen = UMLApp::app()->getGenerator();
    DCodeGenerator *dcodegen = dynamic_cast<DCodeGenerator*>(codegen);
    if (dcodegen)
        dcodegen->setCreateANTBuildFile(form->m_makeANTDocumentCheckBox->isChecked());
     */
    commonPolicy->blockSignals(false);

    // now send out modified code content signal
    commonPolicy->emitModifiedCodeContentSig();
}

