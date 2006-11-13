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
#include "selectopdlg.h"

// qt includes
#include <qlayout.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>

// local includes
#include "../attribute.h"
#include "../operation.h"
#include "../umlclassifierlistitemlist.h"
#include "../umlview.h"
#include "../dialog_utils.h"

SelectOpDlg::SelectOpDlg(UMLView * parent, UMLClassifier * c)
        : KDialogBase(Plain, i18n("Select Operation"), Ok | Cancel , Ok, parent, "_SELOPDLG_", true, true)
{
    m_pView = parent;
    QVBoxLayout * topLayout = new QVBoxLayout(plainPage());

    m_pOpGB = new QGroupBox(i18n("Select Operation"), plainPage());
    topLayout -> addWidget(m_pOpGB);

    QGridLayout * mainLayout = new QGridLayout(m_pOpGB, 3, 2);
    mainLayout -> setSpacing(spacingHint());
    mainLayout -> setMargin(fontMetrics().height());

    Dialog_Utils::makeLabeledEditField( m_pOpGB, mainLayout, 0,
                                    m_pSeqL, i18n("Sequence number:"),
                                    m_pSeqLE );

    m_pOpRB = new QRadioButton(i18n("Class operation:"), m_pOpGB);
    mainLayout -> addWidget(m_pOpRB, 1, 0);

    m_pOpCB = new KComboBox(m_pOpGB);
    m_pOpCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    m_pOpCB->setDuplicatesEnabled(false);//only allow one of each type in box
    mainLayout -> addWidget(m_pOpCB, 1, 1);

    m_pCustomRB = new QRadioButton(i18n("Custom operation:"), m_pOpGB);
    mainLayout -> addWidget(m_pCustomRB, 2, 0);

    m_pOpLE = new QLineEdit(m_pOpGB);
    mainLayout -> addWidget(m_pOpLE, 2, 1);

    m_pOpBG = new QButtonGroup(0);
    m_pOpBG -> insert(m_pOpRB, OP);
    m_pOpBG -> insert(m_pCustomRB, CUSTOM);
    m_pOpBG -> setExclusive(true);
    m_pOpBG -> setButton(OP);

    UMLOperationList list = c -> getOpList(true);
    for (UMLOperation *obj = list.first(); obj; obj=list.next()) {
        insertOperation( obj->toString(Uml::st_SigNoVis) );
    }
    //disableResize();
    connect(m_pOpBG, SIGNAL(clicked(int)), this, SLOT(slotSelected(int)));
    m_nOpCount = c -> operations();
    slotSelected(OP);
}

SelectOpDlg::~SelectOpDlg() {
    disconnect(m_pOpBG, SIGNAL(clicked(int)), this, SLOT(slotSelected(int)));
    delete m_pOpBG;
}

void SelectOpDlg::insertOperation( const QString& type, int index )
{
    m_pOpCB->insertItem( type, index );
    m_pOpCB->completionObject()->addItem( type );
}

QString SelectOpDlg::getOpText() {
    if(m_pOpRB -> isChecked())
        return m_pOpCB -> currentText();
    else
        return m_pOpLE -> text();
}

bool SelectOpDlg::isClassOp() const {
    return (m_id == OP);
}

void SelectOpDlg::slotSelected(int id) {
    if(id == OP) {
        m_pOpLE -> setEnabled(false);
        if(m_nOpCount > 0)
            m_pOpCB -> setEnabled(true);
    } else {
        m_pOpLE -> setEnabled(true);
        m_pOpCB -> setEnabled(false);
    }
    m_id = id;
}

void SelectOpDlg::setCustomOp(const QString &op) {
    m_pOpLE -> setText(op);
    if(op.length() > 0) {
        slotSelected(CUSTOM);
        m_pCustomRB -> setChecked(true);
    }
}

bool SelectOpDlg::setClassOp(const QString &op) {
    for (int i = 1; i!= m_pOpCB->count(); ++i)
    {
        if ( m_pOpCB->text(i) == op ) {
            m_pOpCB->setCurrentItem(i);
            m_pCustomRB -> setChecked(false);
            slotSelected(OP);
            return true;
        }
    }
    return false;
}

QString SelectOpDlg::getSeqNumber() {
    return m_pSeqLE->text();
}

void SelectOpDlg::setSeqNumber(const QString &num) {
    m_pSeqLE->setText(num);
}



#include "selectopdlg.moc"
