/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSATTPAGE_H
#define CLASSATTPAGE_H

//quicktime  includes
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qtextedit.h>
//kde includes
#include <karrowbutton.h>

//app includes
#include "../concept.h"
#include "../listpopupmenu.h"
class UMLDoc;

/**
 *	A dialog page to display class attribute properties.  This is not normally setup
 *	called by you.  It is used by the @ref ClassPropDlg.
 *
 *	@short	A dialog page to display class attribute properties.
 *	@author Paul Hensgen <phensgen@techie.com>
 *	@version	1.0
 *	@see	ClassPropDlg
 */
class ClassAttPage : public QWidget {
	Q_OBJECT
public:
	/**
	 *	Sets up the ClassAttPage.
	 *
	 *	@param	parent	The parent to the ClassAttPage.
	 *	@param	c	The Concept to display the properties of.
	 */
	ClassAttPage(QWidget *parent, UMLConcept *c, UMLDoc * doc);

	/**
	 *	Standard deconstructor.
	 */
	~ClassAttPage();

	/**
	 *	Will move information from the dialog into the object.
	 *	Call when the ok or apply button is pressed.
	 */
	void updateObject();

private:
	/**
	 *	Set the state of the widgets on the page with the given value.
	 *
	 *	@param	state	The state to set the widgets as.
	 */
	void enableWidgets(bool state);

	UMLConcept * m_pConcept;
	QGroupBox * m_pDocGB, * m_pAttsGB;
	QListBox * m_pAttsLB;
	QTextEdit * m_pDocTE;
	QPtrList<UMLAttribute> * m_pAttList;

	KArrowButton * m_pUpArrowB, * m_pDownArrowB;
	QPushButton* m_pDeleteAttributeButton;
	QPushButton* m_pPropertiesButton;

	UMLAttribute * m_pOldAtt;
	UMLDoc * m_pDoc;
	ListPopupMenu * m_pMenu;
	bool m_bSigWaiting;

public slots:
	/**
	 * called when list view is clicked on
	 * calls enableWidgets()
	 */
	void slotClicked(QListBoxItem *item);

	void slotPopupMenuSel(int id);
	void slotAttributeCreated(UMLObject * object);
	void slotAttributeRenamed(UMLObject * object);
	void slotRightButtonClicked(QListBoxItem * item, const QPoint & p);
	void slotRightButtonPressed(QListBoxItem * item, const QPoint & p);

	/**
	 * shows properties dialogue for the attribute clicked on
	 */
	void slotDoubleClick( QListBoxItem * item );


	/**
	 * moves selected attribute up in list
	 */
	void slotUpClicked();

	/**
	 * moved selected attribute down in list
	 */
	void slotDownClicked();

	/**
	 * shows dialogue for new attribute
	 */
	void slotNewAttribute();

	/**
	 * removes currently seleted attribute
	 */
	void slotDelete();

	/**
	 * shows properties dialogue for currently selected attribute
	 */
	void slotProperties();
signals:
	void sigUpdateChildObject(int id);
};

#endif
