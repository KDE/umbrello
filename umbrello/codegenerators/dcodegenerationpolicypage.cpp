
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "dcodegenerationpolicypage.h"
// qt/kde includes
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "../uml.h"

DCodeGenerationPolicyPage::DCodeGenerationPolicyPage( QWidget *parent, const char *name, DCodeGenerationPolicy * policy )
  : CodeGenerationPolicyPage(parent, name, policy)
{
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->getCommonPolicy();
    form = new DCodeGenerationFormBase(this);
    form->m_SelectCommentStyle->setCurrentItem((int)(commonPolicy->getCommentStyle()));
    form->m_generateConstructors->setChecked(commonPolicy->getAutoGenerateConstructors());
    form->m_generateAttribAccessors->setChecked(policy->getAutoGenerateAttribAccessors());
    form->m_generateAssocAccessors->setChecked(policy->getAutoGenerateAssocAccessors());
    form->m_accessorScopeCB->setCurrentItem(commonPolicy->getAttributeAccessorScope());
    form->m_assocFieldScopeCB->setCurrentItem(commonPolicy->getAssociationFieldScope());

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
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->getCommonPolicy();
    DCodeGenerationPolicy * parent = (DCodeGenerationPolicy*) m_parentPolicy;

    // block signals so we don't cause too many update content calls to code documents
    commonPolicy->blockSignals(true);

    commonPolicy->setCommentStyle((CodeGenerationPolicy::CommentStyle ) form->m_SelectCommentStyle->currentItem());
    commonPolicy->setAttributeAccessorScope((CodeGenerationPolicy::ScopePolicy) (form->m_accessorScopeCB->currentItem()));
    commonPolicy->setAssociationFieldScope((CodeGenerationPolicy::ScopePolicy) (form->m_assocFieldScopeCB->currentItem()));
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


#include "dcodegenerationpolicypage.moc"
