/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "umloperationdialog.h"

//app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "operation.h"
#include "classifier.h"
#include "template.h"
#include "listpopupmenu.h"
#include "umlattributelist.h"
#include "umldatatypewidget.h"
#include "umlstereotypewidget.h"
#include "classifierlistitem.h"
#include "documentationwidget.h"
#include "umlclassifierlistitemlist.h"
#include "dialog_utils.h"
#include "parameterpropertiesdialog.h"
#include "stereotype.h"
#include "uniqueid.h"
#include "visibilityenumwidget.h"

//kde includes
#include <KLocalizedString>
#include <KMessageBox>

/**
 * Constructor.
 */
UMLOperationDialog::UMLOperationDialog(QWidget * parent, UMLOperation * pOperation)
  : SinglePageDialogBase(parent)
  , ui(new Ui::UMLOperationDialog)
{
    setCaption(i18n("Operation Properties"));
    ui->setupUi(mainWidget());
    m_operation = pOperation;
    m_doc = UMLApp::app()->document();
    m_menu = 0;
    setupDialog();
}

/**
 * Destructor.
 */
UMLOperationDialog::~UMLOperationDialog()
{
}

/**
 * Sets up the dialog.
 */
void UMLOperationDialog::setupDialog()
{
    ui->dataTypeWidget->setOperation(m_operation);
    ui->stereotypeWidget->setUMLObject(m_operation);
    ui->visibilityWidget->setUMLObject(m_operation);
    ui->documentationWidget->setUMLObject(m_operation);
    ui->parametersListLW->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->nameLE->setText(m_operation->name());
    ui->abstractCB->setChecked(m_operation->isAbstract());
    ui->classifierScopeCB->setChecked(m_operation->isStatic());
    ui->queryConstCB->setChecked(m_operation->getConst());

    // setup push buttons signals
    connect(ui->newParameterPB, &QPushButton::clicked, this, &UMLOperationDialog::slotNewParameter);
    connect(ui->deletePB, &QPushButton::clicked, this, &UMLOperationDialog::slotDeleteParameter);
    connect(ui->propertiesPB, &QPushButton::clicked, this, &UMLOperationDialog::slotParameterProperties);

    ui->deletePB->setEnabled(false);
    ui->propertiesPB->setEnabled(false);
    ui->downTB->setEnabled(false);
    ui->upTB->setEnabled(false);

    // fill in parm list box
    UMLAttributeList list = m_operation->getParmList();
    foreach (UMLAttribute* pAtt, list) {
        ui->parametersListLW->addItem(pAtt->toString(Uml::SignatureType::SigNoVis));
    }
    connect(ui->upTB, &QToolButton::clicked, this, &UMLOperationDialog::slotParameterUp);
    connect(ui->downTB, &QToolButton::clicked, this, &UMLOperationDialog::slotParameterDown);

    connect(ui->parametersListLW, &QListWidget::itemClicked, this, &UMLOperationDialog::slotParamsBoxClicked);
    connect(ui->parametersListLW, &QListWidget::customContextMenuRequested, this, &UMLOperationDialog::slotParmRightButtonPressed);
    connect(ui->parametersListLW, &QListWidget::itemDoubleClicked, this, &UMLOperationDialog::slotParmDoubleClick);

    ui->nameLE->setFocus();

    connect(ui->nameLE, &QLineEdit::textChanged, this, &UMLOperationDialog::slotNameChanged);
}

void UMLOperationDialog::slotNameChanged(const QString &_text)
{
    enableButtonOk(!_text.isEmpty());
}

void UMLOperationDialog::slotParmRightButtonPressed(const QPoint &p)
{
    ListPopupMenu::MenuType type = ListPopupMenu::mt_Undefined;
    QListWidgetItem* item = ui->parametersListLW->itemAt(p);
    if (item) // pressed on an item
        type = ListPopupMenu::mt_Parameter_Selected;
    else // pressed into fresh air
        type = ListPopupMenu::mt_New_Parameter;

    if (m_menu) {
        m_menu->hide();
        disconnect(m_menu, &ListPopupMenu::triggered, this, &UMLOperationDialog::slotMenuSelection);
        delete m_menu;
        m_menu = 0;
    }
    ListPopupMenu popup(this, type);
    QAction *triggered = popup.exec(ui->parametersListLW->mapToGlobal(p));
    slotMenuSelection(triggered);
}

void UMLOperationDialog::slotParmDoubleClick(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    // this happens, when there was no right click in the list widget
    ListPopupMenu popup(this, ListPopupMenu::mt_Parameter_Selected);
    QAction* action = popup.getAction(ListPopupMenu::mt_Properties);
    slotMenuSelection(action);
}

void UMLOperationDialog::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType id = ListPopupMenu::typeFromAction(action);
    if(id == ListPopupMenu::mt_Rename || id == ListPopupMenu::mt_Properties) {
        slotParameterProperties();
    } else if(id == ListPopupMenu::mt_New_Parameter) {
        slotNewParameter();
    }
    else if(id == ListPopupMenu::mt_Delete) {
        slotDeleteParameter();
    }
}

void UMLOperationDialog::slotNewParameter()
{
    UMLAttribute* pAtt = 0;

    QString currentName = m_operation->getUniqueParameterName();
    UMLAttribute* newAttribute = new UMLAttribute(m_operation, currentName, Uml::ID::Reserved);

    QPointer<ParameterPropertiesDialog> dlg = new ParameterPropertiesDialog(this, m_doc, newAttribute);
    if (dlg->exec()) {
        pAtt = m_operation->findParm(newAttribute->name());

        if (!pAtt) {
            newAttribute->setID(UniqueID::gen());
            m_operation->addParm(newAttribute);
            ui->parametersListLW->addItem(newAttribute->toString(Uml::SignatureType::SigNoVis));
            m_doc->setModified(true);
        } else {
            KMessageBox::sorry(this, i18n("The parameter name you have chosen\nis already being used in this operation."),
                               i18n("Parameter Name Not Unique"), 0);
            delete newAttribute;
        }
    } else {
        delete newAttribute;
    }
    delete dlg;
}

void UMLOperationDialog::slotDeleteParameter()
{
    UMLAttribute* pOldAtt = m_operation->getParmList().at(ui->parametersListLW->row(ui->parametersListLW->currentItem()));

    m_operation->removeParm(pOldAtt);
    ui->parametersListLW->takeItem(ui->parametersListLW->currentRow());
    m_doc->setModified(true);

    ui->deletePB->setEnabled(false);
    ui->propertiesPB->setEnabled(false);
    ui->upTB->setEnabled(false);
    ui->downTB->setEnabled(false);
}

void UMLOperationDialog::slotParameterProperties()
{
    UMLAttribute* pAtt = 0, * pOldAtt = 0;

    int position = ui->parametersListLW->row(ui->parametersListLW->currentItem());
    pOldAtt = m_operation->getParmList().at(position);
    if (!pOldAtt) {
        uDebug() << "THE impossible has occurred for:" << ui->parametersListLW->currentItem()->text();
        return;
    } // should never occur

    QString oldAttName = pOldAtt->name();
    UMLAttribute* tempAttribute = pOldAtt->clone()->asUMLAttribute(); // create a clone of the parameter

    // send the clone to the properties dialog box. it will fill in the new parameters.
    QPointer<ParameterPropertiesDialog> dlg = new ParameterPropertiesDialog(this, m_doc, tempAttribute);
    if (dlg->exec()) {
        bool namingConflict = false;
        QString newName = tempAttribute->name();

        pAtt = m_operation->findParm(newName); // search whether a parameter with this name already exists
        if(pAtt && pAtt != pOldAtt) {
            KMessageBox::error(this, i18n("The parameter name you have chosen is already being used in this operation."),
                               i18n("Parameter Name Not Unique"), 0);
            namingConflict = true;
        }

        tempAttribute->copyInto(pOldAtt); // copy all attributes from the clone
        if (namingConflict) {
            pOldAtt->setName(oldAttName); // reset the name if there was a naming conflict
        }

        QListWidgetItem* item = ui->parametersListLW->currentItem();
        item->setText(pOldAtt->toString(Uml::SignatureType::SigNoVis));
        m_doc->setModified(true);
    }
    delete tempAttribute;
    delete dlg;
}

void UMLOperationDialog::slotParameterUp()
{
    int row = ui->parametersListLW->currentRow();
    QListWidgetItem* item = ui->parametersListLW->currentItem();
    if (item) {
        UMLAttribute* pOldAtt = m_operation->getParmList().at(ui->parametersListLW->row(item));

        m_operation->moveParmLeft(pOldAtt);
        ui->parametersListLW->takeItem(row);
        ui->parametersListLW->insertItem(row - 1, item);

        m_doc->setModified(true);
        slotParamsBoxClicked(item);
    }
    else {
        uDebug() << "No current item in list widget!?";
    }
}

void UMLOperationDialog::slotParameterDown()
{
    int row = ui->parametersListLW->currentRow();
    QListWidgetItem* item = ui->parametersListLW->currentItem();
    if (item) {
        UMLAttribute* pOldAtt = m_operation->getParmList().at(ui->parametersListLW->row(item));

        m_operation->moveParmRight(pOldAtt);
        ui->parametersListLW->takeItem(row);
        ui->parametersListLW->insertItem(row + 1, item);

        m_doc->setModified(true);
        slotParamsBoxClicked(item);
    }
    else {
        uDebug() << "No current item in list widget!?";
    }
}

/**
 * Enables or disables buttons.
 */
void UMLOperationDialog::slotParamsBoxClicked(QListWidgetItem* parameterItem)
{
    if (parameterItem) {
        ui->deletePB->setEnabled(true);
        ui->propertiesPB->setEnabled(true);
        int row = ui->parametersListLW->row(parameterItem);
        bool hasNext = (row < ui->parametersListLW->count() - 1);
        bool hasPrev = (row > 0);
        ui->upTB->setEnabled(hasPrev);
        ui->downTB->setEnabled(hasNext);
    } else {
        ui->deletePB->setEnabled(false);
        ui->propertiesPB->setEnabled(false);
        ui->upTB->setEnabled(false);
        ui->downTB->setEnabled(false);
    }
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false.
 */
bool UMLOperationDialog::apply()
{
    QString name = ui->nameLE->text();
    if(name.length() == 0) {
        KMessageBox::error(this, i18n("You have entered an invalid operation name."),
                           i18n("Operation Name Invalid"), 0);
        ui->nameLE->setText(m_operation->name());
        return false;
    }

    UMLClassifier *classifier = m_operation->umlParent()->asUMLClassifier();
    if(classifier != 0 &&
            classifier->checkOperationSignature(name, m_operation->getParmList(), m_operation))
    {
        QString msg = i18n("An operation with that signature already exists in %1.\n", classifier->name())
                      +
                      i18n("Choose a different name or parameter list.");
        KMessageBox::error(this, msg, i18n("Operation Name Invalid"), 0);
        return false;
    }
    m_operation->setName(name);

    ui->visibilityWidget->apply();
    ui->dataTypeWidget->apply();
    ui->stereotypeWidget->apply();

    bool isAbstract = ui->abstractCB->isChecked();
    m_operation->setAbstract(isAbstract);
    if (isAbstract) {
        /* If any operation is abstract then the owning class needs
           to be made abstract.
           The inverse is not true: The fact that no operation is
           abstract does not mean that the class must be non-abstract.
         */
        if (classifier) {
            classifier->setAbstract(true);
        }
    }
    m_operation->setStatic(ui->classifierScopeCB->isChecked());
    m_operation->setConst(ui->queryConstCB->isChecked());
    ui->documentationWidget->apply();

    return true;
}


