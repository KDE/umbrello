/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLTEMPLATEDIALOG_H
#define UMLTEMPLATEDIALOG_H

#include <kdialogbase.h>

/** //FIXME
 * @author Paul Hensgen
 */

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QRadioButton;
class UMLTemplate;

class UMLTemplateDialog : public KDialogBase {
	Q_OBJECT
public:
	UMLTemplateDialog(QWidget* pParent, UMLTemplate* pAttribute);
	~UMLTemplateDialog();

protected:
	/**
	*	Sets up the dialog
	*/
	void setupDialog();

	/**
	 * Checks if changes are valid and applies them if they are,
	 * else returns false
	 */
	bool apply();

	/**
	*	The Attribute to represent
	*/
	UMLTemplate* m_pTemplate;

	//GUI Widgets
	QGroupBox* m_pTemplateGB;
	QGroupBox* m_pValuesGB;
	QLabel* m_pTypeL, * m_pNameL;
	QComboBox* m_pTypeCB;
	QLineEdit* m_pNameLE;

public slots:
	/**
	 * I don't think this is used, but if we had an apply button
	 * it would slot into here
	 */
	void slotApply();

	/**
	 * Used when the OK button is clicked.  Calls apply()
	 */
	void slotOk();
};

#endif
