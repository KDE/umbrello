/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef ASSOCPROPDLG_H
#define ASSOCPROPDLG_H

#include <qlineedit.h>
#include <qgroupbox.h>
#include <qmultilineedit.h>

#include <kdialogbase.h>

#include "../associationwidget.h"

/**
 * This dialog box will display the properties of an @ref Association.
 * Most fields are read-only and change depending on what type of
 * association it is displaying.
 *
 * @see ClassPropDlg
 * @see ConceptWidget
 *
 * @short	This dialog box will display the properties of an @ref Association.
 * @author Paul Hensgen	<phensgen@techie.com>
 * @version 1.0
 */
class AssocPropDlg : public KDialogBase {
public:
	/**
	 *	Constructs an AssocPageDlg.
	 *
	 *	@param	parent	The parent of this class.
	 *	@param	a	The @ref Association to show properties for.
	 */
	AssocPropDlg(QWidget * parent, AssociationWidget * a);

	/**
	 *	Standard deconstructor.
	 */
	~AssocPropDlg();

	/**
	 *	Return the text representing the role name.
	 *
	 *	@return	Return the text representing the role name.
	 */
	QString getRoleName() {
		return m_pRoleNameLE -> text();
	}

	/**
	 *	Return the text that represents the multiplicity for Widget A.
	 *
	 *	@return	Return the text that represents the multiplicity for Widget A
	 */
	QString getMultiA() {
		return m_pMultiALE -> text();
	}

	/**
	 *	Return the text that represents the multiplicity for Widget B.
	 *
	 *	@return	Return the text that represents the multiplicity for Widget A
	 */
	QString getMultiB() {
		return m_pMultiBLE -> text();
	}
public slots:
	void slotOk();
private:
	QLineEdit * m_pNameALE, * m_pNameBLE, * m_pMultiALE, * m_pMultiBLE, *m_pRoleNameLE, * m_pTypeLE;
	QGroupBox * m_pAssocGB, * m_pDocGB;
	QMultiLineEdit * m_pDocMLE;
	AssociationWidget * assocWidget;
};

#endif
