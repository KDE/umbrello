/*
 *  copyright (C) 2002-2005
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "parmpropdlg.h"

// qt includes
#include <qlayout.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

// kde includes
#include <klocale.h>
#include <kdebug.h>

// local includes
#include "../classifier.h"
#include "../umltemplatelist.h"
#include "../template.h"
#include "../umldoc.h"
#include "../dialog_utils.h"
#include "../object_factory.h"

ParmPropDlg::ParmPropDlg(QWidget * parent, UMLDoc * doc, UMLAttribute * a)
        : KDialogBase(Plain, i18n("Parameter Properties"), Help | Ok | Cancel , Ok, parent, "_PARMPROPDLG_", true, true)
{
    m_pUmldoc = doc;
    m_pAtt = a;
    QString type, text, name, initialValue;
    if(!a) {
        type = text = name = initialValue = "";
    } else {
        type = a -> getTypeName();
        name = a -> getName();
        initialValue = a -> getInitialValue();
        text = a -> getDoc();
    }
    int margin = fontMetrics().height();
    setMinimumSize(300, 400);
    //disableResize();
    QVBoxLayout * topLayout = new QVBoxLayout(plainPage());
    topLayout -> setSpacing(10);
    topLayout -> setMargin(margin);

    m_pParmGB = new Q3GroupBox(i18n("Properties"), plainPage());
    topLayout -> addWidget(m_pParmGB);

    QGridLayout * propLayout = new QGridLayout(m_pParmGB, 4, 2);
    propLayout -> setSpacing(10);
    propLayout -> setMargin(margin);

    m_pTypeL = new QLabel(i18n("&Type:"), m_pParmGB);
    propLayout -> addWidget(m_pTypeL, 0, 0);

    m_pTypeCB = new QComboBox(m_pParmGB);
    propLayout -> addWidget(m_pTypeCB, 0, 1);
    m_pTypeL->setBuddy(m_pTypeCB);

    Dialog_Utils::makeLabeledEditField( m_pParmGB, propLayout, 1,
                                    m_pNameL, i18n("&Name:"),
                                    m_pNameLE, name );

    Dialog_Utils::makeLabeledEditField( m_pParmGB, propLayout, 2,
                                    m_pInitialL, i18n("&Initial value:"),
                                    m_pInitialLE, initialValue );

    Dialog_Utils::makeLabeledEditField( m_pParmGB, propLayout, 3,
                                    m_pStereoTypeL, i18n("&Stereotype name:"),
                                    m_pStereoTypeLE, m_pAtt->getStereotype() );

    m_pKind =  new Q3ButtonGroup(i18n("Passing Direction"), plainPage());
    m_pKind->setExclusive(true);
    QToolTip::add(m_pKind, i18n("\"in\" is a readonly parameter, \"out\" is a writeonly parameter and \"inout\" is a parameter for reading and writing."));

    QHBoxLayout * kindLayout = new QHBoxLayout( m_pKind );
    kindLayout->setMargin(margin);

    m_pIn =  new QRadioButton( "in", m_pKind );
    kindLayout->addWidget( m_pIn );

    m_pInOut =  new QRadioButton( "inout", m_pKind );
    kindLayout->addWidget( m_pInOut );

    m_pOut =  new QRadioButton( "out", m_pKind );
    kindLayout->addWidget( m_pOut );

    topLayout -> addWidget(m_pKind);

    m_pDocGB = new Q3GroupBox(i18n("Documentation"), plainPage());
    QHBoxLayout * docLayout = new QHBoxLayout(m_pDocGB);
    docLayout -> setMargin(margin);

    m_pDoc = new Q3MultiLineEdit(m_pDocGB);
    ///////////
    m_pDoc->setWordWrap(Q3MultiLineEdit::WidgetWidth);
    //////////
    m_pDoc -> setText(text);
    docLayout -> addWidget(m_pDoc);
    topLayout -> addWidget(m_pDocGB);

    // Check the proper Kind radiobutton.
    if (a) {
        Uml::Parameter_Direction kind = a->getParmKind();
        if (kind == Uml::pd_Out)
            m_pOut->setChecked(true);
        else if (kind == Uml::pd_InOut)
            m_pInOut->setChecked(true);
        else
            m_pIn->setChecked(true);
    } else
        m_pIn->setChecked(true);

    m_pTypeCB->setDuplicatesEnabled(false);//only allow one of each type in box
    m_pTypeCB->setEditable(true);
    m_pTypeCB->setAutoCompletion(false);

    //add template parameters
    UMLClassifier *pConcept = dynamic_cast<UMLClassifier*>( m_pAtt->parent()->parent() );
    if (pConcept == NULL) {
        kdError() << "ParmPropDlg: grandparent of " << m_pAtt->getName()
        << " is not a UMLClassifier" << endl;
    } else {
        UMLTemplateList tmplParams( pConcept->getTemplateList() );
        for (UMLTemplate *t = tmplParams.first(); t; t = tmplParams.next())
            m_pTypeCB->insertItem( t->getName() );
    }
    //now add the Concepts
    UMLClassifierList namesList( m_pUmldoc->getConcepts() );
    UMLClassifier * obj;
    for(obj=namesList.first(); obj!=0 ;obj=namesList.next()) {
        m_pTypeCB->insertItem( obj->getFullyQualifiedName() );
    }

    //work out which one to select
    int typeBoxCount = 0;
    bool foundType = false;
    while (typeBoxCount < m_pTypeCB->count() && foundType == false) {
        QString typeBoxString = m_pTypeCB->text(typeBoxCount);
        if ( typeBoxString == type ) { //getTypeName()
            foundType = true;
            m_pTypeCB->setCurrentItem(typeBoxCount);
        } else {
            typeBoxCount++;
        }
    }

    if (!foundType) {
        m_pTypeCB->insertItem( type, 0 );
        m_pTypeCB->setCurrentItem(0);
    }

    // set tab order
    setTabOrder(m_pKind, m_pTypeCB);
    setTabOrder(m_pTypeCB, m_pNameLE);
    setTabOrder(m_pNameLE, m_pInitialLE);
    setTabOrder(m_pInitialLE, m_pStereoTypeLE);
    setTabOrder(m_pStereoTypeLE, m_pIn);
    setTabOrder(m_pIn, m_pDoc);

    m_pNameLE->setFocus();
}

Uml::Parameter_Direction ParmPropDlg::getParmKind() {
    Uml::Parameter_Direction pk = Uml::pd_In;
    if (m_pOut->isChecked())
        pk = Uml::pd_Out;
    else if (m_pInOut->isChecked())
        pk = Uml::pd_InOut;
    return pk;
}

void ParmPropDlg::slotOk() {
    if (m_pAtt != NULL) {
        m_pAtt->setParmKind( getParmKind() );
        m_pAtt->setStereotype( m_pStereoTypeLE->text() );
        QString typeName = m_pTypeCB->currentText();
        UMLClassifier * pConcept = dynamic_cast<UMLClassifier*>( m_pAtt->parent()->parent() );
        if (pConcept == NULL) {
            kdError() << "ParmPropDlg::slotOk: grandparent of " << m_pAtt->getName()
            << " is not a UMLClassifier" << endl;
        } else {
            UMLTemplate *tmplParam = pConcept->findTemplate(typeName);
            if (tmplParam) {
                m_pAtt->setType(tmplParam);
                accept();
                return;
            }
        }
        UMLClassifierList namesList( m_pUmldoc->getConcepts() );
        UMLClassifier * obj;
        for (obj = namesList.first(); obj; obj = namesList.next()) {
            if (obj->getFullyQualifiedName() == typeName) {
                m_pAtt->setType( obj );
                break;
            }
        }
        if (obj == NULL) {
            // Nothing found: Create a new type on the fly.
            // @todo There should be an extra dialog to decide whether to
            // create a datatype or a class. For now, we create a class.
            kdDebug() << "ParmPropDlg::slotOk: " << typeName << " not found."
                << " Creating a new class for the type." << endl;
            UMLObject *o = Object_Factory::createUMLObject(Uml::ot_Class, typeName);
            m_pAtt->setType(o);
        }

    }
    accept();
}

ParmPropDlg::~ParmPropDlg() {}

