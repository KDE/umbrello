/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cppcodegenerationpolicypage.h"

// app includes
#include "debug_utils.h"
#include "uml.h"

// kde includes
#include <klocale.h>
#include <kcombobox.h>

// qt includes
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>

CPPCodeGenerationPolicyPage::CPPCodeGenerationPolicyPage( QWidget *parent, const char *name, CPPCodeGenerationPolicy * policy )
  : CodeGenerationPolicyPage(parent, name, policy)
{
    CodeGenerationPolicy *common = UMLApp::app()->commonPolicy();

    QVBoxLayout* vboxLayout = new QVBoxLayout( this );

    form = new CPPCodeGenerationForm(this);
    form->ui_selectCommentStyle->setCurrentIndex((int)(common->getCommentStyle()));
    form->setPackageIsANamespace(policy->getPackageIsNamespace());
    form->setVirtualDestructors(policy->getDestructorsAreVirtual());
    form->setGenerateAccessorMethods(policy->getAutoGenerateAccessors());
    form->setGenerateEmptyConstructors(common->getAutoGenerateConstructors());
    form->setOperationsAreInline(policy->getOperationsAreInline());
    form->setAccessorsAreInline(policy->getAccessorsAreInline());
    form->setAccessorsArePublic(policy->getAccessorsArePublic());
    form->setDocToolTag(policy->getDocToolTag());

    form->ui_stringClassHCombo->setCurrentItem(policy->getStringClassName(),true);
    form->ui_listClassHCombo->setCurrentItem(policy->getVectorClassName(),true);

    form->ui_stringIncludeFileHistoryCombo->setCurrentItem(policy->getStringClassNameInclude(),true);
    form->ui_listIncludeFileHistoryCombo->setCurrentItem(policy->getVectorClassNameInclude(),true);

    form->ui_globalStringCheckBox->setChecked(policy->stringIncludeIsGlobal());
    form->ui_globalListCheckBox->setChecked(policy->vectorIncludeIsGlobal());

    vboxLayout->addWidget( form );
}

CPPCodeGenerationPolicyPage::~CPPCodeGenerationPolicyPage()
{
}

void CPPCodeGenerationPolicyPage::apply()
{
    CodeGenerationPolicy *common = UMLApp::app()->commonPolicy();

    // now do our cpp-specific configs
    CPPCodeGenerationPolicy * parent = (CPPCodeGenerationPolicy*) m_parentPolicy;

    // block signals so that we don't generate too many sync signals for child code
    // documents
    parent->blockSignals(true);

    common->setCommentStyle((CodeGenerationPolicy::CommentStyle ) form->ui_selectCommentStyle->currentIndex());
    common->setAutoGenerateConstructors(form->getGenerateEmptyConstructors());
    parent->setAutoGenerateAccessors(form->getGenerateAccessorMethods());
    uDebug() << form->getGenerateAccessorMethods();

    parent->setDestructorsAreVirtual(form->getVirtualDestructors());
    parent->setPackageIsNamespace(form->getPackageIsANamespace());
    parent->setAccessorsAreInline(form->getAccessorsAreInline());
    parent->setOperationsAreInline(form->getOperationsAreInline());
    parent->setAccessorsArePublic(form->getAccessorsArePublic());
    parent->setDocToolTag(form->getDocToolTag());

    parent->setStringClassName(form->ui_stringClassHCombo->currentText());
    parent->setStringClassNameInclude(form->ui_stringIncludeFileHistoryCombo->currentText());
    parent->setStringIncludeIsGlobal(form->ui_globalStringCheckBox->isChecked());

    parent->setVectorClassName(form->ui_listClassHCombo->currentText());
    parent->setVectorClassNameInclude(form->ui_listIncludeFileHistoryCombo->currentText());
    parent->setVectorIncludeIsGlobal(form->ui_globalListCheckBox->isChecked());

    parent->blockSignals(false);

    // now send out modified code content signal
    common->emitModifiedCodeContentSig();
}


#include "cppcodegenerationpolicypage.moc"
