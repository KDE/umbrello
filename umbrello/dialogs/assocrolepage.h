/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ASSOCROLEPAGE_H
#define ASSOCROLEPAGE_H

#include <qwidget.h>
class QRadioButton;
class QLineEdit;
class QMultiLineEdit;

class UMLAssociation;


/**
 *	Dialog page to display / set detailed information of a UMLAssociaton
 *
 *	Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssocRolePage : public QWidget {
	Q_OBJECT
public:
	/**
	 *	Constructor
	 *
	 *	@param	parent	The parent Widget of this page
	 *	@param	a	The UMLAssociation to observe
	 */
	AssocRolePage( QWidget *parent, UMLAssociation *a);

	/**
	 *	destructor
	 */
	virtual ~AssocRolePage();

	/**
	 *	Will move information from the dialog into the object.
	 *	Called when the ok or apply button is pressed.
	 */
	void updateObject();

private:
	void constructWidget();
	
	UMLAssociation *m_assoc;
	
	QLineEdit *m_pRoleALE, *m_pRoleBLE, *m_pMultiALE, *m_pMultiBLE;
	QMultiLineEdit *m_pDocA, *m_pDocB;
	
	QRadioButton *m_PublicARB, *m_ProtectedARB, *m_PrivateARB;
	QRadioButton *m_PublicBRB, *m_ProtectedBRB, *m_PrivateBRB;
	QRadioButton *m_ChangeableARB, *m_AddOnlyARB, *m_FrozenARB;
	QRadioButton *m_ChangeableBRB, *m_AddOnlyBRB, *m_FrozenBRB;
};

#endif
