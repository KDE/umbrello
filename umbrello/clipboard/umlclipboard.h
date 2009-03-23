/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLCLIPBOARD_H
#define UMLCLIPBOARD_H

#include "associationwidgetlist.h"
#include "umlobjectlist.h"
#include "umllistviewitemlist.h"
#include "umllistviewitem.h"
#include "umlviewlist.h"
#include "umlwidgetlist.h"

class IDChangeLog;
class QMimeData;

/**
 * This class manages the uml's interaction with the KDE
 * Clipboard. It makes possible to copy stuff from one uml
 * instance to another one.
 *
 * @short Clipboard management class
 * @author Gustavo Madrigal
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLClipboard : public QObject
{
    Q_OBJECT
public:

    UMLClipboard();

    virtual ~UMLClipboard();

    bool paste(const QMimeData* data);

    QMimeData* copy(bool fromView = false);

    /// Enumeration that codes the different types of UML clips.
    enum UMLCopyType
    {
        clip1 = 1, ///<UMLObjects (not diagrams)
        clip2 = 2, ///<UMLObjects, UMLListViewItems (not diagrams) and diagrams
        clip3 = 3, ///<UMLListViewItems (not diagrams)
        clip4 = 4, ///<UMLObjects, Associations and UMLWidgets
        clip5 = 5  ///<Only Attributes, Operations, Templates and EnumLiterals
    };

private:

    void CleanAssociations(AssociationWidgetList& associations);

    bool pasteClip1(const QMimeData* data);
    bool pasteClip2(const QMimeData* data);
    bool pasteClip3(const QMimeData* data);
    bool pasteClip4(const QMimeData* data);
    bool pasteClip5(const QMimeData* data);

    bool checkPasteWidgets(UMLWidgetList & widgetList);

    UMLObjectList m_ObjectList;
    UMLListViewItemList m_ItemList;
    UMLWidgetList m_WidgetList;
    AssociationWidgetList m_AssociationList;
    UMLViewList m_ViewList;
    UMLCopyType m_type; ///< Type of copy operation to perform.

private:

    bool fillSelectionLists(UMLListViewItemList& selectedItems);

    void setCopyType(UMLListViewItemList& selectedItems);

    void checkItemForCopyType(UMLListViewItem* item,
                              bool& withDiagrams,
                              bool& withObjects,
                              bool& onlyAttsOps);

    bool insertItemChildren(UMLListViewItem* item,
                            UMLListViewItemList& selectedItems);

    bool insertItemChildren(UMLListViewItem* item);

    bool pasteChildren(UMLListViewItem* parent, IDChangeLog *chgLog);

    void pasteItemAlreadyExists();
};

#endif
