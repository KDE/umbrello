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
#include "parmpropdlg.h"

// local includes
#include "classifier.h"
#include "debug_utils.h"
#include "umltemplatelist.h"
#include "template.h"
#include "umldoc.h"
#include "dialog_utils.h"
#include "object_factory.h"
#include "stereotype.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>

// qt includes
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>
#include <QtGui/QRadioButton>

/**
 * Constructs a ParmPropDlg.
 * @param parent   the parent of the dialog
 * @param doc      
 * @param attr     the parameter to represent
 */
ParmPropDlg::ParmPropDlg(QWidget * parent, UMLDoc * doc, UMLAttribute * attr)
        : KDialog(parent)
{
    setCaption( i18n("Parameter Properties") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );

    m_pUmldoc = doc;
    m_pAtt = attr;
    QString type, text, name, initialValue;
    if (attr) {
        type = attr->getTypeName();
        name = attr->name();
        initialValue = attr->getInitialValue();
        text = attr->doc();
    }
    int margin = fontMetrics().height();
    setMinimumSize(300, 400);
    //disableResize();
    QFrame *frame = new QFrame( this );
    setMainWidget( frame );
    QVBoxLayout * topLayout = new QVBoxLayout(frame);
    topLayout->setSpacing(10);
    topLayout->setMargin(margin);

    m_pParmGB = new QGroupBox(i18n("Properties"), frame);
    topLayout->addWidget(m_pParmGB);

    QGridLayout * propLayout = new QGridLayout(m_pParmGB);
    propLayout->setSpacing(10);
    propLayout->setMargin(margin);

    m_pTypeL = new QLabel(i18n("&Type:"), m_pParmGB);
    propLayout->addWidget(m_pTypeL, 0, 0);

    m_pTypeCB = new KComboBox(m_pParmGB);
    propLayout->addWidget(m_pTypeCB, 0, 1);
    m_pTypeL->setBuddy(m_pTypeCB);

    Dialog_Utils::makeLabeledEditField( m_pParmGB, propLayout, 1,
                                    m_pNameL, i18nc("property name", "&Name:"),
                                    m_pNameLE, name );

    Dialog_Utils::makeLabeledEditField( m_pParmGB, propLayout, 2,
                                    m_pInitialL, i18n("&Initial value:"),
                                    m_pInitialLE, initialValue );

    m_pStereoTypeL = new QLabel( i18n("Stereotype name:"), m_pParmGB );
    propLayout->addWidget(m_pStereoTypeL, 3, 0);
    m_pStereoTypeCB = new KComboBox(true, m_pParmGB );
    propLayout->addWidget(m_pStereoTypeCB, 3, 1);

    m_pKind =  new QGroupBox(i18n("Passing Direction"), frame);
    m_pKind->setToolTip( i18n("\"in\" is a readonly parameter, \"out\" is a writeonly parameter and \"inout\" is a parameter for reading and writing."));

    QHBoxLayout * kindLayout = new QHBoxLayout( m_pKind );
    kindLayout->setMargin(margin);

    m_pIn =  new QRadioButton( "in", m_pKind );
    kindLayout->addWidget( m_pIn );

    m_pInOut =  new QRadioButton( "inout", m_pKind );
    kindLayout->addWidget( m_pInOut );

    m_pOut =  new QRadioButton( "out", m_pKind );
    kindLayout->addWidget( m_pOut );

    topLayout->addWidget(m_pKind);

    m_docGB = new QGroupBox(i18n("Documentation"), frame);
    QHBoxLayout * docLayout = new QHBoxLayout(m_docGB);
    docLayout->setMargin(margin);

    m_doc = new KTextEdit(m_docGB);
    m_doc->setWordWrapMode(QTextOption::WordWrap);
    m_doc->setText(text);
    docLayout->addWidget(m_doc);
    topLayout->addWidget(m_docGB);

    // Check the proper Kind radiobutton.
    if (attr) {
        Uml::Parameter_Direction kind = attr->getParmKind();
        if (kind == Uml::pd_Out)
            m_pOut->setChecked(true);
        else if (kind == Uml::pd_InOut)
            m_pInOut->setChecked(true);
        else
            m_pIn->setChecked(true);
    } else
        m_pIn->setChecked(true);

    // manage types
    m_pTypeCB->setDuplicatesEnabled(false); // only allow one of each type in box
    m_pTypeCB->setEditable(true);
    m_pTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
//    m_pTypeCB->setCompleter(...);
    if (attr) {
        insertTypesSorted(attr->getTypeName());
    }

    // manage stereotypes
    m_pStereoTypeCB->setDuplicatesEnabled(false); //only allow one of each type in box
    m_pStereoTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    if (m_pAtt) {
        insertStereotypesSorted(m_pAtt->stereotype());
    }

    // set tab order
    setTabOrder(m_pKind, m_pTypeCB);
    setTabOrder(m_pTypeCB, m_pNameLE);
    setTabOrder(m_pNameLE, m_pInitialLE);
    setTabOrder(m_pInitialLE, m_pStereoTypeCB);
    setTabOrder(m_pStereoTypeCB, m_pIn);
    setTabOrder(m_pIn, m_doc);
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    m_pNameLE->setFocus();
}

/**
 * Standard destructor.
 */
ParmPropDlg::~ParmPropDlg()
{
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add and selected
 */
void ParmPropDlg::insertTypesSorted(const QString& type)
{
    QStringList types;
    // add template parameters
    UMLClassifier *pConcept = dynamic_cast<UMLClassifier*>( m_pAtt->parent()->parent() );
    if (pConcept == NULL) {
        uError() << "ParmPropDlg: grandparent of " << m_pAtt->name()
                 << " is not a UMLClassifier";
    } else {
        UMLTemplateList tmplParams( pConcept->getTemplateList() );
        foreach( UMLTemplate* t, tmplParams ) {
            types << t->name();
        }
    }
    // now add the Concepts
    UMLClassifierList namesList( m_pUmldoc->concepts() );
    foreach(UMLClassifier* obj, namesList ) {
        types << obj->fullyQualifiedName();
    }
    // add the given parameter
    if ( !types.contains(type) ) {
        types << type;
    }
    types.sort();

    m_pTypeCB->clear();
    m_pTypeCB->insertItems(-1, types);

    // select the given parameter
    int currentIndex = m_pTypeCB->findText(type);
    if (currentIndex > -1) {
        m_pTypeCB->setCurrentIndex(currentIndex);
    }
}

/**
 * Inserts @p type into the stereotype-combobox as well as its completion object.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add and selected
 */
void ParmPropDlg::insertStereotypesSorted(const QString& type)
{
    QStringList types;
    types << ""; // an empty stereotype is the default
    foreach (UMLStereotype* currentSt, m_pUmldoc->stereotypes() ) {
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
}

/**
 * Return the kind of the parameter (in, out, or inout).
 * @return  The Uml::Parameter_Direction corresponding to
 *          the selected "Kind" radiobutton.
 */
Uml::Parameter_Direction ParmPropDlg::getParmKind()
{
    Uml::Parameter_Direction pk = Uml::pd_In;
    if (m_pOut->isChecked())
        pk = Uml::pd_Out;
    else if (m_pInOut->isChecked())
        pk = Uml::pd_InOut;
    return pk;
}

/**
 * Validates the fields in the dialog box.
 * @return success state
 */
bool ParmPropDlg::validate()
{
    // currently only validates whether the name is not null.
    if ( getName().trimmed().length() == 0 ) {
            KMessageBox::error(this, i18n("You have entered an invalid parameter name."),
                               i18n("Parameter Name Invalid"), 0);
            return false;
    }
    return true;
}

/**
 * Activated when a button is clicked
 * @param button The button that was clicked
 */
void ParmPropDlg::slotButtonClicked(int button)
{
    if ( button == KDialog::Ok ) {
        if ( !validate() ) {
            return;
        }
    }
    KDialog::slotButtonClicked( button );
}

/**
 * Ok clicked slot.
 */
void ParmPropDlg::slotOk()
{
    if (m_pAtt != NULL) {

        m_pAtt->setName( getName() );         // set the name
        m_pAtt->setParmKind( getParmKind() );  // set the direction
        m_pAtt->setStereotype( m_pStereoTypeCB->currentText() ); // set the stereotype

        // set the type name
        QString typeName = m_pTypeCB->currentText();
        UMLClassifier * pConcept = dynamic_cast<UMLClassifier*>( m_pAtt->parent()->parent() );
        if (pConcept == NULL) {
            uError() << "grandparent of " << m_pAtt->name() << " is not a UMLClassifier";
        } else {
            UMLTemplate *tmplParam = pConcept->findTemplate(typeName);
            if (tmplParam) {
                m_pAtt->setType(tmplParam);
                return;
            }
        }
        UMLClassifierList namesList( m_pUmldoc->concepts() );
        bool matchFound = false;

        foreach (UMLClassifier* obj, namesList) {
            if (obj->fullyQualifiedName() == typeName) {
                m_pAtt->setType(obj);
                matchFound = true;
                break;
            }
        }
        if (!matchFound) {
            // Nothing found: Create a new type on the fly.
            // @todo There should be an extra dialog to decide whether to
            // create a datatype or a class. For now, we create a class.
            uDebug() << "" << typeName << " not found."
                << " Creating a new class for the type.";
            UMLObject *newObj = Object_Factory::createUMLObject(UMLObject::ot_Class, typeName);
            m_pAtt->setType(newObj);
        }

        m_pAtt->setDoc( getDoc() ); // set the documentation
        m_pAtt->setInitialValue( getInitialValue() ); // set the initial value
    }
}

#include "parmpropdlg.moc"
