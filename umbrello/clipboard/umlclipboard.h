/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLCLIPBOARD_H
#define UMLCLIPBOARD_H

#include "../associationwidgetlist.h"
#include "../umlobjectlist.h"
#include "../umllistviewitemlist.h"
#include "../umllistviewitem.h"
#include "../umlviewlist.h"
#include "../umlwidgetlist.h"

class IDChangeLog;
class QMimeSource;

/**
 * This class manages the uml's interaction with the KDE
 * Clipboard. It makes possible to copy stuff from one uml
 * instance to another one.
 *
 * @short Clipboard management class
 * @author Gustavo Madrigal
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLClipboard : public QObject {
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    UMLClipboard();

    /**
     * Deconstructor.
     */
    virtual ~UMLClipboard();

    /**
     * Inserts the clipboard's contents.
     *
     * @param Data              Pointer to the MIME format clipboard data.
     * @return  True for successful operation.
     */
    bool paste(QMimeSource* Data);

    /**
     * Copies the selected stuff from the list view or current diagram
     * to a QMimeSource ready to be put in the clipboard.
     *
     * @return  Pointer to the created clipboard data.
     */
    QMimeSource* copy(bool fromView = false);

    /// Enumeration that codes the different types of UML clips.
    enum UMLCopyType
    {
        clip1 = 1, ///<UMLObjects (not diagrams)
        clip2 = 2, ///<UMLObjects, UMLListViewItems (not diagrams) and diagrams
        clip3 = 3, ///<UMLListViewItems (not diagrams)
        clip4 = 4, ///<UMLObjects, Associations and UMLWidgets
        clip5 = 5  ///<Only Attributes and Operations
    };

private:
    /**
     * Cleans the list of associations taking out the ones
     * that point to an object not in m_ObjectList.
     *
     * @param associations      The list of associations to process.
     */
    void CleanAssociations(AssociationWidgetList& associations);

    /**
     * If clipboard has mime type application/x-uml-clip1,
     * pastes the data from the clipboard.
     *
     * @param data              Pointer to the source clip.
     * @return  True for successful operation.
     */
    bool pasteClip1(QMimeSource* data);

    /**
     * If clipboard has mime type application/x-uml-clip2,
     * pastes the data from the clipboard.
     *
     * @param data              Pointer to the source clip.
     * @return  True for successful operation.
     */
    bool pasteClip2(QMimeSource* data);

    /**
     * If clipboard has mime type application/x-uml-clip3,
     * pastes the data from the clipboard.
     *
     * @param data              Pointer to the source clip.
     * @return  True for successful operation.
     */
    bool pasteClip3(QMimeSource* data);

    /**
     * If clipboard has mime type application/x-uml-clip4,
     * pastes the data from the clipboard.
     *
     * @param data              Pointer to the source clip.
     * @return  True for successful operation.
     */
    bool pasteClip4(QMimeSource* data);

    /**
     * If clipboard has mime type application/x-uml-clip5,
     * pastes the data from the clipboard.
     *
     * @param data              Pointer to the source clip.
     * @return  True for successful operation.
     */
    bool pasteClip5(QMimeSource* data);

    /**
     * When pasting widgets see if they can be pasted on
     * different diagram types.  Will return true if all the
     * widgets to be pasted can be.  At the moment this only
     * includes NoteWidgets and lines of text.
     *
     * @param widgetList        List of widgets to examine.
     * @return  True if all widgets can be put on different diagrams.
     */
    bool checkPasteWidgets(UMLWidgetList & widgetList);

    UMLObjectList m_ObjectList;
    UMLListViewItemList m_ItemList;
    UMLWidgetList m_WidgetList;
    AssociationWidgetList m_AssociationList;
    UMLViewList m_ViewList;
    UMLCopyType m_type; ///< Type of copy operation to perform.

private:
    /**
     * Fills the member lists with all the objects and other
     * stuff to be copied to the clipboard.
     */
    bool fillSelectionLists(UMLListViewItemList& SelectedItems);

    /**
     * Checks the whole list to determine the copy action
     * type to be * performed, sets the type in the m_type
     * member variable.
     */
    void setCopyType(UMLListViewItemList& SelectedItems);

    /**
     * Searches the child items of a UMLListViewItem to
     * establish which Copy type is to be perfomed.
     */
    void checkItemForCopyType(UMLListViewItem* Item,
                              bool& WithDiagrams,
                              bool& WithObjects,
                              bool& OnlyAttsOps);

    /**
     * Adds the children of a UMLListViewItem to m_ItemList.
     */
    bool insertItemChildren(UMLListViewItem* Item,
                            UMLListViewItemList& SelectedItems);

    /**
     * Inserts the data of the children of the given item
     * into the item data list.  Used for clip type 4.  Used
     * to make * sure classes have all the attributes and
     * operations saved.
     */
    bool insertItemChildren(UMLListViewItem* item);

    /**
     * Pastes the children of a UMLListViewItem (The Parent)
     */
    bool pasteChildren(UMLListViewItem* parent, IDChangeLog *chgLog);

    /**
     * Gives a `sorry' message box if you're pasting an item which
     * already exists and can't be duplicated.
     */
    void pasteItemAlreadyExists();
};

#endif
