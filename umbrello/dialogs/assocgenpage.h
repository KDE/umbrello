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

//quicktime class includes
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qmultilineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

//my class includes
#include "../umlobject.h"
#include "../objectwidget.h"
#include "../umldoc.h"
#include "../associationwidget.h"

/**
 *	Displays properties of a UMLObject in a dialog box.  This is not usually directly
 *	called.  The class @ref AssocPropDlg	will set this up for you.
 *
 *	@short	Display properties on a UMLObject.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version 1.0
 */
class AssocGenPage : public QWidget {
	Q_OBJECT
public:

	/**
	 *	Sets up the AssocGenPage.
	 *
	 *	@param	d	The UMLDoc which controls controls object creation.
	 *	@param	parent	The parent to the AssocGenPage.
	 *	@param	o	The ObjectWidget to display the properties of.
	 */
	AssocGenPage(UMLDoc *d, QWidget *parent, AssociationWidget *a);

	/**
	 *	Standard deconstructor.
	 */
	~AssocGenPage();

	/**
	 *	Will move information from the dialog into the object.
	 *	Call when the ok or apply button is pressed.
	 */
	void updateObject();

private:
	QLineEdit * m_pAssocNameLE, *m_pTypeLE;
	QMultiLineEdit * m_pDoc;

	AssociationWidget *m_pAssociationWidget;
	UMLDoc * m_pUmldoc;
	ObjectWidget * m_pWidget;

	void constructWidget();

public slots:
	/**
	 * 	When the draw as actor check box is toggled, the draw
	 * 	as multi instance need to be enabled/disabled.  They
	 * 	both can't be available at the same time.
	 */
	// void slotActorToggled( bool state );
};

#endif
