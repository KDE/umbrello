/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLCLIPBOARD_H
#define UMLCLIPBOARD_H

#include "../associationwidgetlist.h"
#include "../umlobjectlist.h"
#include "../umllistviewitemdatalist.h"
#include "../umllistviewitemlist.h"
#include "../umlviewlist.h"
#include "../umlwidgetlist.h"

class IDChangeLog;
class QMimeSource;
class UMLDoc;
class UMLDrag;

/**
 *	This class manages the uml's interaction with the KDE
 *	Clipboard. It makes possible to copy stuff from one uml
 *	instance to another one.
 *
 *	@author Gustavo Madrigal
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLClipboard : public QObject {
public:
	/**
	* 	Constructor
	*/
	UMLClipboard();

	/**
	* 	Deconstructor
	*/
	virtual ~UMLClipboard();

	/**
	* 	Inserts the clipboard's contents into an UML Document
	*/
	bool paste(UMLDoc * Doc, QMimeSource* Data);

	/**
	*	Copies the selected stuff from an UMLDocument to a
	*	QMimeSource ready to be put in the clipboard
	*/
	QMimeSource* copy(UMLDoc * Doc, bool fromView = false);

	enum UMLCopyType
	{
	    clip1 = 1, //UMLObjects and UMLListViewItems (not diagrams)
	    clip2 = 2, //UMLObjects, UMLListViewItems (not diagrams) and diagrams
	    clip3 = 3, //UMLListViewItems (not diagrams)
	    clip4 = 4, //UMLObjects, UMLListViewItems, Associations and UMLWidgets
	    clip5 = 5  //Only Attributes and Operations
	};
private:
	/**
	*	Cleans the list of associations taking out the ones
	*	that point to an object not in m_pObjectList
	*/
	void CleanAssociations(AssociationWidgetList& associations);

	/**
	*	If clipboard has mime type application/x-uml-clip1,
	*	Pastes the data from the clipboard into the current
	*	Doc
	*/
	bool pasteClip1(UMLDoc* doc, QMimeSource* data);

	/**
	*	If clipboard has mime type application/x-uml-clip2,
	*	Pastes the data from the clipboard into the current
	*	Doc
	*/
	bool pasteClip2(UMLDoc* doc, QMimeSource* data);

	/**
	*	If clipboard has mime type application/x-uml-clip3,
	*	Pastes the data from the clipboard into the current
	*	Doc
	*/
	bool pasteClip3(UMLDoc* doc, QMimeSource* data);

	/**
	*	If clipboard has mime type application/x-uml-clip4,
	*	Pastes the data from the clipboard into the current
	*	Doc
	*/
	bool pasteClip4(UMLDoc* doc, QMimeSource* data);

	/**
	*	If clipboard has mime type application/x-uml-clip5,
	*	Pastes the data from the clipboard into the current
	*	Doc
	*/
	bool pasteClip5(UMLDoc* doc, QMimeSource* data);

	/**
	*	When pasting widgets see if they can be pasted on
	*	different diagram types.  Will return true if all the
	*	widgets to be pasted can be.  At the moment this only
	*	includes NoteWidgets and lines of text.
	*/
	bool checkPasteWidgets(UMLWidgetList & widgetList);

	UMLObjectList* m_pObjectList;
	UMLListViewItemDataList* m_pItemDataList;
	UMLWidgetList* m_pWidgetList;
	AssociationWidgetList* m_pAssociationList;
	UMLViewList* m_pViewList;
	UMLCopyType m_type; //Type of copy operation to perform

private:
	/**
	*	Fills the member lists with all the objects and other
	*	stuff to be copied to the clipboard
	*/
	bool fillSelectionLists(UMLListViewItemList& SelectedItems);

	/**
	* 	Checks the whole list to determine the copy action
	* 	type to be * performed, sets the type in the m_type
	* 	member variable
	*/
	void setCopyType(UMLListViewItemList& SelectedItems, UMLDoc* Doc);

	/**
	*	Searches the child items of a UMLListViewItem to
	*	establish * with Copy type is to be perfomed
	*/
	void checkItemForCopyType(UMLListViewItem* Item, bool & WithDiagrams, bool &WithObjects, bool &OnlyAttsOps,	UMLDoc* Doc);

	/**
	 *	Adds the children of a UMLListViewItem to m_pItemList
	 */
	bool insertItemChildren(UMLListViewItem* Item, UMLListViewItemList& SelectedItems);

	/**
	 * 	Inserts the data of the children of the given item
	 * 	into the item data list.  Used for clip type 4.  Used
	 * 	to make * sure classes have all the attributes and
	 * 	operations saved.
	 */
	bool insertItemDataChildren(UMLListViewItem* item);

	/**
	 *	 Pastes the children of a UMLListViewItem (The Parent)
	 */
	bool pasteChildren(UMLListViewItem* Parent, UMLListViewItemDataListIt* It,
			   IDChangeLog& ChangeLog, UMLDoc * Doc);

	/**
	 * Gives a `sorry' message box if you're pasting an item which already exists
	 * and can't be duplicated
	 */
	void pasteItemAlreadyExists(UMLDoc* doc);
};

#endif
