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

#include "../classwidget.h"
#include "../optionstate.h"

class ClassWidget;
class InterfaceWidget;

/**
 *	A dialog page to display options for a @ref UMLWidget and it's
 *	children.  This is not normally called by you.  It is used by
 *	the @ref ClassPropDlg.
 *
 *	@short	A dialog page to display the options for a UMLWidget.
 *	@author Paul Hensgen <phensgen@techie.com>
 *	@see	ClassPropDlg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassOptionsPage : public QWidget {
public:
	/**
	*	Constructor - observe and modify a Widget
	*/
	ClassOptionsPage(QWidget* pParent, UMLWidget* pWidget, UMLWidget_Type type);
	
	/**
	*	Constructor - observe and modify an OptionState structure
	*/
	ClassOptionsPage(QWidget* pParent, Settings::OptionState *options );

	/**
	*	destructor
	*/
	virtual ~ClassOptionsPage();

	/**
	*	Updates the widget with the dialog page properties.
	*/
	void updateUMLWidget();

	/**
	*	Sets the widget to be used.  Only used by @ref UMLView to set
	*	settings of a widget.
	*/
	void setWidget( ClassWidget * pWidget ) {
		m_pClassWidget = pWidget;
	}
protected:

	/**
	 * Creates the page with the correct options for a class/concept
	 */
	void setupClassPage();

	/**
	 * Creates the page with the correct options for an interface
	 */
	void setupInterfacePage();
	
	/**
	 * Creates the page base on de OptionState
	 */
	void setupClassPageOption();

	/**
	 * Sets the class's properties to those selected in this dialogue page.
	 */
	void updateClassWidget();

	/**
	 * Sets the interface's properties to those selected in this dialogue page.
	 */
	void updateInterfaceWidget();
	
	/**
	 * Sets the OptionState to the values selected in this dialogue page.
	 */
	void updateOptionState();

	//GUI widgets
	QGroupBox * m_pVisibilityGB;
	QCheckBox * m_pShowScopeCB, * m_pShowAttSigCB;
	QCheckBox * m_pShowOpSigCB, * m_pShowAttsCB, * m_pShowOpsCB;
	QCheckBox * m_pShowStereotypeCB, * m_pShowPackageCB;
	QCheckBox* m_pDrawAsCircleCB;

	/**
	*	The class widget to represent in the dialog page.
	*/
	ClassWidget* m_pClassWidget;
	/**
	*	The interface widget to represent in the dialog page.
	*/
	InterfaceWidget* m_pInterfaceWidget;
	/**
	*	The OptionState structure to represent in the dialog page.
	*/
	Settings::OptionState *m_options;
};
#endif
