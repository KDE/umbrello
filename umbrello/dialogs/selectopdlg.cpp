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
#include "selectopdlg.h"

// local includes
#include "attribute.h"
#include "debug_utils.h"
#include "operation.h"
#include "umlclassifierlistitemlist.h"
#include "umlview.h"
#include "dialog_utils.h"

// kde includes
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>

// qt includes
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QGroupBox>

SelectOpDlg::SelectOpDlg(QWidget * parent, UMLClassifier * c)
   : KDialog(parent)
{
    setCaption( i18n("Select Operation") );
    setButtons( Ok | Cancel );
    setDefaultButton( Yes );
    setModal( true );
    showButtonSeparator( true );

    QFrame *frame = new QFrame( this );
    setMainWidget( frame );

    QVBoxLayout * topLayout = new QVBoxLayout(frame);

    m_pOpGB = new QGroupBox(i18n("Select Operation"), frame);
    topLayout->addWidget(m_pOpGB);

    QGridLayout * mainLayout = new QGridLayout(m_pOpGB);
    mainLayout->setSpacing(spacingHint());
    mainLayout->setMargin(fontMetrics().height());

    Dialog_Utils::makeLabeledEditField( m_pOpGB, mainLayout, 0,
                                    m_pSeqL, i18n("Sequence number:"),
                                    m_pSeqLE );

    m_pOpRB = new QRadioButton(i18n("Class operation:"), m_pOpGB);
    connect(m_pOpRB, SIGNAL(clicked()), this, SLOT(slotSelectedOp()));
    mainLayout->addWidget(m_pOpRB, 1, 0);

    m_pOpCB = new KComboBox(m_pOpGB);
    m_pOpCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    m_pOpCB->setDuplicatesEnabled(false); // only allow one of each type in box
    mainLayout->addWidget(m_pOpCB, 1, 1);

    m_pCustomRB = new QRadioButton(i18n("Custom operation:"), m_pOpGB);
    connect(m_pCustomRB, SIGNAL(clicked()), this, SLOT(slotSelectedCustom()));
    mainLayout->addWidget(m_pCustomRB, 2, 0);

    m_pOpLE = new KLineEdit(m_pOpGB);
    mainLayout->addWidget(m_pOpLE, 2, 1);

    UMLOperationList list = c->getOpList(true);
    foreach (UMLOperation* obj, list ) {
        insertOperation( obj->toString(Uml::SignatureType::SigNoVis), list.count() );
    }

    m_nOpCount = c->operations();
    m_pOpRB->click();
}

SelectOpDlg::~SelectOpDlg()
{
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void SelectOpDlg::insertOperation( const QString& type, int index )
{
    m_pOpCB->insertItem( index, type );
    m_pOpCB->completionObject()->addItem( type );
}

/**
 *  Returns the operation to display.
 *
 *  @return The operation to display.
 */
QString SelectOpDlg::getOpText()
{
    if (m_pOpRB->isChecked())
        return m_pOpCB->currentText();
    else
        return m_pOpLE->text();
}

/**
 * Return whether the user selected a class operation
 * or a custom operation.
 *
 * @return  True if user selected a class operation,
 *          false if user selected a custom operation
 */
bool SelectOpDlg::isClassOp() const
{
    return (m_id == OP);
}

void SelectOpDlg::slotSelectedOp()
{
    m_pOpLE->setEnabled(false);
    if (m_nOpCount > 0) {
        m_pOpCB->setEnabled(true);
    }
    m_id = OP;
}

void SelectOpDlg::slotSelectedCustom()
{
    m_pOpLE->setEnabled(true);
    m_pOpCB->setEnabled(false);
    m_id = CUSTOM;
}

/**
 * Set the custom operation text.
 *
 *  @param op The operation to set as the custom operation.
 */
void SelectOpDlg::setCustomOp(const QString &op)
{
    m_pOpLE->setText(op);
    if (op.length() > 0) {
        slotSelectedCustom();
        m_pCustomRB->setChecked(true);
    }
}

/**
 * Set the class operation text.
 *
 *  @param op The operation to set as the class operation.
 * @return false if no such operation exists.
 */
bool SelectOpDlg::setClassOp(const QString &op)
{
    for (int i = 1; i!= m_pOpCB->count(); ++i) {
        if ( m_pOpCB->itemText(i) == op ) {
            m_pOpCB->setCurrentIndex(i);
            m_pCustomRB->setChecked(false);
            slotSelectedOp();
            return true;
        }
    }
    return false;
}

/**
 *  Returns the sequence number for the operation.
 *
 *  @return Returns the sequence number for the operation.
 */
QString SelectOpDlg::getSeqNumber()
{
    return m_pSeqLE->text();
}

/**
 * Set the sequence number text.
 *
 *  @param  num     The number to set the sequence to.
 */
void SelectOpDlg::setSeqNumber(const QString &num)
{
    m_pSeqLE->setText(num);
}

#include "selectopdlg.moc"
