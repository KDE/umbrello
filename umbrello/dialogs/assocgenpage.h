/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ASSOCGENPAGE_H
#define ASSOCGENPAGE_H

#include <qwidget.h>
class QLineEdit;
class QMultiLineEdit;


class UMLAssociation;

/**
 * Dialog to displays / set the general properites of a UMLAssociation
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssocGenPage : public QWidget {
	Q_OBJECT
public:
	/**
	 *	Constructor
	 *
	 *	@param	parent	The parent Widget of this page.
	 *	@param	a       The UMLAssociation to observe.
	 */
	AssocGenPage( QWidget *parent, UMLAssociation *a);

	/**
	 *	destructor
	 */
	virtual ~AssocGenPage();

	/**
	 *	Will move information from the dialog into the object.
	 *	Called when the ok or apply button is pressed.
	 */
	void updateObject();
	
private:
	void constructWidget();
	
	UMLAssociation *m_assoc;
	
	QLineEdit  *m_pAssocNameLE, *m_pTypeLE;
	QMultiLineEdit *m_pDoc;
};

#endif
