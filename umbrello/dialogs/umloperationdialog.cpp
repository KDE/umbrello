/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umloperationdialog.h"

//qt includes
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>

//kde includes
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kbuttonbox.h>
#include <karrowbutton.h>

//app includes
#include "../uml.h"
#include "../umldoc.h"
#include "../operation.h"
#include "../classifier.h"
#include "../template.h"
#include "../listpopupmenu.h"
#include "../umlattributelist.h"
#include "../classifierlistitem.h"
#include "../umlclassifierlistitemlist.h"
#include "../dialog_utils.h"
#include "parmpropdlg.h"
#include "../stereotype.h"
#include "../uniqueid.h"

UMLOperationDialog::UMLOperationDialog( QWidget * parent, UMLOperation * pOperation )
        : KDialogBase( Plain, i18n("Operation Properties"), Help | Ok | Cancel , Ok, parent, "_UMLOPERATIONDLG_", true, true) {
    m_pOperation = pOperation;
    m_doc = UMLApp::app()->getDocument();
    m_pMenu = 0;
    setupDialog();
}

UMLOperationDialog::~UMLOperationDialog() {}

void UMLOperationDialog::setupDialog() {

    int margin = fontMetrics().height();
    QVBoxLayout * topLayout = new QVBoxLayout( plainPage() );

    m_pGenGB = new QGroupBox(i18n("General Properties"), plainPage() );
    QGridLayout * genLayout = new QGridLayout(m_pGenGB, 3, 4 );
    genLayout -> setColStretch(1, 1);
    genLayout -> setColStretch(3, 1);
    genLayout -> addColSpacing(1, 200);
    genLayout -> addColSpacing(3, 200);
    genLayout -> setMargin(margin);
    genLayout -> setSpacing(10);

    Dialog_Utils::makeLabeledEditField( m_pGenGB, genLayout, 0,
                                    m_pNameL, i18n("&Name:"),
                                    m_pNameLE, m_pOperation->getName() );

    m_pRtypeL = new QLabel(i18n("&Type:"), m_pGenGB );
    genLayout -> addWidget(m_pRtypeL, 0, 2);

    m_pRtypeCB = new KComboBox(true, m_pGenGB );
    genLayout -> addWidget(m_pRtypeCB, 0, 3);
    m_pRtypeL->setBuddy(m_pRtypeCB);

    m_pStereoTypeL = new QLabel( i18n("Stereotype name:"), m_pGenGB );
    genLayout -> addWidget(m_pStereoTypeL, 1, 0);
    m_pStereoTypeCB = new KComboBox(true, m_pGenGB );
    genLayout -> addWidget(m_pStereoTypeCB, 1, 1);

    m_pAbstractCB = new QCheckBox( i18n("&Abstract operation"), m_pGenGB );
    m_pAbstractCB -> setChecked( m_pOperation->getAbstract() );
    genLayout -> addWidget( m_pAbstractCB, 2, 0 );
    m_pStaticCB = new QCheckBox( i18n("Classifier &scope (\"static\")"), m_pGenGB );
    m_pStaticCB -> setChecked( m_pOperation->getStatic() );
    genLayout -> addWidget( m_pStaticCB, 2, 1 );
    m_pQueryCB = new QCheckBox( i18n("&Query (\"const\")"), m_pGenGB );
    m_pQueryCB -> setChecked( m_pOperation->getConst() );
    genLayout -> addWidget( m_pQueryCB, 2, 2 );

    topLayout -> addWidget( m_pGenGB );

    m_pScopeBG = new QButtonGroup(i18n("Visibility"), plainPage() );

    QHBoxLayout * scopeLayout = new QHBoxLayout(m_pScopeBG);
    scopeLayout -> setMargin(margin);

    m_pPublicRB = new QRadioButton(i18n("P&ublic"), m_pScopeBG);
    scopeLayout -> addWidget(m_pPublicRB);

    m_pPrivateRB = new QRadioButton(i18n("P&rivate"), m_pScopeBG);
    scopeLayout -> addWidget(m_pPrivateRB);

    m_pProtectedRB = new QRadioButton(i18n("Prot&ected"), m_pScopeBG);
    scopeLayout -> addWidget(m_pProtectedRB);

    m_pImplementationRB = new QRadioButton(i18n("I&mplementation"), m_pScopeBG);
    scopeLayout -> addWidget(m_pImplementationRB);

    topLayout -> addWidget(m_pScopeBG);

    m_pParmsGB = new QGroupBox(i18n("Parameters"), plainPage() );
    QVBoxLayout* parmsLayout = new QVBoxLayout(m_pParmsGB);
    parmsLayout->setMargin(margin);
    parmsLayout->setSpacing(10);

    //horizontal box contains the list box and the move up/down buttons
    QHBoxLayout* parmsHBoxLayout = new QHBoxLayout(parmsLayout);
    m_pParmsLB = new QListBox(m_pParmsGB);
    parmsHBoxLayout->addWidget(m_pParmsLB);

    //the move up/down buttons (another vertical box)
    QVBoxLayout* buttonLayout = new QVBoxLayout( parmsHBoxLayout );
    m_pUpButton = new KArrowButton( m_pParmsGB );
    m_pUpButton->setEnabled( false );
    buttonLayout->addWidget( m_pUpButton );

    m_pDownButton = new KArrowButton( m_pParmsGB, Qt::DownArrow );
    m_pDownButton->setEnabled( false );
    buttonLayout->addWidget( m_pDownButton );

    KButtonBox* buttonBox = new KButtonBox(m_pParmsGB);
    buttonBox->addButton( i18n("Ne&w Parameter..."), this, SLOT(slotNewParameter()) );
    m_pDeleteButton = buttonBox->addButton( i18n("&Delete"), this, SLOT(slotDeleteParameter()) );
    m_pPropertiesButton = buttonBox->addButton( i18n("&Properties"), this,
                          SLOT(slotParameterProperties()) );
    parmsLayout->addWidget(buttonBox);

    topLayout -> addWidget(m_pParmsGB);

    m_pDeleteButton->setEnabled(false);
    m_pPropertiesButton->setEnabled(false);
    m_pUpButton->setEnabled(false);
    m_pDownButton->setEnabled(false);

    // Add "void". We use this for denoting "no return type" independent
    // of the programming language.
    // For example, the Ada generator would interpret the return type
    // "void" as an instruction to generate a procedure instead of a
    // function.
    insertType( "void" );

    m_pRtypeCB->setDuplicatesEnabled(false);//only allow one of each type in box
    m_pRtypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );

    // add template parameters
    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(m_pOperation->parent());
    if (classifier) {
        UMLClassifierListItemList tmplParams( classifier->getFilteredList(Uml::ot_Template) );
        for (UMLClassifierListItem *li = tmplParams.first(); li; li = tmplParams.next())
            insertType( li->getName() );
    }
    //now add the Classes and Interfaces (both are Concepts)
    UMLClassifierList namesList( m_doc->getConcepts() );
    UMLClassifier* pConcept = 0;
    for(pConcept=namesList.first(); pConcept!=0 ;pConcept=namesList.next()) {
        insertType( pConcept->getFullyQualifiedName() );
    }

    //work out which one to select
    int returnBoxCount = 0;
    bool foundReturnType = false;
    while (returnBoxCount < m_pRtypeCB->count() && foundReturnType == false) {
        QString returnBoxString = m_pRtypeCB->text(returnBoxCount);
        if ( returnBoxString == m_pOperation->getTypeName() ) {
            foundReturnType = true;
            m_pRtypeCB->setCurrentItem(returnBoxCount);
            break;
        }
        returnBoxCount++;
    }

    if (!foundReturnType) {
        insertType( m_pOperation->getTypeName(), 0 );
        m_pRtypeCB->setCurrentItem(0);
    }

    //fill in parm list box
    UMLAttributeList list = m_pOperation->getParmList();
    UMLAttribute * pAtt = 0;
    for (pAtt = list.first(); pAtt; pAtt = list.next())
        m_pParmsLB->insertItem( pAtt->getName() );

    //set scope
    Uml::Visibility scope = m_pOperation -> getVisibility();
    if( scope == Uml::Visibility::Public )
      m_pPublicRB -> setChecked( true );
    else if( scope == Uml::Visibility::Private )
      m_pPrivateRB -> setChecked( true );
    else if( scope == Uml::Visibility::Protected )
      m_pProtectedRB -> setChecked( true );
    else if( scope == Uml::Visibility::Implementation )
      m_pImplementationRB -> setChecked( true );

    // manage stereotypes
    m_pStereoTypeCB -> setDuplicatesEnabled(false);//only allow one of each type in box
    m_pStereoTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    insertStereotype (QString("")); // an empty stereotype is the default
    int defaultStereotype=0;
    bool foundDefaultStereotype = false;
    for (UMLStereotypeListIt it(m_doc->getStereotypes()); it.current(); ++it) {
        if (!foundDefaultStereotype) {
            if ( m_pOperation->getStereotype() == it.current()->getName()) {
                foundDefaultStereotype = true;
            }
            defaultStereotype++;
        }
        insertStereotype (it.current()->getName());
    }
    // lookup for a default stereotype, if the operation doesn't have one
    if (foundDefaultStereotype)
        m_pStereoTypeCB->setCurrentItem(defaultStereotype);
    else
        m_pStereoTypeCB->setCurrentItem(-1);

    //setup parm list box signals
    connect( m_pUpButton, SIGNAL( clicked() ), this, SLOT( slotParameterUp() ) );
    connect( m_pDownButton, SIGNAL( clicked() ), this, SLOT( slotParameterDown() ) );

    connect(m_pParmsLB, SIGNAL(clicked(QListBoxItem*)),
            this, SLOT(slotParamsBoxClicked(QListBoxItem*)));

    connect(m_pParmsLB, SIGNAL(rightButtonPressed(QListBoxItem *, const QPoint &)),
            this, SLOT(slotParmRightButtonPressed(QListBoxItem *, const QPoint &)));

    connect(m_pParmsLB, SIGNAL(rightButtonClicked(QListBoxItem *, const QPoint &)),
            this, SLOT(slotParmRightButtonClicked(QListBoxItem *, const QPoint &)));


    connect(m_pParmsLB, SIGNAL(doubleClicked(QListBoxItem *)),
            this, SLOT(slotParmDoubleClick(QListBoxItem *)));

    m_pNameLE->setFocus();
    connect( m_pNameLE, SIGNAL( textChanged ( const QString & ) ), SLOT( slotNameChanged( const QString & ) ) );
    slotNameChanged(m_pNameLE->text() );

}

void UMLOperationDialog::slotNameChanged( const QString &_text )
{
    enableButtonOK( !_text.isEmpty() );
}


void UMLOperationDialog::slotParmRightButtonPressed(QListBoxItem *item, const QPoint &p) {
    ListPopupMenu::Menu_Type type = ListPopupMenu::mt_Undefined;
    if(item)//pressed on an item
    {
        type = ListPopupMenu::mt_Parameter_Selected;
    } else//pressed into fresh air
    {
        type = ListPopupMenu::mt_New_Parameter;
    }
    if(m_pMenu) {
        m_pMenu -> hide();
        disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotParmPopupMenuSel(int)));
        delete m_pMenu;
        m_pMenu = 0;
    }
    m_pMenu = new ListPopupMenu(this, type);
    m_pMenu->popup(p);
    connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotParmPopupMenuSel(int)));

}

void UMLOperationDialog::slotParmRightButtonClicked(QListBoxItem */*item*/, const QPoint &/*p*/) {
    if(m_pMenu) {
        m_pMenu -> hide();
        disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotParmPopupMenuSel(int)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

void UMLOperationDialog::slotParmDoubleClick(QListBoxItem *item) {
    if(!item)
        return;
    slotParmPopupMenuSel(ListPopupMenu::mt_Properties);
}

void UMLOperationDialog::slotParmPopupMenuSel(int id) {
    if( id == ListPopupMenu::mt_Rename || id == ListPopupMenu::mt_Properties ) {
        slotParameterProperties();
    } else if( id == ListPopupMenu::mt_New_Parameter ) {
        slotNewParameter();
    }
    else if( id == ListPopupMenu::mt_Delete ) {
        slotDeleteParameter();
    }
}

void UMLOperationDialog::slotNewParameter() {
    int result = 0;
    UMLAttribute* pAtt = 0;

    QString currentName = m_pOperation->getUniqueParameterName();
    UMLAttribute* newAttribute = new UMLAttribute(m_pOperation, currentName, Uml::id_Reserved);

    ParmPropDlg dlg(this, m_doc, newAttribute);
    result = dlg.exec();
    QString name = dlg.getName();
    pAtt = m_pOperation -> findParm( name );
    if( result ) {
        if( name.length() == 0 ) {
            KMessageBox::error(this, i18n("You have entered an invalid parameter name."),
                               i18n("Parameter Name Invalid"), false);
            delete newAttribute;
            return;
        }
        if( !pAtt ) {
            newAttribute->setID( UniqueID::gen() );
            newAttribute->setName( name );
            newAttribute->setTypeName( dlg.getTypeName() );
            newAttribute->setInitialValue( dlg.getInitialValue() );
            newAttribute->setDoc( dlg.getDoc() );
            newAttribute->setParmKind( dlg.getParmKind() );
            m_pOperation->addParm( newAttribute );
            m_pParmsLB -> insertItem( name );
            m_doc -> setModified( true );
        } else {
            KMessageBox::sorry(this, i18n("The parameter name you have chosen\nis already being used in this operation."),
                               i18n("Parameter Name Not Unique"), false);
            delete newAttribute;
        }
    } else {
        delete newAttribute;
    }
}

void UMLOperationDialog::slotDeleteParameter() {
    UMLAttribute* pOldAtt = m_pOperation->findParm( m_pParmsLB->currentText() );

    m_pOperation->removeParm( pOldAtt );
    m_pParmsLB->removeItem( m_pParmsLB->currentItem() );
    m_doc->setModified(true);

    m_pDeleteButton->setEnabled(false);
    m_pPropertiesButton->setEnabled(false);
    m_pUpButton->setEnabled(false);
    m_pDownButton->setEnabled(false);
}

void UMLOperationDialog::slotParameterProperties() {
    int result = 0;
    UMLAttribute* pAtt = 0, * pOldAtt = 0;
    pOldAtt = m_pOperation->findParm( m_pParmsLB->currentText() );

    if( !pOldAtt ) {
        kDebug() << "THE impossible has occurred for:" << m_pParmsLB->currentText() << endl;
        return;
    }//should never occur
    ParmPropDlg dlg(this, m_doc, pOldAtt);
    result = dlg.exec();
    QString name = dlg.getName();
    pAtt = m_pOperation->findParm( name );
    if( result ) {
        if( name.length() == 0 ) {
            KMessageBox::error(this, i18n("You have entered an invalid parameter name."),
                               i18n("Parameter Name Invalid"), false);
            return;
        }
        if ( !pAtt || pOldAtt->getTypeName() != dlg.getTypeName() ||
                pOldAtt->getDoc() != dlg.getDoc() ||
                pOldAtt->getInitialValue() != dlg.getInitialValue() ) {
            pOldAtt->setName( name );
            QString typeName = dlg.getTypeName();
            if (pOldAtt->getTypeName() != typeName) {
                UMLClassifierList namesList( m_doc->getConcepts() );
                UMLClassifier* obj = NULL;
                for (obj=namesList.first(); obj!=0; obj=namesList.next()) {
                    if (typeName == obj->getFullyQualifiedName()) {
                        pOldAtt->setType( obj );
                        break;
                    }
                }
                if (obj == NULL) {
                    // Nothing found: set type name directly. Bad.
                    kDebug() << "UMLOperationDialog::slotParameterProperties: "
                    << typeName << " not found." << endl;
                    pOldAtt->setTypeName( typeName );  // Bad.
                }
            }
            m_pParmsLB->changeItem( dlg.getName(), m_pParmsLB -> currentItem() );
            pOldAtt->setDoc( dlg.getDoc() );
            pOldAtt->setInitialValue( dlg.getInitialValue() );
            m_doc->setModified( true );
        } else if( pAtt != pOldAtt ) {
            KMessageBox::error(this, i18n("The parameter name you have chosen is already being used in this operation."),
                               i18n("Parameter Name Not Unique"), false);
        }
    }
}

void UMLOperationDialog::slotParameterUp()
{
    kDebug() << k_funcinfo << endl;
    UMLAttribute* pOldAtt = m_pOperation->findParm( m_pParmsLB->currentText() );

    m_pOperation->moveParmLeft( pOldAtt );
    QString tmp = m_pParmsLB->currentText();
    m_pParmsLB->changeItem( m_pParmsLB->item( m_pParmsLB->currentItem() - 1 )->text(), m_pParmsLB->currentItem() );
    m_pParmsLB->changeItem( tmp, m_pParmsLB->currentItem() - 1 );
    m_doc->setModified(true);
    slotParamsBoxClicked( m_pParmsLB->selectedItem() );
}

void UMLOperationDialog::slotParameterDown()
{
    UMLAttribute* pOldAtt = m_pOperation->findParm( m_pParmsLB->currentText() );

    m_pOperation->moveParmRight( pOldAtt );
    QString tmp = m_pParmsLB->currentText();
    m_pParmsLB->changeItem( m_pParmsLB->item( m_pParmsLB->currentItem() + 1 )->text(), m_pParmsLB->currentItem() );
    m_pParmsLB->changeItem( tmp, m_pParmsLB->currentItem() + 1 );

    m_doc->setModified(true);
    slotParamsBoxClicked( m_pParmsLB->selectedItem() );
}

void UMLOperationDialog::slotParamsBoxClicked(QListBoxItem* parameterItem) {
    if (parameterItem) {
        m_pDeleteButton->setEnabled(true);
        m_pPropertiesButton->setEnabled(true);
        m_pUpButton->setEnabled( parameterItem->prev() );
        m_pDownButton->setEnabled( parameterItem->next() );
    } else {
        m_pDeleteButton->setEnabled(false);
        m_pPropertiesButton->setEnabled(false);
        m_pUpButton->setEnabled(false);
        m_pDownButton->setEnabled(false);
    }
}

bool UMLOperationDialog::apply()
{
    QString name = m_pNameLE -> text();
    if( name.length() == 0 ) {
        KMessageBox::error(this, i18n("You have entered an invalid operation name."),
                           i18n("Operation Name Invalid"), false);
        m_pNameLE -> setText( m_pOperation -> getName() );
        return false;
    }

    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>( m_pOperation->parent() );
    if( classifier != 0L &&
            classifier->checkOperationSignature(name, m_pOperation->getParmList(), m_pOperation) )
    {
        QString msg = QString(i18n("An operation with that signature already exists in %1.\n")).arg(classifier->getName())
                      +
                      QString(i18n("Choose a different name or parameter list." ));
        KMessageBox::error(this, msg, i18n("Operation Name Invalid"), false);
        return false;
    }
    m_pOperation -> setName( name );

    if( m_pPublicRB -> isChecked() )
      m_pOperation -> setVisibility( Uml::Visibility::Public );
    else if( m_pPrivateRB -> isChecked() )
      m_pOperation -> setVisibility( Uml::Visibility::Private );
    else if (m_pProtectedRB -> isChecked() )
      m_pOperation -> setVisibility( Uml::Visibility::Protected );
    else if (m_pImplementationRB -> isChecked() )
      m_pOperation -> setVisibility( Uml::Visibility::Implementation );

    QString typeName = m_pRtypeCB->currentText();
    UMLTemplate *tmplParam = classifier->findTemplate(typeName);
    if (tmplParam)
        m_pOperation->setType(tmplParam);
    else
        m_pOperation->setTypeName(typeName);

    m_pOperation->setStereotype( m_pStereoTypeCB->currentText() );

    bool isAbstract = m_pAbstractCB->isChecked();
    m_pOperation -> setAbstract( isAbstract );
    if (isAbstract) {
        /* If any operation is abstract then the owning class needs
           to be made abstract.
           The inverse is not true: The fact that no operation is
           abstract does not mean that the class must be non-abstract.
         */
        classifier->setAbstract(true);
    }
    m_pOperation->setStatic( m_pStaticCB->isChecked() );
    m_pOperation->setConst( m_pQueryCB->isChecked() );

    return true;
}

void UMLOperationDialog::slotApply() {
    apply();
}

void UMLOperationDialog::slotOk() {
    if ( apply() ) {
        accept();
    }
}

void UMLOperationDialog::insertType( const QString& type, int index )
{
    m_pRtypeCB->insertItem( type, index );
    m_pRtypeCB->completionObject()->addItem( type );
}

void UMLOperationDialog::insertStereotype( const QString& type, int index )
{
    m_pStereoTypeCB->insertItem( type, index );
    m_pStereoTypeCB->completionObject()->addItem( type );
}

#include "umloperationdialog.moc"
