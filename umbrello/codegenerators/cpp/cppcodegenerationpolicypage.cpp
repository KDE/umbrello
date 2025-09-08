/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cppcodegenerationpolicypage.h"

// app includes
#include "debug_utils.h"
#include "umlapp.h"

// kde includes
#include <KLocalizedString>
#include <kcombobox.h>

// qt includes
#include <QCheckBox>
#include <QLabel>

DEBUG_REGISTER(CPPCodeGenerationPolicyPage)

CPPCodeGenerationPolicyPage::CPPCodeGenerationPolicyPage(QWidget *parent, const char *name, CPPCodeGenerationPolicy * policy)
  : CodeGenerationPolicyPage(parent, name, policy)
{
    CodeGenerationPolicy *common = UMLApp::app()->commonPolicy();

    QVBoxLayout* vboxLayout = new QVBoxLayout(this);

    form = new CPPCodeGenerationForm(this);
    form->ui_selectCommentStyle->setCurrentIndex((int)(common->getCommentStyle()));
    form->setPackageIsANamespace(policy->getPackageIsNamespace());
    form->setVirtualDestructors(policy->getDestructorsAreVirtual());
    form->setGenerateAccessorMethods(policy->getAutoGenerateAccessors());
    form->setGenerateEmptyConstructors(common->getAutoGenerateConstructors());
    form->setOperationsAreInline(policy->getOperationsAreInline());
    form->setAccessorsAreInline(policy->getAccessorsAreInline());
    form->setAccessorsArePublic(policy->getAccessorsArePublic());
    form->setGetterWithoutGetPrefix(policy->getGetterWithGetPrefix());
    form->setRemovePrefixFromAccessorMethodName(policy->getRemovePrefixFromAccessorMethods());
    form->setAccessorMethodsStartWithUpperCase(policy->getAccessorMethodsStartWithUpperCase());
    form->setDocToolTag(policy->getDocToolTag());
    form->setClassMemberPrefix(policy->getClassMemberPrefix());

    form->ui_stringClassHCombo->setCurrentItem(policy->getStringClassName(), true);
    form->ui_listClassHCombo->setCurrentItem(policy->getVectorClassName(), true);

    form->ui_stringIncludeFileHistoryCombo->setCurrentItem(policy->getStringClassNameInclude(), true);
    form->ui_listIncludeFileHistoryCombo->setCurrentItem(policy->getVectorClassNameInclude(), true);

    form->ui_globalStringCheckBox->setChecked(policy->stringIncludeIsGlobal());
    form->ui_globalListCheckBox->setChecked(policy->vectorIncludeIsGlobal());

    vboxLayout->addWidget(form);
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

    common->setCommentStyle((CodeGenerationPolicy::CommentStyle) form->ui_selectCommentStyle->currentIndex());
    common->setAutoGenerateConstructors(form->getGenerateEmptyConstructors());
    parent->setAutoGenerateAccessors(form->getGenerateAccessorMethods());
    bool accMethodGen = form->getGenerateAccessorMethods();
    logDebug1("CPPCodeGenerationPolicyPage::apply: form->getGenerateAccessorMethods returns %1", accMethodGen);

    parent->setDestructorsAreVirtual(form->getVirtualDestructors());
    parent->setPackageIsNamespace(form->getPackageIsANamespace());
    parent->setAccessorsAreInline(form->getAccessorsAreInline());
    parent->setOperationsAreInline(form->getOperationsAreInline());
    parent->setAccessorsArePublic(form->getAccessorsArePublic());
    parent->setGetterWithGetPrefix(form->getGettersWithGetPrefix());
    parent->setRemovePrefixFromAccessorMethods(form->getRemovePrefixFromAccessorMethodName());
    parent->setAccessorMethodsStartWithUpperCase(form->getAccessorMethodsStartWithUpperCase());

    parent->setStringClassName(form->ui_stringClassHCombo->currentText());
    parent->setStringClassNameInclude(form->ui_stringIncludeFileHistoryCombo->currentText());
    parent->setStringIncludeIsGlobal(form->ui_globalStringCheckBox->isChecked());

    parent->setVectorClassName(form->ui_listClassHCombo->currentText());
    parent->setVectorClassNameInclude(form->ui_listIncludeFileHistoryCombo->currentText());
    parent->setVectorIncludeIsGlobal(form->ui_globalListCheckBox->isChecked());

    parent->setDocToolTag(form->getDocToolTag());
    parent->setClassMemberPrefix(form->getClassMemberPrefix());

    parent->blockSignals(false);

    // now send out modified code content signal
    common->emitModifiedCodeContentSig();
}


