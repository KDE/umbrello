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
#if QT_VERSION < 0x050000
#include <kdialogbuttonbox.h>
#endif
#include <klineedit.h>
#include <kcombobox.h>
#include <KLocalizedString>
#include <KMessageBox>

//qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>
#include <QVBoxLayout>

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
    ui->stereotypeWidget->setUMLObject(m_operation);
    ui->visibilityWidget->setUMLObject(m_operation);
    ui->documentationWidget->setUMLObject(m_operation);
    ui->list_parameters->setContextMenuPolicy(Qt::CustomContextMenu);

    // setup push buttons signals
    connect(ui->pb_newParameter, &QPushButton::clicked, this, &UMLOperationDialog::slotNewParameter);
    connect(ui->pb_delete, &QPushButton::clicked, this, &UMLOperationDialog::slotDeleteParameter);
    connect(ui->pb_properties, &QPushButton::clicked, this, &UMLOperationDialog::slotParameterProperties);

    ui->pb_delete->setEnabled(false);
    ui->pb_properties->setEnabled(false);
    ui->pb_down->setEnabled(false);
    ui->pb_up->setEnabled(false);

    // fill in parm list box
    UMLAttributeList list = m_operation->getParmList();
    foreach (UMLAttribute* pAtt, list) {
        ui->list_parameters->addItem(pAtt->toString(Uml::SignatureType::SigNoVis));
    }
    connect(ui->pb_up, &QToolButton::clicked, this, &UMLOperationDialog::slotParameterUp);
    connect(ui->pb_down, &QToolButton::clicked, this, &UMLOperationDialog::slotParameterDown);

    connect(ui->list_parameters, &QListWidget::itemClicked, this, &UMLOperationDialog::slotParamsBoxClicked);
    connect(ui->list_parameters, &QListWidget::customContextMenuRequested, this, &UMLOperationDialog::slotParmRightButtonPressed);
    connect(ui->list_parameters, &QListWidget::itemDoubleClicked, this, &UMLOperationDialog::slotParmDoubleClick);

    ui->tb_name->setFocus();

    connect(ui->tb_name, &QLineEdit::textChanged, this, &UMLOperationDialog::slotNameChanged);
}

void UMLOperationDialog::slotNameChanged(const QString &_text)
{
    enableButtonOk(!_text.isEmpty());
}

void UMLOperationDialog::slotParmRightButtonPressed(const QPoint &p)
{
    ListPopupMenu::MenuType type = ListPopupMenu::mt_Undefined;
    QListWidgetItem* item = ui->list_parameters->itemAt(p);
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
    QAction *triggered = popup.exec(ui->list_parameters->mapToGlobal(p));
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
            ui->list_parameters->addItem(newAttribute->toString(Uml::SignatureType::SigNoVis));
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
    UMLAttribute* pOldAtt = m_operation->getParmList().at(ui->list_parameters->row(ui->list_parameters->currentItem()));

    m_operation->removeParm(pOldAtt);
    ui->list_parameters->takeItem(ui->list_parameters->currentRow());
    m_doc->setModified(true);

    ui->pb_delete->setEnabled(false);
    ui->pb_properties->setEnabled(false);
    ui->pb_up->setEnabled(false);
    ui->pb_down->setEnabled(false);
}

void UMLOperationDialog::slotParameterProperties()
{
    UMLAttribute* pAtt = 0, * pOldAtt = 0;

    int position = ui->list_parameters->row(ui->list_parameters->currentItem());
    pOldAtt = m_operation->getParmList().at(position);
    if (!pOldAtt) {
        uDebug() << "THE impossible has occurred for:" << ui->list_parameters->currentItem()->text();
        return;
    } // should never occur

    QString oldAttName = pOldAtt->name();
    UMLAttribute* tempAttribute = static_cast<UMLAttribute*>(pOldAtt->clone()); // create a clone of the parameter

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

        QListWidgetItem* item = ui->list_parameters->currentItem();
        item->setText(pOldAtt->toString(Uml::SignatureType::SigNoVis));
        m_doc->setModified(true);
    }
    delete tempAttribute;
    delete dlg;
}

void UMLOperationDialog::slotParameterUp()
{
    int row = ui->list_parameters->currentRow();
    QListWidgetItem* item = ui->list_parameters->currentItem();
    if (item) {
        UMLAttribute* pOldAtt = m_operation->getParmList().at(ui->list_parameters->row(item));

        m_operation->moveParmLeft(pOldAtt);
        ui->list_parameters->takeItem(row);
        ui->list_parameters->insertItem(row - 1, item);

        m_doc->setModified(true);
        slotParamsBoxClicked(item);
    }
    else {
        uDebug() << "No current item in list widget!?";
    }
}

void UMLOperationDialog::slotParameterDown()
{
    int row = ui->list_parameters->currentRow();
    QListWidgetItem* item = ui->list_parameters->currentItem();
    if (item) {
        UMLAttribute* pOldAtt = m_operation->getParmList().at(ui->list_parameters->row(item));

        m_operation->moveParmRight(pOldAtt);
        ui->list_parameters->takeItem(row);
        ui->list_parameters->insertItem(row + 1, item);

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
        ui->pb_delete->setEnabled(true);
        ui->pb_properties->setEnabled(true);
        int row = ui->list_parameters->row(parameterItem);
        bool hasNext = (row < ui->list_parameters->count() - 1);
        bool hasPrev = (row > 0);
        ui->pb_up->setEnabled(hasPrev);
        ui->pb_down->setEnabled(hasNext);
    } else {
        ui->pb_delete->setEnabled(false);
        ui->pb_properties->setEnabled(false);
        ui->pb_up->setEnabled(false);
        ui->pb_down->setEnabled(false);
    }
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false.
 */
bool UMLOperationDialog::apply()
{
    QString name = ui->tb_name->text();
    if(name.length() == 0) {
        KMessageBox::error(this, i18n("You have entered an invalid operation name."),
                           i18n("Operation Name Invalid"), 0);
        ui->tb_name->setText(m_operation->name());
        return false;
    }

    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(m_operation->parent());
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

    bool isAbstract = ui->ck_abstract->isChecked();
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
    m_operation->setStatic(ui->ck_classifierScope->isChecked());
    m_operation->setConst(ui->ck_queryConst->isChecked());
    ui->documentationWidget->apply();

    return true;
}


