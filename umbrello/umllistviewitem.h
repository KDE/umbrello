/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLLISTVIEWITEM_H
#define UMLLISTVIEWITEM_H

#include <QtGui/QTreeWidget>
#include <QtCore/QMap>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include "umlnamespace.h"
#include "icon_utils.h"

// forward declarations
class UMLListView;
class UMLObject;
class UMLClassifierListItem;

typedef QTreeWidgetItemIterator UMLListViewItemIterator;

/**
 * Items used by the class @ref UMLListView.  This is needed as the type
 * and object information is required to be stored.
 *
 * @short  Items used by @ref UMLListView.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see    UMLListView
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLListViewItem : public QTreeWidgetItem
{
public:

    UMLListViewItem(UMLListView * parent, const QString &name, Uml::ListView_Type t, UMLObject*o=0);
    UMLListViewItem(UMLListView * parent);
    UMLListViewItem(UMLListViewItem * parent);
    UMLListViewItem(UMLListViewItem * parent, const QString &name, Uml::ListView_Type t, UMLObject*o=0);
    UMLListViewItem(UMLListViewItem * parent, const QString &name, Uml::ListView_Type t, Uml::IDType id);

    ~UMLListViewItem();

    Uml::ListView_Type getType() const;

    void setID(Uml::IDType id);
    Uml::IDType getID() const;

    void setUMLObject(UMLObject * obj);
    UMLObject * getUMLObject() const;

    bool isOwnParent(Uml::IDType listViewItemID);

    void updateObject();
    void updateFolder();

    void setText(int column, const QString &text);
    void setText(const QString &text );
    QString getText() const;
    void setVisible(bool state);

    QString toolTip();

    void setCreating(bool creating);

    void setIcon(Icon_Utils::Icon_Type iconType);

    void startRename(int col);
    void cancelRename(int col);

    void addClassifierListItem(UMLClassifierListItem *child, UMLListViewItem *childItem);

    void deleteChildItem(UMLClassifierListItem *child);

    //virtual int compare(UMLListViewItem *other, int col, bool ascending) const;

    UMLListViewItem* deepCopy(UMLListViewItem *newParent);

    UMLListViewItem* findUMLObject(const UMLObject *o);
    UMLListViewItem* findChildObject(UMLClassifierListItem *cli);
    UMLListViewItem* findItem(Uml::IDType id);

    int childCount() const;
    UMLListViewItem* childItem(int i);

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    bool loadFromXMI(QDomElement& qElement);

    void setOpen(bool state);
    void okRename( int col );    
        
protected:
    void init(UMLListView * parent = 0);

    void cancelRenameWithMsg();

    /**
     * This list view all the instance of this class are displayed on.
     */
    static UMLListView * s_pListView;

    /**
     * Auxiliary map of child UMLLisViewItems keyed by UMLClassifierListItem.
     * Used by findChildObject() for efficiency instead of looping using
     * firstChild()/nextSibling() because the latter incur enforceItemVisible()
     * and thus expensive sorting.
     */
    typedef QMap<UMLClassifierListItem*, UMLListViewItem*> ChildObjectMap;

    bool               m_bCreating;  ///< flag to set the state of creating
    Uml::ListView_Type m_Type;
    Uml::IDType        m_nId;
    int                m_nChildren;
    UMLObject *        m_pObject;
    QString            m_Label;
    ChildObjectMap     m_comap;
    int                m_childIndex;

};

#endif
