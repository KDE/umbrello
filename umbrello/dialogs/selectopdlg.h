/*
 *  copyright (C) 2002-2004
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


#ifndef SELECTOPDLG_H
#define SELECTOPDLG_H

#include <kdialogbase.h>

#include <q3groupbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <qlineedit.h>
#include <qlabel.h>
#include "../classifier.h"

class UMLView;

#define OP     0
#define CUSTOM  1

/**
 * A dialog used to select an operation.
 *
 * @short A dialog to select an operation.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class SelectOpDlg : public KDialogBase
{
    Q_OBJECT
public:
    /**
     *  Constructs a SelectOpDlg instance.
     *
     *  @param  parent  The parent to this instance.
     *  @param  c       The concept to get the operations from.
     */
    SelectOpDlg(UMLView * parent, UMLClassifier * c);

    /**
     *  Standard deconstructor.
     */
    ~SelectOpDlg();

    /**
     *  Returns the operation to display.
     *
     *  @return The operation to display.
     */
    QString getOpText();

    /**
     *  Returns the sequence number for the operation.
     *
     *  @return Returns the sequence number for the operation.
     */
    QString getSeqNumber();

    /**
     * Set the sequence number text.
     *
     *  @param  num     The number to set the sequence to.
     */
    void setSeqNumber(const QString &num);

    /**
     * Set the custom operation text.
     *
     *  @param op The operation to set as the custom operation.
     */
    void setCustomOp(const QString &op);
    /**
     * Set the class operation text.
     *
     *  @param op The operation to set as the class operation.
     * @return false if no such operation exists.
     */
    bool setClassOp(const QString &op);
private:
    Q3GroupBox * m_pOpGB;
    QComboBox * m_pOpCB;
    QLabel * m_pSeqL;
    QLineEdit * m_pOpLE, * m_pSeqLE;
    QRadioButton * m_pCustomRB, * m_pOpRB;
    Q3ButtonGroup * m_pOpBG, * m_pDocGB;
    QString m_Text;
    int m_nOpCount;
    UMLView *m_pView;
public slots:
    void slotSelected(int id);
};

#endif

