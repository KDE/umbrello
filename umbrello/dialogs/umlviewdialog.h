/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLVIEWDIALOG_H
#define UMLVIEWDIALOG_H
//qt includes
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qtextedit.h>
#include <qspinbox.h>
//kde includes
#include <kdialogbase.h>
#include <kfontdialog.h>
//app includes
#include "classoptionspage.h"
#include "umlwidgetcolorpage.h"
#include "classoptionspage.h"

/**
 * @author Paul Hensgen
 */
class UMLView;
class ConceptWidget;

class UMLViewDialog : public KDialogBase {
	Q_OBJECT
public:
	/**
	*	Constructor
	*/
	UMLViewDialog( QWidget * pParent, UMLView * pView );

	/**
	*	Deconstructor
	*/
	~UMLViewDialog();
protected:
	enum Page
	{
	    General = 0,
	    Color,
	    Font,
	    Class
	};

	/**
	* 	Sets up the dialog pages.
	*/
	void setupPages();

	/**
	*	Sets up the Class page
	*/
	void setupClassPage();

	/**
	*	Sets up the color page.
	*/
	void setupColorPage();

	/**
	*	Sets up font page.
	*/
	void setupFontPage();

	/**
	*	Applys the properties of the given page.
	*/
	void applyPage( Page page );

	/**
	*	Checks whether the name is unique and sets it if it is.
	*/
	void checkName();

	/**
	*	The view to represent.
	*/
	UMLView * m_pView;

	/**
	*	Used to setup widget defaults.
	*/
	ConceptWidget * m_pTempWidget;

	KFontChooser * m_pChooser;
	ClassOptionsPage * m_pOptionsPage;
	UMLWidgetColorPage * m_pColorPage;

	//GUI widgets
	QLabel * m_pNameL, * m_pSpinXL, * m_pSpinYL;
	QLineEdit * m_pNameLE;
	QGroupBox * m_pDocGB, * m_pValuesGB;
	QCheckBox * m_pSnapToGridCB, * m_pShowSnapCB;
	QTextEdit * m_pDocTE;
	QSpinBox * m_pSnapXSB, * m_pSnapYSB;
public slots:
	void slotOk();
	void slotApply();
};

#endif
