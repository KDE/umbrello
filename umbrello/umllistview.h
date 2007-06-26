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

#ifndef UMLLISTVIEW_H
#define UMLLISTVIEW_H

#include <qdom.h>
#include <qpixmap.h>
#include <klistview.h>
#include "umlnamespace.h"
#include "umllistviewitemlist.h"

/**
 * This is one of the main classes used in this program.
 * Information is displayed here in a tree view.  No objects are created
 * here.  A call to @ref UMLDoc make any additions/deletion or updates to
 * objects.  This class will then wait for a signal before updating the tree view.
 *
 * @short  Displays the list view for the program.
 * @author Paul Hensgen    <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class QMouseEvent;
class QContextMenuEvent;
class QKeyEvent;
class IDChangeLog;
class ListPopupMenu;
class UMLClassifier;
class UMLDoc;
class UMLListViewItem;
class UMLView;
class UMLObject;
class UMLClassifierListItem;

class UMLListView : public KListView {
    Q_OBJECT
public:

    /**
     * Constructs the tree view.
     *
     * @param parent    The parent to this.
     * @param name              The internal name for this class.
     */
    UMLListView(QWidget *parent,const char *name);

    /**
     *  Standard deconstructor.
     */
    ~UMLListView();

    /**
     * Sets the document his is associated with.  This is important as
     * this is required as to setup the callbacks.
     *
     * @param d         The document to associate with this class.
     */
    void setDocument(UMLDoc * d);

    /**
     * Carries out initalisation of attributes in class.
     */
    void init();

    /**
     * Set the current view to the given view.
     *
     * @param v         The current view.
     */
    void setView(UMLView* v);

    /**
     * Get selected items.
     *
     * @param ItemList  List of UMLListViewItems returned.
     * @return          The number of selected items.
     */
    int getSelectedItems(UMLListViewItemList &ItemList);

    /**
     * Get selected items, but only root elements selected (without children).
     *
     * @param ItemList  List of UMLListViewItems returned.
     * @return          The number of selected items.
     */
    int getSelectedItemsRoot(UMLListViewItemList &ItemList);

    /**
     * Create a listview item for an existing diagram.
     *
     * @param v         The existing diagram.
     */
    UMLListViewItem* createDiagramItem(UMLView *v);

    /**
     * CHECK - This is perhaps redundant since the
     *         UMLListViewItemData => UMLListViewItem merge.
     * Creates a new UMLListViewItem from a UMLListViewItem, if
     * parent is null the ListView Decides who is going to be the
     * parent
     */
    UMLListViewItem* createItem(UMLListViewItem& Data, IDChangeLog& IDChanges,
                                UMLListViewItem* parent = 0);

    /**
     * Find the parent folder for a diagram.
     * If the currently selected item in the list view is a folder
     * then that folder is returned as the parent.
     *
     * @param dt   The Diagram_Type of the diagram.
     *             The type will only be used if there is no currently
     *             selected item, or if the current item is not a folder.
     *             In that case the root folder which is suitable for the
     *             Diagram_Type is returned.
     * @return  Pointer to the parent UMLListViewItem for the diagram.
     */
    UMLListViewItem *findFolderForDiagram(Uml::Diagram_Type dt);

    /**
     * Determine the parent ListViewItem given an UMLObject.
     *
     * @param object   Pointer to the UMLObject for which to look up the parent.
     * @return    Pointer to the parent UMLListViewItem chosen.
     *            Returns NULL on error (no parent could be determined.)
     */
    UMLListViewItem* determineParentItem(UMLObject* object) const;

    /**
     * Determine the parent ListViewItem given a ListView_Type.
     * This parent is used for creating new UMLListViewItems.
     *
     * @param lvt       The ListView_Type for which to lookup the parent.
     * @return  Pointer to the parent UMLListViewItem chosen.
     */
    UMLListViewItem* determineParentItem(Uml::ListView_Type lvt) const;

    /**
     * Return true if the given Object_Type permits child items.
     * A "child item" is anything that qualifies as a UMLClassifierListItem,
     * e.g. operations and attributes of classifiers.
     */
    static bool mayHaveChildItems(Uml::Object_Type type);

    /**
     *  Return the amount of items selected.
     */
    int getSelectedCount();

    /**
     * Returns the correct pixmap for the given type.
     */
    QPixmap & getPixmap( Uml::Icon_Type type );

    /**
     * Returns the document pointer.  Called by the UMLListViewItem class.
     */
    UMLDoc * getDocument() {
        return m_doc;
    }

    /**
     * Adds a new item to the tree of the given type under the given parent.
     * Method will take care of signalling anyone needed on creation of new item.
     * e.g. UMLDoc if an UMLObject is created.
     */
    void addNewItem(UMLListViewItem * parent, Uml::ListView_Type type);

    /**
     * Find an UMLObject in the listview.
     *
     * @param p         Pointer to the object to find in the list view.
     * @return  Pointer to the UMLObject found or NULL if not found.
     */
    UMLListViewItem * findUMLObject(const UMLObject *p) const;

    /**
     * Searches through the tree for the item which represents the diagram given
     * @param v  the diagram to search for
     * @return the item which represents the diagram
     */
    UMLListViewItem * findView(UMLView *v);

    /**
     * Searches through the tree for the item with the given ID.
     *
     * @param id                The ID to search for.
     * @return  The item with the given ID or NULL if not found.
     */
    UMLListViewItem * findItem(Uml::IDType id);

    /**
     * Returns the corresponding view if the listview type is one of the root views,
     * Root/Logical/UseCase/Component/Deployment/EntityRelation View.
     */
    UMLListViewItem *rootView(Uml::ListView_Type type);

    /**
     * Changes the icon for the given UMLObject to the given icon.
     */
    void changeIconOf(UMLObject *o, Uml::Icon_Type to);

    /**
     * Creates a UMLObject out of the given list view item.
     */
    UMLObject *createUMLObject( UMLListViewItem * item, Uml::Object_Type type );

    /**
     * Creates a child UMLObject out of the given list view item.
     */
    bool createChildUMLObject( UMLListViewItem * item, Uml::Object_Type type );

    /**
     * Creates a diagram out of the given list view item.
     */
    void createDiagram( UMLListViewItem * item, Uml::Diagram_Type type );

    /**
     * Returns a unique name for a diagram.
     */
    QString getUniqueDiagramName( Uml::Diagram_Type type );

    /**
     * Returns if the given name is unique for the given items type.
     */
    bool isUnique( UMLListViewItem * item, const QString &name );

    /**
     * Cancel rename event has occurred for the given item.
     */
    void  cancelRename( QListViewItem * item );

    /**
     * Set the variable m_bStartedCut
     * to indicate that selection should be deleted
     * in slotCutSuccessful()
     */
    void setStartedCut(bool startedCut);

    /**
     * Set the variable m_bStartedCopy.
     * NB: While m_bStartedCut is reset as soon as the Cut operation is done,
     *     the variable m_bStartedCopy is reset much later - upon pasting.
     */
    void setStartedCopy(bool startedCopy);

    /**
     * Return the variable m_bStartedCopy.
     */
    bool startedCopy() const;

    /**
     * Moves an object given is unique ID and listview type to an
     * other listview parent item.
     * Also takes care of the corresponding move in the model.
     */
    UMLListViewItem * moveObject(Uml::IDType srcId, Uml::ListView_Type srcType,
                                 UMLListViewItem *newParent);

    /**
     * Called for informing the list view that an item was renamed.
     */
    bool itemRenamed(QListViewItem* item , int col);

    void closeDatatypesFolder();

    UMLListViewItem *theRootView() { return m_rv; }
    UMLListViewItem *theLogicalView() { return m_lv[Uml::mt_Logical]; }
    UMLListViewItem *theUseCaseView() { return m_lv[Uml::mt_UseCase]; }
    UMLListViewItem *theComponentView() { return m_lv[Uml::mt_Component]; }
    UMLListViewItem *theDeploymentView() { return m_lv[Uml::mt_Deployment]; }
    UMLListViewItem *theDatatypeFolder() { return m_datatypeFolder; }

    /**
     * Determines the root listview type of the given UMLListViewItem.
     * Starts at the given item, compares it against each of the
     * predefined root views (Root, Logical, UseCase, Component,
     * Deployment, EntityRelationship.) Returns the ListView_Type
     * of the matching root view; if no match then continues the
     * search using the item's parent, then grandparent, and so forth.
     * Returns Uml::lvt_Unknown if no match at all is found.
     */
    Uml::ListView_Type rootViewType(UMLListViewItem *item);

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement);

    bool loadFromXMI( QDomElement & element );

    bool loadChildrenFromXMI( UMLListViewItem * parent, QDomElement & element );

protected:
    UMLListViewItem* m_rv;    // root view (home)
    UMLListViewItem* m_lv[Uml::N_MODELTYPES];    // predefined list view roots
    UMLListViewItem* m_datatypeFolder;
    ListPopupMenu * m_pMenu;
    QString oldText, message;
    UMLDoc *m_doc;
    bool m_bStartedCut, m_bStartedCopy, m_bIgnoreCancelRename;

    /**
     * Used when creating an attribute or an operation to stop it adding a second listViewItem
     */
    bool m_bCreatingChildObject;

    QPixmap m_Pixmaps[Uml::N_ICONTYPES];

    bool eventFilter(QObject *o, QEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent * me);
    void contentsMousePressEvent(QMouseEvent *me);
    void contentsMouseDoubleClickEvent(QMouseEvent * me);
    void focusOutEvent ( QFocusEvent * fe);
    QDragObject* dragObject();
    void startDrag();
    bool acceptDrag (QDropEvent* event) const;
    void keyPressEvent(QKeyEvent *);

    /**
     * This methods looks for a object in a folder an its subfolders recursive.
     * @param item The folder entry of the list view.
     * @param o The object to be found in the folder.
     *
     * @return The object if found else a NULL pointer.
     */
    UMLListViewItem * findUMLObjectInFolder(UMLListViewItem *item, UMLObject *o);

    /**
     * Return true if the given list view type can be expanded/collapsed.
     */
    static bool isExpandable(Uml::ListView_Type lvt);

    /**
     *  Loads the pixmaps to use in the list items.
     */
    void loadPixmaps();

    /**
     * Deletes all child-items of @p parent.
     */
    void deleteChildrenOf( QListViewItem *parent );

    /**
     * Delete a listview item.
     * @param temp a non-null UMLListViewItem, for example:
                             (UMLListViewItem*)currentItem()
     * @return     true if correctly deleted
     */
    bool deleteItem( UMLListViewItem *temp );

    /**
     * Adds a new operation, attribute or template item to a classifier, identical to
     * childObjectAdded(obj) but with an explicit parent.
     * @param child the child object
     * @param parent the parent object
     */
    void childObjectAdded(UMLClassifierListItem* child, UMLClassifier* parent);

    /**
     * Auxiliary method for moveObject(): Adds the model object at the proper
     * new container (package if nested, UMLDoc if at global level), and
     * updates the containment relationships in the model.
     */
    void addAtContainer(UMLListViewItem *item, UMLListViewItem *parent);

public slots:

    /**
     * Creates a new item to represent a new diagram
     * @param id the id of the new diagram
     */
    void slotDiagramCreated(Uml::IDType id);

    /**
     * renames a diagram in the list view
     * @param id    the id of the renamed diagram
     */
    void slotDiagramRenamed(Uml::IDType id);

    /**
     * Creates a new list view item and connects the appropriate signals/slots
     * @param object the newly created object
     */
    void slotObjectCreated(UMLObject* object);

    /**
     * connect some signals into slots in the list view for newly created UMLObjects
     */
    void connectNewObjectsSlots(UMLObject* object);

    /**
     * Adds a new operation, attribute or template item to a classifier
     * @param obj the child object
     */
    void childObjectAdded(UMLClassifierListItem* obj);

    /**
     * disconnects signals and removes the list view item
     * @param object the object about to be removed
     */
    void slotObjectRemoved(UMLObject* object);

    /**
     * deletes the list view item
     * @param obj the object to remove
     */
    void childObjectRemoved(UMLClassifierListItem* obj);

    /**
     * calls updateObject() on the item representing the sending object
     * no parameters, uses sender() to work out which object called the slot
     */
    void slotObjectChanged();

    /**
     * removes the item representing a diagram
     * @param id the id of the diagram
     */
    void slotDiagramRemoved(Uml::IDType id);

    /**
     * Called when a right mouse button menu has an item selected
     */
    void popupMenuSel(int sel);

    /**
     * Something has been dragged and dropped onto the list view
     */
    void slotDropped(QDropEvent* de, QListViewItem* parent, QListViewItem* item);

    /**
     * calls updateFolder() on the item to update the icon to open
     */
    void slotExpanded(QListViewItem* item);

    /**
     * calls updateFolder() on the item to update the icon to closed
     */
    void slotCollapsed(QListViewItem* item);

    /**
     * Open all items in the list view
     */
    void expandAll(QListViewItem *item);

    /**
     * Close all items in the list view
     */
    void collapseAll(QListViewItem *item);

    /**
     *  Connects to the signal that @ref UMLApp emits when a
     *  cut operation is successful.
     */
    void slotCutSuccessful();

private:
    /**
     * Searches the tree for a diagram (view).
     * Used by findView().
     */
    UMLListViewItem* recursiveSearchForView(UMLListViewItem* folder,
                                            Uml::ListView_Type type, Uml::IDType id);

};

#endif
