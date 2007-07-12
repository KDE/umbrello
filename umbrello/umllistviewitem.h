/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLLISTVIEWITEM_H
#define UMLLISTVIEWITEM_H

#include <qlistview.h>
#include <qmap.h>
#include <qdom.h>
#include "umlnamespace.h"

// forward declarations
class UMLListView;
class UMLObject;
class UMLClassifierListItem;

/**
 * Items used by the class @ref UMLListView.  This is needed as the type
 * and object information is required to be stored.
 *
 * @short  Items used by @ref UMLListView.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see    UMLListView
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLListViewItem : public QListViewItem {
public:
    /**
     * Sets up an instance.
     *
     * @param parent    The parent to this instance.
     * @param name              The name of this instance.
     * @param t         The type of this instance.
     * @param o         The object it represents.
     */
    UMLListViewItem(UMLListView * parent, const QString &name, Uml::ListView_Type t, UMLObject*o=0);

    /**
     * Sets up an instance for subsequent loadFromXMI().
     *
     * @param parent    The parent to this instance.
     */
    UMLListViewItem(UMLListView * parent);

    /**
     * Sets up an instance for subsequent loadFromXMI().
     *
     * @param parent    The parent to this instance.
     */
    UMLListViewItem(UMLListViewItem * parent);

    /**
     * Sets up an instance.
     *
     * @param parent    The parent to this instance.
     * @param name              The name of this instance.
     * @param t         The type of this instance.
     * @param o         The object it represents.
     */
    UMLListViewItem(UMLListViewItem * parent, const QString &name, Uml::ListView_Type t, UMLObject*o=0);

    /**
     * Sets up an instance.
     *
     * @param parent    The parent to this instance.
     * @param name              The name of this instance.
     * @param t         The type of this instance.
     * @param id                The id of this instance.
     */
    UMLListViewItem(UMLListViewItem * parent, const QString &name, Uml::ListView_Type t, Uml::IDType id);

    /**
     * Standard deconstructor.
     */
    ~UMLListViewItem();

    /**
     * Returns the type this instance represents.
     *
     * @return  The type this instance represents.
     */
    Uml::ListView_Type getType() const;

    /**
     * Sets the id this class represents.
     * This only sets the ID locally, not at the UMLObject that is perhaps
     * associated to this UMLListViewItem.
     *
     * @return  The id this class represents.
     */
    void setID(Uml::IDType id);

    /**
     * Returns the id this class represents.
     *
     * @return  The id this class represents.
     */
    Uml::IDType getID() const;

    /**
     * Set the UMLObject associated with this instance.
     *
     * @param obj  The object this class represents.
     */
    void setUMLObject(UMLObject * obj) {
        m_pObject = obj;
    }

    /**
     * Return the UMLObject associated with this instance.
     *
     * @return  The object this class represents.
     */
    UMLObject * getUMLObject() {
        return m_pObject;
    }

    /**
     * Returns true if the UMLListViewItem of the given ID is a parent of
     * this UMLListViewItem.
     */
    bool isOwnParent(Uml::IDType listViewItemID);

    /**
     * Updates the representation of the object.
     */
    void updateObject();

    /**
     * Updates the icon on a folder.
     */
    void updateFolder();

    /**
     * Overrides default method.
     * Will call default method but also makes sure correct icon is shown.
     */
    void setOpen( bool open );

    /**
     * Changes the current text and updates the tooltip.
     */
    void setText( const QString &text );

    /**
     * Returns the current text.
     */
    QString getText() const;

    /**
     * Sets if the item is in the middle of being created.
     */
    void setCreating( bool creating ) {
        m_bCreating = creating;
    }

    /**
     * Set the pixmap corresponding to the given Icon_Type.
     */
    void setIcon(Uml::Icon_Type iconType);

    /**
     * Overrides default method to make public.
     */
    void cancelRename( int col );

    /**
     * Adds the child listview item representing the given UMLClassifierListItem.
     */
    void addClassifierListItem(UMLClassifierListItem *child, UMLListViewItem *childItem);

    /**
     * Deletes the child listview item representing the given UMLClassifierListItem.
     */
    void deleteChildItem(UMLClassifierListItem *child);

    /**
     * Overrides the default sorting to sort by item type.
     */
    virtual int compare(QListViewItem *other, int col, bool ascending) const;

    /**
     * Returns the number of children of the UMLListViewItem
     * containing this object
     */
    int childCount() const {
        return m_nChildren;
    }

    /**
     * Create a deep copy of this UMLListViewItem, but using the
     * given parent instead of the parent of this UMLListViewItem.
     * Return the new UMLListViewItem created.
     */
    UMLListViewItem* deepCopy(UMLListViewItem *newParent);

    /**
     * Find the UMLListViewItem that is related to the given UMLObject
     * in the tree rooted at the current UMLListViewItem.
     * Return a pointer to the item or NULL if not found.
     */
    UMLListViewItem* findUMLObject(const UMLObject *o);

    /**
     * Find the UMLListViewItem that represents the given UMLClassifierListItem
     * in the children of the current UMLListViewItem.  (Only makes sense if
     * the current UMLListViewItem represents a UMLClassifier.)
     * Return a pointer to the item or NULL if not found.
     */
    UMLListViewItem* findChildObject(UMLClassifierListItem *cli);

    /**
     * Find the UMLListViewItem of the given ID in the tree rooted at
     * the current UMLListViewItem.
     * Return a pointer to the item or NULL if not found.
     *
     * @param id                The ID to search for.
     * @return  The item with the given ID or NULL if not found.
     */
    UMLListViewItem * findItem(Uml::IDType id);

    /**
     * saves the listview item to a "listitem" tag
     */
    void saveToXMI( QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Loads a "listitem" tag, this is only used by the clipboard currently
     */
    bool loadFromXMI(QDomElement& qElement);

protected:
    /**
     * Initializes key variables of the class.
     */
    void init(UMLListView * parent = 0);

    /**
     * This function is called if the user presses Enter during in-place renaming
     * of the item in column col, reimplemented from QlistViewItem
     */
    void okRename( int col );

    /**
     * Auxiliary method for okRename().
     */
    void cancelRenameWithMsg();

    /**
     * This list view all the instance of this class are displayed on.
     */
    static UMLListView * s_pListView;

    /**
     * Flag used to set the state of creating.
     */
    bool m_bCreating;

    /**
     * Auxiliary map of child UMLLisViewItems keyed by UMLClassifierListItem.
     * Used by findChildObject() for efficiency instead of looping using
     * firstChild()/nextSibling() because the latter incur enforceItemVisible()
     * and thus expensive sorting.
     */
    typedef QMap<UMLClassifierListItem*, UMLListViewItem*> ChildObjectMap;

    Uml::ListView_Type m_Type;
    Uml::IDType m_nId;
    int m_nChildren;
    UMLObject * m_pObject;
    QString m_Label;
    ChildObjectMap m_comap;
};

#endif
