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

#include <qgroupbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qlabel.h>
#include "../classifier.h"


#define OP 				0
#define CUSTOM 	1

/**
 *	A dialog used to select an operation.
 *
 *	@short A dialog to select an operation.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version	1.0
 */

class SelectOpDlg : public KDialogBase
{
	Q_OBJECT
public:
	/**
	 *	Constructs a SelectOpDlg instance.
	 *
	 *	@param	parent	The parent to this instance.
	 *	@param	c	The concept to get the operations from.
	 */
	SelectOpDlg(QWidget * parent, UMLClassifier * c);

	/**
	 *	Standard deconstructor.
	 */
	~SelectOpDlg();

	/**
	 *	Returns the operation to display.
	 *
	 *	@return The operation to display.
	 */
	QString getOpText();

	/**
	 *	Returns the sequence number for the operation.
	 *
	 *	@return Returns the sequence number for the operation.
	 */
	QString getSeqNumber() {
		return m_pSeqLE -> text();
	}

	/**
	 * Set the sequence number text.
	 *
	 *	@param	num	The number to set the sequence to.
	 */
	void setSeqNumber(QString num) {
		m_pSeqLE -> setText(num);
	}

	/**
	 * Set the custom operation text.
	 *
	 *	@param op The operation to set as the custom operation.
	 */
	void setCustomOp(QString op);
private:
	QGroupBox * m_pOpGB;
	QComboBox *	m_pOpCB;
	QLabel * m_pSeqL;
	QLineEdit * m_pOpLE, * m_pSeqLE;
	QRadioButton * m_pCustomRB, * m_pOpRB;
	QButtonGroup * m_pOpBG, * m_pDocGB;
	QString m_Text;
	int m_nOpCount;
public slots:
	void slotSelected(int id);
};

#endif

