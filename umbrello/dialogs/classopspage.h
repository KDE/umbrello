/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSOPSPAGE_H
#define CLASSOPSPAGE_H

#include <qwidget.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qtextedit.h>

#include <karrowbutton.h>

#include "../classifier.h"
#include "../listpopupmenu.h"
class UMLDoc;

/**
 *	A dialog page to display class operation properties.  This is not normally setup
 *	called by you.  It is used by the @ref ClassPropDlg.
 *
 *	@short	A dialog page to display class operation properties.
 *	@author Paul Hensgen <phensgen@techie.com>
 *	@see	ClassPropDlg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ClassOpsPage : public QWidget {
	Q_OBJECT
public:

	/**
	 *	Sets up the ClassOpsPage.
	 *
	 *	@param	parent	The parent to the ClassOpsPage.
	 *	@param	c	The Concept to display the properties of.
	 */
	ClassOpsPage(QWidget *parent, UMLClassifier *c, UMLDoc * doc);

	/**
	 *	Standard deconstructor.
	 */
	~ClassOpsPage();


	/**
	 *	Will move information from the dialog into the object.
	 *	Call when the ok or apply button is pressed.
	 */
	void updateObject();

private:
	UMLClassifier * m_pConcept;
	QGroupBox * m_pOpsGB, * m_pDocGB;
	QListBox * m_pOpsLB;
	QTextEdit * m_pDocTE;
	KArrowButton * m_pUpArrowB, * m_pDownArrowB;
	QPushButton* m_pDeleteOperationButton;
	QPushButton* m_pPropertiesButton;

	UMLOperation * m_pOldOp;
	ListPopupMenu * m_pMenu;
	UMLDoc * m_pDoc;
	bool m_bSigWaiting;
	QPtrList<UMLOperation> * m_pOpList;
	/**
	 *	Set the state of the widgets on the page with the given value.
	 *
	 *	@param	state	The state to set the widgets as.
	 */
	void enableWidgets(bool state);

	/**
	 * Saves the documentation for the currently selected item
	 */
	void saveCurrentItemDocumentation();
signals:
	void sigUpdateChildObject(int id);
public slots:
	/**
	 * called when list view is clicked on
	 * calls enableWidgets()
	 */
	void slotClicked(QListBoxItem *i);

	void slotOpPopupMenuSel(int id);
	void slotOpRightButtonPressed(QListBoxItem *item, const QPoint &p);
	void slotOpRightButtonClicked(QListBoxItem *item, const QPoint &p);
	void slotOperationCreated(UMLObject * object);
	void slotOperationRenamed(UMLObject * object);

	/**
	 * shows properties dialogue for the operation clicked on
	 */
	void slotDoubleClick( QListBoxItem * item );

	/**
	 * moves selected operation up in list
	 */
	void slotUpClicked();

	/**
	 * shows dialogue for new operation
	 */
	void slotNewOperation();

	/**
	 * moved selected operation down in list
	 */
	void slotDownClicked();

	/**
	 * removes currently seleted operation
	 */
	void slotDelete();

	/**
	 * shows properties dialogue for currently selected operation
	 */
	void slotProperties();
};

#endif
