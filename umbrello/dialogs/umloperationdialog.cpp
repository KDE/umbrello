/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umloperationdialog.h"

//app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "operation.h"
#include "classifier.h"
#include "template.h"
#include "dialogspopupmenu.h"
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

DEBUG_REGISTER(UMLOperationDialog)

/**
 * Constructor.
 */
UMLOperationDialog::UMLOperationDialog(QWidget * parent, UMLOperation * pOperation)
  : SinglePageDialogBase(parent),
    m_pOverrideCB(0)
{
    setCaption(i18n("Operation Properties"));
    m_operation = pOperation;
    m_doc = UMLApp::app()->document();
    m_menu = 0;
    for (int i = 0; i < N_STEREOATTRS; i++) {
        m_pTagL [i] = 0;
        m_pTagLE[i] = 0;
    }
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
    QFrame *frame = new QFrame(this);
    setMainWidget(frame);
    int margin = fontMetrics().height();
    QVBoxLayout * topLayout = new QVBoxLayout(frame);

    m_pGenGB = new QGroupBox(i18n("General Properties"), frame);
    m_pGenLayout = new QGridLayout(m_pGenGB);
    m_pGenLayout->setColumnStretch(1, 1);
    m_pGenLayout->setColumnStretch(3, 1);
    m_pGenLayout->addItem(new QSpacerItem(200, 0), 0, 1);
    m_pGenLayout->addItem(new QSpacerItem(200, 0), 0, 3);
    m_pGenLayout->setSpacing(10);

    Dialog_Utils::makeLabeledEditField(m_pGenLayout, 0,
                                    m_pNameL, i18nc("operation name", "&Name:"),
                                    m_pNameLE, m_operation->name());

    m_datatypeWidget = new UMLDatatypeWidget(m_operation);
    m_datatypeWidget->addToLayout(m_pGenLayout, 0, 2);

    m_stereotypeWidget = new UMLStereotypeWidget(m_operation);
    m_stereotypeWidget->addToLayout(m_pGenLayout, 1);
    connect(m_stereotypeWidget->editField(), SIGNAL(currentTextChanged(const QString&)),
                                       this, SLOT(slotStereoTextChanged(const QString&)));
    Dialog_Utils::makeTagEditFields(m_operation, m_pGenLayout, m_pTagL, m_pTagLE);
    bool isInterface = m_operation->umlPackage()->asUMLClassifier()->isInterface();
    m_pAbstractCB = new QCheckBox(i18n("&Abstract operation"), m_pGenGB);
    m_pAbstractCB->setChecked(m_operation->isAbstract());
    m_pAbstractCB->setEnabled(!isInterface);
    m_pGenLayout->addWidget(m_pAbstractCB, 2, 0);
    m_pStaticCB = new QCheckBox(i18n("Classifier &scope (\"static\")"), m_pGenGB);
    m_pStaticCB->setChecked(m_operation->isStatic());
    m_pGenLayout->addWidget(m_pStaticCB, 2, 1);
    m_pQueryCB = new QCheckBox(i18n("&Query (\"const\")"), m_pGenGB);
    m_pQueryCB->setChecked(m_operation->getConst());
    m_pGenLayout->addWidget(m_pQueryCB, 2, 2);
    m_virtualCB = new QCheckBox(i18n("&virtual"), m_pGenGB);
    m_virtualCB->setChecked(m_operation->isVirtual());
    m_virtualCB->setEnabled(!isInterface);
    m_pGenLayout->addWidget(m_virtualCB, 2, 3);
    m_inlineCB = new QCheckBox(i18n("&inline"), m_pGenGB);
    m_inlineCB->setChecked(m_operation->isInline());
    m_inlineCB->setEnabled(!isInterface);
    m_pGenLayout->addWidget(m_inlineCB, 2, 4);
    if (Settings::optionState().codeImportState.supportCPP11) {
        m_pOverrideCB = new QCheckBox(i18n("&Override"), m_pGenGB);
        m_pOverrideCB->setChecked(m_operation->getOverride());
        m_pOverrideCB->setEnabled(!isInterface);
        m_pGenLayout->addWidget(m_pOverrideCB, 2, 5);
    }

    m_visibilityEnumWidget = new VisibilityEnumWidget(m_operation, this);
    m_visibilityEnumWidget->setEnabled(!isInterface);

    m_docWidget = new DocumentationWidget(m_operation, this);

    m_pParmsGB = new QGroupBox(i18n("Parameters"), frame);
    QVBoxLayout* parmsLayout = new QVBoxLayout(m_pParmsGB);
    parmsLayout->setSpacing(10);

    // horizontal box contains the list box and the move up/down buttons
    QHBoxLayout* parmsHBoxLayout = new QHBoxLayout();
    m_pParmsLW = new QListWidget(m_pParmsGB);
    m_pParmsLW->setContextMenuPolicy(Qt::CustomContextMenu);

    // the move up/down buttons (another vertical box)
    QVBoxLayout* buttonLayout = new QVBoxLayout();

    m_pUpButton = new QToolButton(m_pParmsGB);
    m_pUpButton->setArrowType(Qt::UpArrow);
    m_pUpButton->setEnabled(false);
    buttonLayout->addWidget(m_pUpButton);

    m_pDownButton = new QToolButton(m_pParmsGB);
    m_pDownButton->setArrowType(Qt::DownArrow);
    m_pDownButton->setEnabled(false);
    buttonLayout->addWidget(m_pDownButton);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(m_pParmsGB);
    QPushButton* newParam = buttonBox->addButton(i18n("Ne&w Parameter..."), QDialogButtonBox::ActionRole);
    connect(newParam, SIGNAL(clicked()), this, SLOT(slotNewParameter()));
    m_pDeleteButton = buttonBox->addButton(i18n("&Delete"), QDialogButtonBox::ActionRole);
    connect(m_pDeleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteParameter()));
    m_pPropertiesButton = buttonBox->addButton(i18n("&Properties"), QDialogButtonBox::ActionRole);
    connect(m_pPropertiesButton, SIGNAL(clicked()), this, SLOT(slotParameterProperties()));

    parmsHBoxLayout->addWidget(m_pParmsLW);
    parmsHBoxLayout->addLayout(buttonLayout);

    parmsLayout->addLayout(parmsHBoxLayout);
    parmsLayout->addWidget(buttonBox);

    topLayout->addWidget(m_pGenGB);
    topLayout->addWidget(m_visibilityEnumWidget);
    topLayout->addWidget(m_docWidget);
    topLayout->addWidget(m_pParmsGB);

    m_pDeleteButton->setEnabled(false);
    m_pPropertiesButton->setEnabled(false);
    m_pUpButton->setEnabled(false);
    m_pDownButton->setEnabled(false);

    // fill in parm list box
    UMLAttributeList list = m_operation->getParmList();
    for(UMLAttribute* pAtt : list) {
        m_pParmsLW->addItem(pAtt->toString(Uml::SignatureType::SigNoVis));
    }

    // setup parm list box signals
    connect(m_pUpButton, SIGNAL(clicked()), this, SLOT(slotParameterUp()));
    connect(m_pDownButton, SIGNAL(clicked()), this, SLOT(slotParameterDown()));

    connect(m_pParmsLW, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(slotParamsBoxClicked(QListWidgetItem*)));
    connect(m_pParmsLW, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotParmRightButtonPressed(QPoint)));
    connect(m_pParmsLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(slotParmDoubleClick(QListWidgetItem*)));

    m_pNameLE->setFocus();
    connect(m_pNameLE, SIGNAL(textChanged(QString)), SLOT(slotNameChanged(QString)));
    slotNameChanged(m_pNameLE->text());
}

void UMLOperationDialog::slotNameChanged(const QString &_text)
{
    enableButtonOk(!_text.isEmpty());
}

void UMLOperationDialog::slotStereoTextChanged(const QString &stereoText)
{
    Dialog_Utils::remakeTagEditFields(stereoText, m_operation, m_pGenLayout, m_pTagL, m_pTagLE);
}

void UMLOperationDialog::slotParmRightButtonPressed(const QPoint &p)
{
    DialogsPopupMenu::TriggerType type = DialogsPopupMenu::tt_Undefined;
    QListWidgetItem* item = m_pParmsLW->itemAt(p);
    if (item) // pressed on an item
    {
        type = DialogsPopupMenu::tt_Parameter_Selected;
    } else // pressed into fresh air
    {
        type = DialogsPopupMenu::tt_New_Parameter;
    }
    if (m_menu) {
        m_menu->hide();
        disconnect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuSelection(QAction*)));
        delete m_menu;
        m_menu = 0;
    }
    DialogsPopupMenu popup(this, type);
    QAction *triggered = popup.exec(m_pParmsLW->mapToGlobal(p));
    slotMenuSelection(triggered);
}

void UMLOperationDialog::slotParmDoubleClick(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    // this happens, when there was no right click in the list widget
    DialogsPopupMenu popup(this, DialogsPopupMenu::tt_Parameter_Selected);
    QAction *triggered = popup.getAction(DialogsPopupMenu::mt_Properties);
    slotMenuSelection(triggered);
}

void UMLOperationDialog::slotMenuSelection(QAction* action)
{
    DialogsPopupMenu::MenuType id = DialogsPopupMenu::typeFromAction(action);
    if(id == DialogsPopupMenu::mt_Rename || id == DialogsPopupMenu::mt_Properties) {
        slotParameterProperties();
    } else if(id == DialogsPopupMenu::mt_New_Parameter) {
        slotNewParameter();
    }
    else if(id == DialogsPopupMenu::mt_Delete) {
        slotDeleteParameter();
    }
}

void UMLOperationDialog::slotNewParameter()
{
    UMLAttribute* pAtt = nullptr;

    QString currentName = m_operation->getUniqueParameterName();
    UMLAttribute* newAttribute = new UMLAttribute(m_operation, currentName, Uml::ID::Reserved);

    QPointer<ParameterPropertiesDialog> dlg = new ParameterPropertiesDialog(this, m_doc, newAttribute);
    if (dlg->exec()) {
        pAtt = m_operation->findParm(newAttribute->name());

        if (!pAtt) {
            newAttribute->setID(UniqueID::gen());
            m_operation->addParm(newAttribute);
            m_pParmsLW->addItem(newAttribute->toString(Uml::SignatureType::SigNoVis));
            m_doc->setModified(true);
        } else {
            KMessageBox::information(this, i18n("The parameter name you have chosen\nis already being used in this operation."),
                               i18n("Parameter Name Not Unique"));
            delete newAttribute;
        }
    } else {
        delete newAttribute;
    }
    delete dlg;
}

void UMLOperationDialog::slotDeleteParameter()
{
    UMLAttribute* pOldAtt = m_operation->getParmList().at(m_pParmsLW->row(m_pParmsLW->currentItem()));

    m_operation->removeParm(pOldAtt);
    m_pParmsLW->takeItem(m_pParmsLW->currentRow());
    m_doc->setModified(true);

    m_pDeleteButton->setEnabled(false);
    m_pPropertiesButton->setEnabled(false);
    m_pUpButton->setEnabled(false);
    m_pDownButton->setEnabled(false);
}

void UMLOperationDialog::slotParameterProperties()
{
    UMLAttribute* pAtt = nullptr, * pOldAtt = nullptr;

    int position = m_pParmsLW->row(m_pParmsLW->currentItem());
    pOldAtt = m_operation->getParmList().at(position);
    if (!pOldAtt) {
        logDebug1("UMLOperationDialog::slotParameterProperties: The impossible has occurred for: %1",
                  m_pParmsLW->currentItem()->text());
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
                               i18n("Parameter Name Not Unique"));
            namingConflict = true;
        }

        tempAttribute->copyInto(pOldAtt); // copy all attributes from the clone
        if (namingConflict) {
            pOldAtt->setName(oldAttName); // reset the name if there was a naming conflict
        }

        QListWidgetItem* item = m_pParmsLW->currentItem();
        item->setText(pOldAtt->toString(Uml::SignatureType::SigNoVis));
        m_doc->setModified(true);
    }
    delete tempAttribute;
    delete dlg;
}

void UMLOperationDialog::slotParameterUp()
{
    int row = m_pParmsLW->currentRow();
    QListWidgetItem* item = m_pParmsLW->currentItem();
    if (item) {
        UMLAttribute* pOldAtt = m_operation->getParmList().at(m_pParmsLW->row(item));

        m_operation->moveParmLeft(pOldAtt);
        m_pParmsLW->takeItem(row);
        m_pParmsLW->insertItem(row - 1, item);

        m_doc->setModified(true);
        slotParamsBoxClicked(item);
    }
    else {
        logDebug0("UMLOperationDialog::slotParameterUp: No current item in list widget!?");
    }
}

void UMLOperationDialog::slotParameterDown()
{
    int row = m_pParmsLW->currentRow();
    QListWidgetItem* item = m_pParmsLW->currentItem();
    if (item) {
        UMLAttribute* pOldAtt = m_operation->getParmList().at(m_pParmsLW->row(item));

        m_operation->moveParmRight(pOldAtt);
        m_pParmsLW->takeItem(row);
        m_pParmsLW->insertItem(row + 1, item);

        m_doc->setModified(true);
        slotParamsBoxClicked(item);
    }
    else {
        logDebug0("UMLOperationDialog::slotParameterDown: No current item in list widget!?");
    }
}

/**
 * Enables or disables buttons.
 */
void UMLOperationDialog::slotParamsBoxClicked(QListWidgetItem* parameterItem)
{
    if (parameterItem) {
        m_pDeleteButton->setEnabled(true);
        m_pPropertiesButton->setEnabled(true);
        int row = m_pParmsLW->row(parameterItem);
        bool hasNext = (row < m_pParmsLW->count() - 1);
        bool hasPrev = (row > 0);
        m_pUpButton->setEnabled(hasPrev);
        m_pDownButton->setEnabled(hasNext);
    } else {
        m_pDeleteButton->setEnabled(false);
        m_pPropertiesButton->setEnabled(false);
        m_pUpButton->setEnabled(false);
        m_pDownButton->setEnabled(false);
    }
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false.
 */
bool UMLOperationDialog::apply()
{
    QString name = m_pNameLE->text();
    if(name.length() == 0) {
        KMessageBox::error(this, i18n("You have entered an invalid operation name."),
                           i18n("Operation Name Invalid"));
        m_pNameLE->setText(m_operation->name());
        return false;
    }

    UMLClassifier *classifier = m_operation->umlParent()->asUMLClassifier();
    if(classifier != 0 &&
            classifier->checkOperationSignature(name, m_operation->getParmList(), m_operation))
    {
        QString msg = i18n("An operation with that signature already exists in %1.\n", classifier->name())
                      +
                      i18n("Choose a different name or parameter list.");
        KMessageBox::error(this, msg, i18n("Operation Name Invalid"));
        return false;
    }
    m_operation->setName(name);

    m_visibilityEnumWidget->apply();
    m_datatypeWidget->apply();
    m_stereotypeWidget->apply();
    Dialog_Utils::updateTagsFromEditFields(m_operation, m_pTagLE);

    bool isAbstract = m_pAbstractCB->isChecked();
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
    m_operation->setStatic(m_pStaticCB->isChecked());
    m_operation->setConst(m_pQueryCB->isChecked());
    m_operation->setVirtual(m_virtualCB->isChecked());
    m_operation->setInline(m_inlineCB->isChecked());
    if (m_pOverrideCB)
        m_operation->setOverride(m_pOverrideCB->isChecked());
    m_docWidget->apply();

    return true;
}


