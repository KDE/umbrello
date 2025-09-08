/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlforeignkeyconstraintdialog.h"

#include "umlattribute.h"
#include "umlclassifier.h"
#include "umlclassifierlistitem.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "umlentityattribute.h"
#include "enumliteral.h"
#include "umlenum.h"
#include "entity.h"
#include "foreignkeyconstraint.h"
#include "object_factory.h"
#include "operation.h"
#include "template.h"
#include "uml.h"
#include "umldoc.h"
#include "umlentitylist.h"
#include "umlmessagebox.h"
#include "uniqueconstraint.h"
#include "icon_utils.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <KLocalizedString>

#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

DEBUG_REGISTER(UMLForeignKeyConstraintDialog)

/**
 *  Sets up the UMLForeignKeyConstraintDialog
 *
 *  @param parent   The parent to the UMLForeignKeyConstraintDialog.
 *  @param pForeignKeyConstraint The Unique Constraint to show the properties of
 */
UMLForeignKeyConstraintDialog::UMLForeignKeyConstraintDialog(QWidget* parent, UMLForeignKeyConstraint* pForeignKeyConstraint)
  : MultiPageDialogBase(parent),
    m_doc(UMLApp::app()->document()),
    m_pForeignKeyConstraint(pForeignKeyConstraint)
{
    setCaption(i18n("Foreign Key Setup"));
    setupGeneralPage();
    setupColumnPage();

    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

/**
 * Standard destructor.
 */
UMLForeignKeyConstraintDialog::~UMLForeignKeyConstraintDialog()
{
}

/**
 * Adds pair to the list.
 */
void UMLForeignKeyConstraintDialog::slotAddPair()
{
    // get the index of the selected local column and referenced column
    int indexL = m_ColumnWidgets.localColumnCB->currentIndex();
    int indexR = m_ColumnWidgets.referencedColumnCB->currentIndex();

    if (indexL == -1 || indexR == -1) {
        return;
    }

    // local entity attribute
    UMLEntityAttribute* localColumn = m_pLocalAttributeList.at(indexL);
    // referenced entity attribute
    UMLEntityAttribute* referencedColumn = m_pReferencedAttributeList.at(indexR);

    // remove from combo boxes
    m_ColumnWidgets.localColumnCB->removeItem(indexL);
    m_ColumnWidgets.referencedColumnCB->removeItem(indexR);

    // remove from local cache
    m_pLocalAttributeList.removeAt(indexL);
    m_pReferencedAttributeList.removeAt(indexR);

    // add to local cache of mapping
    EntityAttributePair pair = qMakePair(localColumn, referencedColumn);
    m_pAttributeMapList.append(pair);
    // update mapping view

    QTreeWidgetItem* mapping = new QTreeWidgetItem(m_ColumnWidgets.mappingTW);
    mapping->setText(0, localColumn->toString(Uml::SignatureType::SigNoVis));
    mapping->setText(1, referencedColumn->toString(Uml::SignatureType::SigNoVis));

    m_ColumnWidgets.mappingTW->addTopLevelItem(mapping);

    slotResetWidgetState();
}

/**
 * Deletes a pair from the list.
 */
void UMLForeignKeyConstraintDialog::slotDeletePair()
{
    // get the index of the selected pair in the view
    QTreeWidgetItem* twi = m_ColumnWidgets.mappingTW->currentItem();
    int indexP = m_ColumnWidgets.mappingTW->indexOfTopLevelItem(twi);
    if (indexP == -1) {
        return;
    }

    //find pair in local cache
    EntityAttributePair pair = m_pAttributeMapList.at(indexP);

    // remove them from the view and the list
    m_ColumnWidgets.mappingTW->takeTopLevelItem(indexP);
    m_pAttributeMapList.removeAt(indexP);

    // add the attributes to the local caches
    m_pLocalAttributeList.append(pair.first);
    m_pReferencedAttributeList.append(pair.second);

    // add them to the view (combo boxes)
    logDebug2("UMLForeignKeyConstraintDialog::slotDeletePair: %1 %2",
              pair.first->name(), pair.second->name());
    m_ColumnWidgets.localColumnCB->addItem((pair.first)->toString(Uml::SignatureType::SigNoVis));
    m_ColumnWidgets.referencedColumnCB->addItem((pair.second)->toString(Uml::SignatureType::SigNoVis));

    for(const EntityAttributePair& p : m_pAttributeMapList) {
        logDebug4("UMLForeignKeyConstraintDialog::slotDeletePair: AttributeMapList %1 %2 / %3 %4",
                  p.first->name(), p.first->baseType(), p.second->name(), p.second->baseType());
    }

    slotResetWidgetState();
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false.
 */
bool UMLForeignKeyConstraintDialog::apply()
{
    // set the Referenced Entity
    QString entityName = m_GeneralWidgets.referencedEntityCB->currentText();
    UMLObject* uo = m_doc->findUMLObjectRecursive(Uml::ModelType::EntityRelationship,
                                                  entityName,
                                                  UMLObject::ot_Entity);

    UMLEntity* ue = uo->asUMLEntity();

    if (ue == nullptr) {
        logDebug1("UMLForeignKeyConstraintDialog::apply: Could not find UML Entity with name %1",
                  entityName);
        return false;
    }

    m_pForeignKeyConstraint->setReferencedEntity(ue);

    // set all the update and delete actions
    UMLForeignKeyConstraint::UpdateDeleteAction updateAction, deleteAction;
    updateAction = (UMLForeignKeyConstraint::UpdateDeleteAction) m_GeneralWidgets.updateActionCB->currentIndex();
    deleteAction = (UMLForeignKeyConstraint::UpdateDeleteAction) m_GeneralWidgets.deleteActionCB->currentIndex();
    m_pForeignKeyConstraint->setUpdateAction(updateAction);
    m_pForeignKeyConstraint->setDeleteAction(deleteAction);

    // remove all existing mappings first
    m_pForeignKeyConstraint->clearMappings();

    // add all mappings  present in local cache
    for(const EntityAttributePair& pair : m_pAttributeMapList) {
        if (!m_pForeignKeyConstraint->addEntityAttributePair(pair.first, pair.second)) {
            return false;
        }
    }

    // set the name
    m_pForeignKeyConstraint->setName(m_GeneralWidgets.nameT->text());

    // propagate changes to tree view
    m_pForeignKeyConstraint->emitModified();

    return true;
}

/**
 * Setup the General Page.
 */
void UMLForeignKeyConstraintDialog::setupGeneralPage()
{
    //setup General page
    QWidget* page = new QWidget();
    QVBoxLayout* topLayout = new QVBoxLayout();
    page->setLayout(topLayout);
    int margin = fontMetrics().height();

    pageGeneral = createPage(i18nc("general page title", "General"), i18n("General Settings"),
                             Icon_Utils::it_Properties_General, page);

    m_GeneralWidgets.generalGB = new QGroupBox(i18nc("general group title", "General"));
    topLayout->addWidget(m_GeneralWidgets.generalGB);

    QGridLayout* generalLayout = new QGridLayout(m_GeneralWidgets.generalGB);
    generalLayout->setSpacing(Dialog_Utils::spacingHint());
    generalLayout->setContentsMargins(margin, margin, margin, margin);

    Dialog_Utils::makeLabeledEditField(generalLayout, 0,
                                       m_GeneralWidgets.nameL, i18nc("label for entering name", "Name"),
                                       m_GeneralWidgets.nameT);

    m_GeneralWidgets.referencedEntityL = new QLabel(i18n("Referenced Entity"));
    generalLayout->addWidget(m_GeneralWidgets.referencedEntityL, 1, 0);

    m_GeneralWidgets.referencedEntityCB = new KComboBox();
    generalLayout->addWidget(m_GeneralWidgets.referencedEntityCB, 1, 1);

    m_GeneralWidgets.actionGB = new QGroupBox(i18n("Actions"));
    topLayout->addWidget(m_GeneralWidgets.actionGB);

    QGridLayout* actionLayout = new QGridLayout(m_GeneralWidgets.actionGB);
    actionLayout->setSpacing(Dialog_Utils::spacingHint());
    actionLayout->setContentsMargins(margin, margin, margin, margin);

    m_GeneralWidgets.onUpdateL = new QLabel(i18n("On Update"));
    actionLayout->addWidget(m_GeneralWidgets.onUpdateL, 0, 0);

    m_GeneralWidgets.updateActionCB = new KComboBox(page);
    actionLayout->addWidget(m_GeneralWidgets.updateActionCB, 0, 1);

    m_GeneralWidgets.onDeleteL = new QLabel(i18n("On Delete"));
    actionLayout->addWidget(m_GeneralWidgets.onDeleteL, 1, 0);

    m_GeneralWidgets.deleteActionCB = new KComboBox();
    actionLayout->addWidget(m_GeneralWidgets.deleteActionCB, 1, 1);

    // set the name
    m_GeneralWidgets.nameT->setText(m_pForeignKeyConstraint->name());

    // fill up the combo boxes

    // reference entity combo box
    UMLEntityList entList = m_doc->entities();

    for(UMLEntity* ent : entList) {
        m_GeneralWidgets.referencedEntityCB->addItem(ent->name());
    }

    UMLEntity* referencedEntity = m_pForeignKeyConstraint->getReferencedEntity();

    int index;
    if (referencedEntity != nullptr) {
        index = m_GeneralWidgets.referencedEntityCB->findText(referencedEntity->name());
        if (index != -1)
            m_GeneralWidgets.referencedEntityCB->setCurrentIndex(index);
    }
    m_pReferencedEntityIndex = m_GeneralWidgets.referencedEntityCB->currentIndex();

    // action combo boxes

    // do not change order. It is according to enum specification in foreignkeyconstraint.h
    QStringList actions;
    actions << i18n("No Action") << i18n("Restrict") << i18n("Cascade") << i18n("Set Null")
            << i18n("Set Default");

    m_GeneralWidgets.updateActionCB->addItems(actions);
    m_GeneralWidgets.deleteActionCB->addItems(actions);

    m_GeneralWidgets.updateActionCB->setCurrentIndex(m_pForeignKeyConstraint->getUpdateAction());
    m_GeneralWidgets.deleteActionCB->setCurrentIndex(m_pForeignKeyConstraint->getDeleteAction());

    connect(m_GeneralWidgets.referencedEntityCB, SIGNAL(activated(int)), this, SLOT(slotReferencedEntityChanged(int)));
}

/**
 * Setup Column Page.
 */
void UMLForeignKeyConstraintDialog::setupColumnPage()
{
    //setup Columns page
    QWidget* page = new QWidget();
    QVBoxLayout* topLayout = new QVBoxLayout();
    page->setLayout(topLayout);

    pageColumn = createPage(i18n("Columns"), i18n("Columns"),
                            Icon_Utils::it_Properties_Columns, page);

    m_ColumnWidgets.mappingTW = new QTreeWidget();
    topLayout->addWidget(m_ColumnWidgets.mappingTW);

    QStringList headers;
    headers << i18nc("column header local", "Local") << i18nc("column header referenced", "Referenced");
    m_ColumnWidgets.mappingTW->setHeaderLabels(headers);

    QWidget* columns = new QWidget();
    topLayout->addWidget(columns);
    QGridLayout* columnsLayout = new QGridLayout(columns);

    m_ColumnWidgets.localColumnL = new QLabel(i18n("Local Column"));
    columnsLayout->addWidget(m_ColumnWidgets.localColumnL, 0, 0);

    m_ColumnWidgets.localColumnCB = new KComboBox();
    columnsLayout->addWidget(m_ColumnWidgets.localColumnCB, 0, 1);

    m_ColumnWidgets.referencedColumnL = new QLabel(i18n("Referenced Column"));
    columnsLayout->addWidget(m_ColumnWidgets.referencedColumnL, 1, 0);

    m_ColumnWidgets.referencedColumnCB = new KComboBox();
    columnsLayout->addWidget(m_ColumnWidgets.referencedColumnCB, 1, 1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox();
    m_ColumnWidgets.addPB = buttonBox->addButton(i18n("&Add"), QDialogButtonBox::ActionRole);
    connect(m_ColumnWidgets.addPB, SIGNAL(clicked()), this, SLOT(slotAddPair()));
    m_ColumnWidgets.removePB = buttonBox->addButton(i18n("&Delete"), QDialogButtonBox::ActionRole);
    connect(m_ColumnWidgets.removePB, SIGNAL(clicked()), this, SLOT(slotDeletePair()));

    columnsLayout->addWidget(buttonBox, 2, 1);

    // fill the column boxes and their local cache.
    refillLocalAttributeCB();
    refillReferencedAttributeCB();

    QMap<UMLEntityAttribute*, UMLEntityAttribute*>::iterator i;
    QMap<UMLEntityAttribute*, UMLEntityAttribute*>  map = m_pForeignKeyConstraint->getEntityAttributePairs();
    for (i = map.begin(); i != map.end() ; ++i) {

        UMLEntityAttribute* localColumn, *referencedColumn;

        localColumn = const_cast<UMLEntityAttribute*>(i.key());
        referencedColumn = const_cast<UMLEntityAttribute*>(i.value());

        // remove these columns from local cache
        int indexL = m_pLocalAttributeList.indexOf(localColumn);
        int indexR = m_pReferencedAttributeList.indexOf(referencedColumn);

        m_pLocalAttributeList.removeAt(indexL);
        m_pReferencedAttributeList.removeAt(indexR);

        // remove them from combo boxes
        // the conditions may never be violated . Just for safety though
        if (indexL >= 0 && indexL < (m_ColumnWidgets.localColumnCB)->count())
            m_ColumnWidgets.localColumnCB->removeItem(indexL);
        if (indexR >= 0 && indexR < (m_ColumnWidgets.referencedColumnCB)->count())
            m_ColumnWidgets.referencedColumnCB->removeItem(indexR);

        // add to local cache
        m_pAttributeMapList.append(qMakePair(localColumn, referencedColumn));

        // add to view
        QTreeWidgetItem* mapping = new QTreeWidgetItem(m_ColumnWidgets.mappingTW);
        mapping->setText(0, localColumn->toString(Uml::SignatureType::SigNoVis));
        mapping->setText(1, referencedColumn->toString(Uml::SignatureType::SigNoVis));

        m_ColumnWidgets.mappingTW->insertTopLevelItem(0, mapping);
    }

    slotResetWidgetState();

    connect(m_ColumnWidgets.mappingTW, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotResetWidgetState()));
}

/**
 * Used when the Apply button is clicked.
 */
void UMLForeignKeyConstraintDialog::slotApply()
{
    apply();
}

/**
 * Used when the OK button is clicked.  Calls apply().
 */
void UMLForeignKeyConstraintDialog::slotOk()
{
    if (apply()) {
        accept();
    }
}

void UMLForeignKeyConstraintDialog::slotReferencedEntityChanged(int index)
{
    if (index == m_pReferencedEntityIndex) {
        return;
    }

    if (!m_pAttributeMapList.empty()) {
        int result = UmlMessageBox::questionYesNo(this, i18n("You are attempting to change the Referenced Entity of this ForeignKey Constraint. Any unapplied changes to the mappings between local and referenced entities will be lost. Are you sure you want to continue ?"));

        if (result != UmlMessageBox::Yes) {
            // revert back to old index
            m_GeneralWidgets.referencedEntityCB->setCurrentIndex(m_pReferencedEntityIndex);
            return;
        }
    }

    // set the referenced entity index to current index
    m_pReferencedEntityIndex = index;
    m_ColumnWidgets.mappingTW->clear();
    refillReferencedAttributeCB();
    refillLocalAttributeCB();
}

void UMLForeignKeyConstraintDialog::refillReferencedAttributeCB()
{
    m_pReferencedAttributeList.clear();
    m_ColumnWidgets.referencedColumnCB->clear();

    // fill the combo boxes

    UMLObject* uo = m_doc->findUMLObjectRecursive(Uml::ModelType::EntityRelationship,
                                                  m_GeneralWidgets.referencedEntityCB->currentText(),
                                                  UMLObject::ot_Entity);

    const UMLEntity* ue = uo->asUMLEntity();

    if (ue) {
        UMLClassifierListItemList ual = ue->getFilteredList(UMLObject::ot_EntityAttribute);
        for(UMLClassifierListItem* att : ual) {
            m_pReferencedAttributeList.append(att->asUMLEntityAttribute());
            m_ColumnWidgets.referencedColumnCB->addItem(att->toString(Uml::SignatureType::SigNoVis));
        }
    }
}

void UMLForeignKeyConstraintDialog::refillLocalAttributeCB()
{
    m_pLocalAttributeList.clear();
    m_ColumnWidgets.localColumnCB->clear();
    // fill the combo boxes
    const UMLEntity* ue = m_pForeignKeyConstraint->umlParent()->asUMLEntity();

    if (ue) {
        UMLClassifierListItemList ual = ue->getFilteredList(UMLObject::ot_EntityAttribute);
        for(UMLClassifierListItem* att : ual) {
            m_pLocalAttributeList.append(att->asUMLEntityAttribute());
            m_ColumnWidgets.localColumnCB->addItem(att->toString(Uml::SignatureType::SigNoVis));
        }
    }
}

/**
 * Enable/Disable the widgets in the Dialog Box.
 */
void UMLForeignKeyConstraintDialog::slotResetWidgetState()
{
    m_ColumnWidgets.addPB->setEnabled(true);
    m_ColumnWidgets.removePB->setEnabled(true);
    m_ColumnWidgets.localColumnCB->setEnabled(true);
    m_ColumnWidgets.referencedColumnCB->setEnabled(true);

    // If one of the Combo Boxes is empty, then disable the Combo Box
    if (m_ColumnWidgets.localColumnCB->count() == 0 || m_ColumnWidgets.referencedColumnCB->count() == 0) {
        m_ColumnWidgets.localColumnCB->setEnabled(false);
        m_ColumnWidgets.referencedColumnCB->setEnabled(false);
        m_ColumnWidgets.addPB->setEnabled(false);
    }

    // get index of selected Attribute in List Box
    if (m_ColumnWidgets.mappingTW->currentItem() == nullptr) {
        m_ColumnWidgets.removePB->setEnabled(false);
    }
}


