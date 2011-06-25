/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "umlcheckconstraintdialog.h"

// qt includes
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>

// kde includes
#include <klocale.h>
#include <ktextedit.h>
#include <klineedit.h>

#include "uml.h"
#include "umldoc.h"
#include "checkconstraint.h"

UMLCheckConstraintDialog::UMLCheckConstraintDialog(QWidget* parent, UMLCheckConstraint* pCheckConstraint)
  : KDialog(parent)
{
    setCaption( i18n("Check Constraint Properties") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );

    m_pCheckConstraint = pCheckConstraint;
    m_doc = UMLApp::app()->document();

    setupDialog();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
}

UMLCheckConstraintDialog::~UMLCheckConstraintDialog()
{
}

/**
 *   Sets up the dialog
 */
void UMLCheckConstraintDialog::setupDialog()
{
    QFrame *frame = new QFrame( this );
    setMainWidget( frame );

    //main layout contains the name fields, the text field
    QVBoxLayout* mainLayout = new QVBoxLayout( frame );
    mainLayout->setSpacing(15);

    // layout to hold the name label and line edit
    QHBoxLayout* nameLayout = new QHBoxLayout();
    mainLayout->addItem( nameLayout );

    // name label
    m_pNameL = new QLabel( i18nc("name label", "Name"), this );
    nameLayout->addWidget( m_pNameL );
    // name lineEdit
    m_pNameLE = new KLineEdit( this );
    nameLayout->addWidget( m_pNameLE );

    QVBoxLayout* checkConditionLayout = new QVBoxLayout();
    mainLayout->addItem( checkConditionLayout );

    m_pCheckConditionL = new QLabel( i18n( "Check Condition :" ), frame );
    checkConditionLayout->addWidget( m_pCheckConditionL );

    m_pCheckConditionTE = new KTextEdit( frame );
    checkConditionLayout->addWidget( m_pCheckConditionTE );

    // set text of text edit
    m_pCheckConditionTE->setText(m_pCheckConstraint->getCheckCondition());

    // set text of label
    m_pNameLE->setText( m_pCheckConstraint->name() );
}

/**
 * Used when the OK button is clicked. Calls apply()
 */
void UMLCheckConstraintDialog::slotOk()
{
    if ( apply() ) {
        accept();
    }
}

/**
 * Apply Changes
 */
bool UMLCheckConstraintDialog::apply()
{
    m_pCheckConstraint->setCheckCondition( m_pCheckConditionTE->toPlainText().trimmed() );

    // set name
    m_pCheckConstraint->setName( m_pNameLE->text().trimmed() );

    return true;
}

#include "umlcheckconstraintdialog.moc"
