/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
#include "classifierlistitem.h"
#include "umlclassifierlistitemlist.h"
#include "dialog_utils.h"
#include "parmpropdlg.h"
#include "stereotype.h"
#include "uniqueid.h"

//kde includes
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialogbuttonbox.h>

//qt includes
#include <QtCore/QPointer>
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QToolButton>

/**
 * Constructor.
 */
UMLOperationDialog::UMLOperationDialog( QWidget * parent, UMLOperation * pOperation )
  : KDialog( parent)
{
    setCaption( i18n("Operation Properties") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );

    m_operation = pOperation;
    m_doc = UMLApp::app()->document();
    m_menu = 0;
    setupDialog();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
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
    QFrame *frame = new QFrame( this );
    setMainWidget( frame );
    int margin = fontMetrics().height();
    QVBoxLayout * topLayout = new QVBoxLayout( frame );

    m_pGenGB = new QGroupBox(i18n("General Properties"), frame );
    QGridLayout * genLayout = new QGridLayout(m_pGenGB);
    genLayout->setColumnStretch(1, 1);
    genLayout->setColumnStretch(3, 1);
    genLayout->addItem(new QSpacerItem(200, 0), 0, 1);
    genLayout->addItem(new QSpacerItem(200, 0), 0, 3);
    genLayout->setMargin(margin);
    genLayout->setSpacing(10);

    Dialog_Utils::makeLabeledEditField( m_pGenGB, genLayout, 0,
                                    m_pNameL, i18nc("operation name", "&Name:"),
                                    m_pNameLE, m_operation->name() );

    m_pRtypeL = new QLabel(i18n("&Type:"), m_pGenGB );
    genLayout->addWidget(m_pRtypeL, 0, 2);

    m_pRtypeCB = new KComboBox(true, m_pGenGB );
    genLayout->addWidget(m_pRtypeCB, 0, 3);
    m_pRtypeL->setBuddy(m_pRtypeCB);

    m_pStereoTypeL = new QLabel( i18n("Stereotype name:"), m_pGenGB );
    genLayout->addWidget(m_pStereoTypeL, 1, 0);
    m_pStereoTypeCB = new KComboBox(true, m_pGenGB );
    genLayout->addWidget(m_pStereoTypeCB, 1, 1);

    m_pAbstractCB = new QCheckBox( i18n("&Abstract operation"), m_pGenGB );
    m_pAbstractCB->setChecked( m_operation->isAbstract() );
    genLayout->addWidget( m_pAbstractCB, 2, 0 );
    m_pStaticCB = new QCheckBox( i18n("Classifier &scope (\"static\")"), m_pGenGB );
    m_pStaticCB->setChecked( m_operation->isStatic() );
    genLayout->addWidget( m_pStaticCB, 2, 1 );
    m_pQueryCB = new QCheckBox( i18n("&Query (\"const\")"), m_pGenGB );
    m_pQueryCB->setChecked( m_operation->getConst() );
    genLayout->addWidget( m_pQueryCB, 2, 2 );

    m_pScopeGB = new QGroupBox(i18n("Visibility"), frame );

    QHBoxLayout * scopeLayout = new QHBoxLayout(m_pScopeGB);
    scopeLayout->setMargin(margin);

    m_pPublicRB = new QRadioButton(i18nc("public visibility", "P&ublic"), m_pScopeGB);
    scopeLayout->addWidget(m_pPublicRB);

    m_pPrivateRB = new QRadioButton(i18nc("private visibility", "P&rivate"), m_pScopeGB);
    scopeLayout->addWidget(m_pPrivateRB);

    m_pProtectedRB = new QRadioButton(i18nc("protected visibility", "Prot&ected"), m_pScopeGB);
    scopeLayout->addWidget(m_pProtectedRB);

    m_pImplementationRB = new QRadioButton(i18n("I&mplementation"), m_pScopeGB);
    scopeLayout->addWidget(m_pImplementationRB);

    m_pParmsGB = new QGroupBox(i18n("Parameters"), frame );
    QVBoxLayout* parmsLayout = new QVBoxLayout(m_pParmsGB);
    parmsLayout->setMargin(margin);
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

    KDialogButtonBox* buttonBox = new KDialogButtonBox(m_pParmsGB);
    buttonBox->addButton( i18n("Ne&w Parameter..."), KDialogButtonBox::ActionRole,
                          this, SLOT(slotNewParameter()) );
    m_pDeleteButton = buttonBox->addButton( i18n("&Delete"), KDialogButtonBox::ActionRole,
                                            this, SLOT(slotDeleteParameter()) );
    m_pPropertiesButton = buttonBox->addButton( i18n("&Properties"), KDialogButtonBox::ActionRole,
                          this, SLOT(slotParameterProperties()) );

    parmsHBoxLayout->addWidget(m_pParmsLW);
    parmsHBoxLayout->addLayout(buttonLayout);

    parmsLayout->addLayout(parmsHBoxLayout);
    parmsLayout->addWidget(buttonBox);

    topLayout->addWidget(m_pGenGB);
    topLayout->addWidget(m_pScopeGB);
    topLayout->addWidget(m_pParmsGB);

    m_pDeleteButton->setEnabled(false);
    m_pPropertiesButton->setEnabled(false);
    m_pUpButton->setEnabled(false);
    m_pDownButton->setEnabled(false);

    m_pRtypeCB->setDuplicatesEnabled(false); // only allow one of each type in box
    m_pRtypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    // add the return types
    insertTypesSorted(m_operation->getTypeName());

    // fill in parm list box
    UMLAttributeList list = m_operation->getParmList();
    foreach (UMLAttribute* pAtt, list ) {
        m_pParmsLW->addItem( pAtt->toString( Uml::SignatureType::SigNoVis ) );
    }

    // set scope
    switch (m_operation->visibility()) {
    case Uml::Visibility::Public:
        m_pPublicRB->setChecked( true );
        break;
    case Uml::Visibility::Private:
        m_pPrivateRB->setChecked( true );
        break;
    case Uml::Visibility::Protected:
        m_pProtectedRB->setChecked( true );
        break;
    case Uml::Visibility::Implementation:
        m_pImplementationRB->setChecked( true );
        break;
    default:
        break;
    }

    // manage stereotypes
    m_pStereoTypeCB->setDuplicatesEnabled(false); // only allow one of each type in box
    m_pStereoTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    insertStereotypesSorted(m_operation->stereotype());

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
    connect( m_pNameLE, SIGNAL(textChanged(QString)), SLOT(slotNameChanged(QString)));
    slotNameChanged( m_pNameLE->text() );
}

void UMLOperationDialog::slotNameChanged( const QString &_text )
{
    enableButtonOk( !_text.isEmpty() );
}

void UMLOperationDialog::slotParmRightButtonPressed(const QPoint &p)
{
    ListPopupMenu::MenuType type = ListPopupMenu::mt_Undefined;
    QListWidgetItem* item = m_pParmsLW->itemAt(p);
    if (item) // pressed on an item
    {
        type = ListPopupMenu::mt_Parameter_Selected;
    } else // pressed into fresh air
    {
        type = ListPopupMenu::mt_New_Parameter;
    }
    if (m_menu) {
        m_menu->hide();
        disconnect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(slotParmPopupMenuSel(QAction*)));
        delete m_menu;
        m_menu = 0;
    }
    m_menu = new ListPopupMenu(this, type);
    connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(slotParmPopupMenuSel(QAction*)));
    m_menu->exec(m_pParmsLW->mapToGlobal(p));
}

void UMLOperationDialog::slotParmDoubleClick(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    if (!m_menu) {  // this happens, when there was no right click in the list widget
        m_menu = new ListPopupMenu(this, ListPopupMenu::mt_Parameter_Selected);
    }
    QAction* action = m_menu->getAction(ListPopupMenu::mt_Properties);
    slotParmPopupMenuSel(action);
}

void UMLOperationDialog::slotParmPopupMenuSel(QAction* action)
{
    ListPopupMenu::MenuType id = m_menu->getMenuType(action);
    if( id == ListPopupMenu::mt_Rename || id == ListPopupMenu::mt_Properties ) {
        slotParameterProperties();
    } else if( id == ListPopupMenu::mt_New_Parameter ) {
        slotNewParameter();
    }
    else if( id == ListPopupMenu::mt_Delete ) {
        slotDeleteParameter();
    }
}

void UMLOperationDialog::slotNewParameter()
{
    UMLAttribute* pAtt = 0;

    QString currentName = m_operation->getUniqueParameterName();
    UMLAttribute* newAttribute = new UMLAttribute(m_operation, currentName, Uml::id_Reserved);

    QPointer<ParmPropDlg> dlg = new ParmPropDlg(this, m_doc, newAttribute);
    if ( dlg->exec() ) {
        pAtt = m_operation->findParm( newAttribute->name() );

        if ( !pAtt ) {
            newAttribute->setID( UniqueID::gen() );
            m_operation->addParm( newAttribute );
            m_pParmsLW->addItem( newAttribute->toString( Uml::SignatureType::SigNoVis ) );
            m_doc->setModified( true );
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
    UMLAttribute* pOldAtt = m_operation->getParmList().at( m_pParmsLW->row( m_pParmsLW->currentItem() ) );

    m_operation->removeParm( pOldAtt );
    m_pParmsLW->takeItem( m_pParmsLW->currentRow() );
    m_doc->setModified(true);

    m_pDeleteButton->setEnabled(false);
    m_pPropertiesButton->setEnabled(false);
    m_pUpButton->setEnabled(false);
    m_pDownButton->setEnabled(false);
}

void UMLOperationDialog::slotParameterProperties()
{
    UMLAttribute* pAtt = 0, * pOldAtt = 0;

    int position = m_pParmsLW->row( m_pParmsLW->currentItem() );
    pOldAtt = m_operation->getParmList().at( position );
    if ( !pOldAtt ) {
        uDebug() << "THE impossible has occurred for:" << m_pParmsLW->currentItem()->text();
        return;
    } // should never occur

    QString oldAttName = pOldAtt->name();
    UMLAttribute* tempAttribute = static_cast<UMLAttribute*>( pOldAtt->clone() ); // create a clone of the parameter

    // send the clone to the properties dialog box. it will fill in the new parameters.
    QPointer<ParmPropDlg> dlg = new ParmPropDlg(this, m_doc, tempAttribute); 
    if ( dlg->exec() ) {
        bool namingConflict = false;
        QString newName = tempAttribute->name();

        pAtt = m_operation->findParm( newName ); // search whether a parameter with this name already exists
        if( pAtt && pAtt != pOldAtt  ) {
            KMessageBox::error(this, i18n("The parameter name you have chosen is already being used in this operation."),
                               i18n("Parameter Name Not Unique"), 0);
            namingConflict = true;
        }

        tempAttribute->copyInto( pOldAtt ); // copy all attributes from the clone
        if ( namingConflict ) {
            pOldAtt->setName( oldAttName ); // reset the name if there was a naming conflict
        }

        QListWidgetItem* item = m_pParmsLW->currentItem();
        item->setText( pOldAtt->toString(Uml::SignatureType::SigNoVis) );
        m_doc->setModified( true );
    }
    delete tempAttribute;
    delete dlg;
}

void UMLOperationDialog::slotParameterUp()
{
    int row = m_pParmsLW->currentRow();
    QListWidgetItem* item = m_pParmsLW->currentItem();
    if (item) {
        UMLAttribute* pOldAtt = m_operation->getParmList().at( m_pParmsLW->row( item ) );

        m_operation->moveParmLeft( pOldAtt );
        m_pParmsLW->takeItem(row);
        m_pParmsLW->insertItem(row - 1, item);

        m_doc->setModified(true);
        slotParamsBoxClicked(item);
    }
    else {
        uDebug() << "No current item in list widget!?";
    }
}

void UMLOperationDialog::slotParameterDown()
{
    int row = m_pParmsLW->currentRow();
    QListWidgetItem* item = m_pParmsLW->currentItem();
    if (item) {
        UMLAttribute* pOldAtt = m_operation->getParmList().at( m_pParmsLW->row( item ) );

        m_operation->moveParmRight( pOldAtt );
        m_pParmsLW->takeItem(row);
        m_pParmsLW->insertItem(row + 1, item);

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
    if( name.length() == 0 ) {
        KMessageBox::error(this, i18n("You have entered an invalid operation name."),
                           i18n("Operation Name Invalid"), 0);
        m_pNameLE->setText( m_operation->name() );
        return false;
    }

    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>( m_operation->parent() );
    if( classifier != 0 &&
            classifier->checkOperationSignature(name, m_operation->getParmList(), m_operation) )
    {
        QString msg = i18n("An operation with that signature already exists in %1.\n", classifier->name())
                      +
                      i18n("Choose a different name or parameter list.");
        KMessageBox::error(this, msg, i18n("Operation Name Invalid"), 0);
        return false;
    }
    m_operation->setName( name );

    if( m_pPublicRB->isChecked() )
      m_operation->setVisibility( Uml::Visibility::Public );
    else if( m_pPrivateRB->isChecked() )
      m_operation->setVisibility( Uml::Visibility::Private );
    else if (m_pProtectedRB->isChecked() )
      m_operation->setVisibility( Uml::Visibility::Protected );
    else if (m_pImplementationRB->isChecked() )
      m_operation->setVisibility( Uml::Visibility::Implementation );

    QString typeName = m_pRtypeCB->currentText();
    UMLTemplate *tmplParam = 0;
    if (classifier) {
        tmplParam = classifier->findTemplate(typeName);
    }
    if (tmplParam)
        m_operation->setType(tmplParam);
    else
        m_operation->setTypeName(typeName);

    m_operation->setStereotype( m_pStereoTypeCB->currentText() );

    bool isAbstract = m_pAbstractCB->isChecked();
    m_operation->setAbstract( isAbstract );
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
    m_operation->setStatic( m_pStaticCB->isChecked() );
    m_operation->setConst( m_pQueryCB->isChecked() );

    return true;
}

/**
 * I don't think this is used, but if we had an apply button
 * it would slot into here.
 */
void UMLOperationDialog::slotApply()
{
    apply();
}

void UMLOperationDialog::slotOk()
{
    if ( apply() ) {
        accept();
    }
}

/**
 * Inserts @p type into the type-combobox.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add and selected
 */
void UMLOperationDialog::insertTypesSorted( const QString& type )
{
    QStringList types;
    // Add "void". We use this for denoting "no return type" independent
    // of the programming language.
    // For example, the Ada generator would interpret the return type
    // "void" as an instruction to generate a procedure instead of a
    // function.
    types << "void";
    // add template parameters
    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(m_operation->parent());
    if (classifier) {
        UMLClassifierListItemList tmplParams( classifier->getFilteredList(UMLOperation::ot_Template) );
        foreach (UMLClassifierListItem* li, tmplParams ) {
            types << li->name();
        }
    }
    // add the Classes and Interfaces (both are Concepts)
    UMLClassifierList namesList( m_doc->concepts() );
    foreach (UMLClassifier* obj, namesList) {
         types << obj->fullyQualifiedName();
    }
    // add the given parameter
    if ( !types.contains(type) ) {
        types << type;
    }
    types.sort();

    m_pRtypeCB->clear();
    m_pRtypeCB->insertItems(-1, types);

    // select the given parameter
    int currentIndex = m_pRtypeCB->findText(type);
    if (currentIndex > -1) {
        m_pRtypeCB->setCurrentIndex(currentIndex);
    }
}

/**
 * Inserts @p stereotype into the stereotype-combobox as well as its completion object.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add and selected
 */
void UMLOperationDialog::insertStereotypesSorted( const QString& type )
{
    QStringList types;
    foreach (UMLStereotype* currentSt, m_doc->stereotypes() ) {
        types << currentSt->name();
    }
    // add the given parameter
    if ( !types.contains(type) ) {
        types << type;
    }
    types.sort();

    m_pStereoTypeCB->clear();
    m_pStereoTypeCB->insertItems(-1, types);

    // select the given parameter
    int currentIndex = m_pStereoTypeCB->findText(type);
    if (currentIndex > -1) {
        m_pStereoTypeCB->setCurrentIndex(currentIndex);
    }
    m_pStereoTypeCB->completionObject()->addItem( type );
}

#include "umloperationdialog.moc"
