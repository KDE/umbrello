/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSGENPAGE_H
#define CLASSGENPAGE_H

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

/**
 *	Displays properties of a UMLObject in a dialog box.  This is not usually directly
 *	called.  The class @ref ClassPropDlg	will set this up for you.
 *
 *	@short	Display properties on a UMLObject.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version 1.0
 */
class ClassGenPage : public QWidget {
	Q_OBJECT
public:

	/**
	 *	Sets up the ClassGenPage.
	 *
	 *	@param	d	The UMLDoc which controls controls object creation.
	 *	@param	parent	The parent to the ClassGenPage.
	 *	@param	o	The UMLObject to display the properties of.
	 */
	ClassGenPage(UMLDoc *d, QWidget *parent, UMLObject * o);

	/**
	 *	Sets up the ClassGenPage for an ObjectWidget
	 *
	 *	@param	d	The UMLDoc which controls controls object creation.
	 *	@param	parent	The parent to the ClassGenPage.
	 *	@param	o	The ObjectWidget to display the properties of.
	 */
	ClassGenPage(UMLDoc *d, QWidget *parent, ObjectWidget * o);

	/**
	 *	Standard deconstructor.
	 */
	~ClassGenPage();

	/**
	 *	Will move information from the dialog into the object.
	 *	Call when the ok or apply button is pressed.
	 */
	void updateObject();
private:
	QGroupBox *m_pDocGB;
	QButtonGroup	* m_pButtonBG;
	QLabel * m_pNameL, * m_pInstanceL, * m_pStereoTypeL, * m_pPackageL;
	QLineEdit * m_pClassNameLE, *m_pInstanceLE, * m_pStereoTypeLE, * m_pPackageLE;
	QRadioButton * m_pPublicRB, * m_pPrivateRB, * m_pProtectedRB;
	QCheckBox * m_pMultiCB, * m_pDrawActorCB, * m_pAbstractCB, * m_pDeconCB;
	QMultiLineEdit * m_pDoc;
	UMLObject * m_pObject;
	UMLDoc * m_pUmldoc;
	ObjectWidget * m_pWidget;
	QButtonGroup* m_pDrawAsBG;
	QRadioButton* m_pDefaultRB,* m_pFileRB,* m_pLibraryRB,* m_pTableRB;

public slots:
	/**
	 * 	When the draw as actor check box is toggled, the draw
	 * 	as multi instance need to be enabled/disabled.  They
	 * 	both can't be available at the same time.
	 */
	void slotActorToggled( bool state );
};

#endif
