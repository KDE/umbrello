/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSIFIERLISTPAGE_H
#define CLASSIFIERLISTPAGE_H

//qt  includes
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qtextedit.h>
//kde includes
#include <karrowbutton.h>

//app includes
#include "../class.h"
#include "../listpopupmenu.h"
class UMLDoc;

/**
 *	A dialogue page to display classifier list properties.  This is not normally setup
 *	by you.  It is used by the @ref ClassPropDlg and new class wizard.
 *
 *	@short	A dialogue page to display classifier properties.
 *	@author Paul Hensgen, Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassifierListPage : public QWidget {
	Q_OBJECT
public:
	/**
	 *	Sets up the ClassifierListPage
	 *
	 *	@param parent	The parent to the ClassAttPage.
	 *	@param classifier	The Concept to display the properties of.
	 *      @param doc The UMLDoc document
	 *      @param type The type of listItem this handles
	 */
	ClassifierListPage(QWidget* parent, UMLClassifier* classifier, UMLDoc* doc, UMLObject_Type type);

	/**
	 *	Standard deconstructor.
	 */
	~ClassifierListPage();

	/**
	 *	Will move information from the dialogue into the object.
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

	/**
	 * Saves the documentation for the currently selected item
	 */
	void saveCurrentItemDocumentation();

	UMLClassifier* m_pClassifier;
	QGroupBox* m_pDocGB;
	QGroupBox* m_pItemListGB;
	QListBox* m_pItemListLB;
	QTextEdit* m_pDocTE;
	QPtrList<UMLClassifierListItem>* m_pItemList;
	UMLObject_Type itemType;

	KArrowButton* m_pUpArrowB;
	KArrowButton* m_pDownArrowB;
	QPushButton* m_pDeleteListItemButton;
	QPushButton* m_pPropertiesButton;

	UMLClassifierListItem* m_pOldListItem;
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
	 * Called when an item is selected in a right click menu
	 */
	void slotPopupMenuSel(int id);

	void slotListItemCreated(UMLObject* object);
	void slotListItemModified();
	void slotRightButtonClicked(QListBoxItem* item, const QPoint& p);
	void slotRightButtonPressed(QListBoxItem* item, const QPoint& p);

	/**
	 * shows properties dialogue for the attribute clicked on
	 */
	void slotDoubleClick(QListBoxItem* item);


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
	void slotNewListItem();

	/**
	 * removes currently seleted attribute
	 */
	void slotDelete();

	/**
	 * shows properties dialogue for currently selected attribute
	 */
	void slotProperties();
signals:
	/**
	 * slots into classpropdlg::slotUpdateChildObject() whatever that does
	 */
	void sigUpdateChildObject(int id);
};

#endif
