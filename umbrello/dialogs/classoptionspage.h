/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef CLASSOPTIONSPAGE_H
#define CLASSOPTIONSPAGE_H

#include <qwidget.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

#include "../conceptwidget.h"

class ConceptWidget;

/**
 *	A dialog page to display options for a @ref UMLWidget and it's
 *	children.  This is not normally called by you.  It is used by
 *	the @ref ClassPropDlg.
 *
 *	@short	A dialog page to display the options for a UMLWidget.
 *	@author Paul Hensgen <phensgen@techie.com>
 *	@version	1.0
 *	@see	ClassPropDlg
 */
class ClassOptionsPage : public QWidget {
public:
	/**
	*	Constructor
	*/
	ClassOptionsPage( QWidget * pParent, UMLWidget * pWidget );

	/**
	*	Deconstructor
	*/
	~ClassOptionsPage();

	/**
	*	Updates the widget with the dialog page properties.
	*/
	void updateUMLWidget();

	/**
	*	Sets the widget to be used.  Only used by @ref UMLView to set
	*	settings of a widget.
	*/
	void setWidget( ConceptWidget * pWidget ) {
		m_pWidget = pWidget;
	}
protected:
	//GUI widgets
	QGroupBox * m_pVisibilityGB;
	QCheckBox * m_pShowScopeCB, * m_pShowAttSigCB;
	QCheckBox * m_pShowOpSigCB, * m_pShowAttsCB, * m_pShowOpsCB;
	QCheckBox * m_pShowStereotypeCB, * m_pShowPackageCB;

	/**
	*		The widget to represent in the dialog page.
	*/
	ConceptWidget * m_pWidget;
};
#endif
