/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSTEMPLATEPAGE_H
#define CLASSTEMPLATEPAGE_H

//qt includes
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qtextedit.h>

//kde includes
#include <karrowbutton.h>

#include "../umldoc.h"
#include "../concept.h"

class ListPopupMenu;

/**
 *	A dialogue page to display class template properties.  It is used by the @ref ClassPropDlg.
 *
 *	@short	A dialogue page to display class templates.
 *	@author Jonathan Riddell
 *	@see	ClassPropDlg
 */
class ClassTemplatePage : public QWidget {
	Q_OBJECT
public:
	/**
	 *	Sets up the ClassTemplatePage.
	 *
	 *	@param	parent	The parent to the ClassTemplatePage.
	 *	@param	c	The Concept to display the properties of.
	 */
	ClassTemplatePage(QWidget* parent, UMLConcept* c, UMLDoc* doc);

	/**
	 *	Standard deconstructor.
	 */
	~ClassTemplatePage();

	/**
	 *	Will move information from this dialog into the class object.
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

	UMLConcept* m_pConcept;
	QGroupBox* m_pDocGB;
	QGroupBox* m_pTemplateGB;
	QListBox* m_pTemplateLB;
	QTextEdit* m_pDocTE;
	QPtrList<UMLTemplate>* m_pTemplateList;

	KArrowButton* m_pUpArrowB;
	KArrowButton* m_pDownArrowB;
	QPushButton* m_pDeleteTemplateButton;
	QPushButton* m_pPropertiesButton;

	UMLTemplate* m_pOldTemplate;
	UMLDoc* m_pDoc;
	ListPopupMenu* m_pMenu;
	bool m_bSigWaiting;

public slots:
	/**
	 * called when list view is clicked on
	 * calls enableWidgets()
	 */
	void slotClicked(QListBoxItem* item);

	/**
	 * Called when an item is selected in the right mouse button menu
	 */
 	void slotPopupMenuSel(int id);

	/**
	 * Called when a new template has been created to update the list box
	 */
	void slotTemplateCreated(UMLObject* object);

	/**
	 * Called when a template has been renamed to update the list box
	 */
	void slotTemplateRenamed(UMLObject* object);

	/**
	 * Removes the right mouse button menu
	 */
	void slotRightButtonClicked(QListBoxItem* item, const QPoint& p);

	/**
	 * Creates the right mouse button menu
	 */
	void slotRightButtonPressed(QListBoxItem* item, const QPoint& p);

	/**
	 * shows properties dialogue for the template clicked on
	 */
	void slotDoubleClick(QListBoxItem* item);

	/**
	 * moves selected template up in list
	 */
	void slotUpClicked();

	/**
	 * moved selected template down in list
	 */
	void slotDownClicked();

	/**
	 * shows dialogue for new template
	 */
	void slotNewTemplate();

	/**
	 * removes currently seleted template
	 */
	void slotDelete();

	/**
	 * shows properties dialogue for currently selected template
	 */
	void slotProperties();
signals:
	void sigUpdateChildObject(int id);
};

#endif
