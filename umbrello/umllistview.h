 /*
  *  copyright (C) 2002-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLLISTVIEW_H
#define UMLLISTVIEW_H

#include <qdom.h>
#include <qpixmap.h>
#include <klistview.h>
#include "umlnamespace.h"
#include "umllistviewitemlist.h"

/**
 *	This is one of the main classes used in this program.
 *	Information is displayed here in a tree view.  No objects are created
 *	here.  A call to @ref UMLDoc make any additions/deletion or updates to
 *	objects.  This class will then wait for a signal before updating the tree view.
 *
 *	@short	Displays the list view for the program.
 *	@author Paul Hensgen	<phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
 
class QMouseEvent;
class QKeyEvent;
class IDChangeLog;
class ListPopupMenu;
class UMLClassifier;
class UMLDoc;
class UMLListViewItem;
class UMLView;
class UMLObject;

class UMLListView : public KListView {
	Q_OBJECT
 public:

	enum Icon_Type	{
		it_Home = 0,
		it_Folder_Cyan,
		it_Folder_Cyan_Open,
		it_Folder_Green,
		it_Folder_Green_Open,
		it_Folder_Grey,
		it_Folder_Grey_Open,
		it_Folder_Red,
		it_Folder_Red_Open,
		it_Folder_Violet,
		it_Folder_Violet_Open,
		it_Folder_Orange,
		it_Folder_Orange_Open,
		it_Diagram, //change to have different one for each type of diagram
		it_Class,
		it_Template,
		it_Package,
		it_Component,
		it_Node,
		it_Artifact,
		it_Interface,
		it_Datatype,
		it_Enum,
		it_Entity,
		it_Actor,
		it_UseCase,
		it_Public_Method,
		it_Private_Method,
		it_Protected_Method,
		it_Public_Attribute,
		it_Private_Attribute,
		it_Protected_Attribute,
		it_Diagram_Activity,
		it_Diagram_Class,
		it_Diagram_Collaboration,
		it_Diagram_Component,
		it_Diagram_Deployment,
		it_Diagram_EntityRelationship,
		it_Diagram_Sequence,
		it_Diagram_State,
		it_Diagram_Usecase
	};

	/**
	 * Constructs the tree view.
	 *
	 * @param parent	The parent to this.
	 * @param name		The internal name for this class.
	 */
	UMLListView(QWidget *parent,const char *name);

	/**
	 *	Standard deconstructor.
	 */
	~UMLListView();

	/**
	 * Sets the document his is associated with.  This is important as
	 * this is required as to setup the callbacks.
	 *
	 * @param d		The document to associate with this class.
	 */
	void setDocument(UMLDoc * d);

	/**
	 * Carries out initalisation of attributes in class.
	 */
	void init();

	/**
	 * Set the current view to the given view.
	 *
	 * @param v		The current view.
	 */
	void setView(UMLView* v);

	bool getSelectedItems(UMLListViewItemList &ItemList);

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
	 * Determine the parent ListViewItem given a ListView_Type.
	 * This parent is used for creating new UMLListViewItems.
	 *
	 * @param lvt	The ListView_Type for which to lookup the parent.
	 * @return	Pointer to the parent UMLListViewItem chosen.
	 */
	UMLListViewItem* determineParentItem(Uml::ListView_Type lvt) const;

	/**
	 *  Return the amount of items selected.
	 */
	int getSelectedCount();

	/**
	 * Returns the correct pixmap for the given type.
	 */
	QPixmap & getPixmap( Icon_Type type );

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
	void addNewItem( QListViewItem * parent, Uml::ListView_Type type );

	/**
	 * Find an UMLObject in the listview.
	 *
	 * @param p		Pointer to the object to find in the list view.
	 * @return	Pointer to the UMLObject found or NULL if not found.
	 */
	UMLListViewItem * findUMLObject(UMLObject *p) const;

	/**
	 * Searches through the tree for the item with the given ID.
	 *
	 * @param id		The ID to search for.
	 * @return	The item with the given ID or NULL if not found.
	 */
	UMLListViewItem * findItem(Uml::IDType id);

	/**
	 * Returns true if the listview type also has a widget representation in diagrams.
	 */
	static bool typeIsCanvasWidget(Uml::ListView_Type type);

	/**
	 * Returns true if the listview type is a logical, usecase or component folder.
	 */
	static bool typeIsFolder(Uml::ListView_Type type);

	/**
	 * Returns true if the listview type is a diagram.
	 */
	static bool typeIsDiagram(Uml::ListView_Type type);

	/**
	 * Returns true if the listview type is an attribute, operation, or template.
	 */
	static bool typeIsClassifierList(Uml::ListView_Type type);

	/**
	 * Creates a UMLObject out of the given list view item.
	 */
	void createUMLObject( UMLListViewItem * item, Uml::Object_Type type );

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
	 * Returns a unique name for a child umlobject.
	 */
	QString getUniqueChildUMLObjectName( UMLClassifier * parent, Uml::Object_Type type );

	/**
	 * Returns a unique name for an object.
	 */
	QString getUniqueUMLObjectName( Uml::Object_Type type );

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
	 * Converts an object type enum to the equivalent list view type
	 */
	static Uml::ListView_Type convert_OT_LVT(Uml::Object_Type ot);

	/**
	 * Moves an object given is unique ID and listview type to an
	 * other listview parent item.
	 * Also takes care of the corresponding move in the model.
	 */
	UMLListViewItem * moveObject(Uml::IDType srcId, Uml::ListView_Type srcType,
				     UMLListViewItem *newParent);

	void closeDatatypesFolder();

	UMLListViewItem *theRootView() { return rv; }
	UMLListViewItem *theLogicalView() { return lv; }

	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & element );

	bool loadChildrenFromXMI( UMLListViewItem * parent, QDomElement & element );

 protected:
	UMLListViewItem* rv;//root view
	UMLListViewItem* ucv;//use case view item
	UMLListViewItem* lv;//logical view item
	UMLListViewItem* componentView;//component view item
	UMLListViewItem* deploymentView;//deployment view item
	UMLListViewItem* entityRelationshipModel;//entity relationship view item
	UMLListViewItem* datatypeFolder;
	UMLListViewItem* diagramFolder;
	ListPopupMenu * m_pMenu;
	QString oldText, message;
	UMLDoc *m_doc;
	bool m_bStartedCut, m_bStartedCopy, m_bIgnoreCancelRename;

	/**
	 * Used when creating an attribute or an operation to stop it adding a second listViewItem
	 */
	bool m_bCreatingChildObject;

	struct Pixmaps {
		QPixmap Home;
		QPixmap Folder_Cyan;
		QPixmap Folder_Cyan_Open;
		QPixmap Folder_Green;
		QPixmap Folder_Green_Open;
		QPixmap Folder_Grey;
		QPixmap Folder_Grey_Open;
		QPixmap Folder_Orange;
		QPixmap Folder_Orange_Open;
		QPixmap Folder_Red;
		QPixmap Folder_Red_Open;
		QPixmap Folder_Violet;
		QPixmap Folder_Violet_Open;
		QPixmap Diagram; //change to have different one for each type of diagram
		QPixmap Class;
		QPixmap Template;
		QPixmap Package;
		QPixmap Component;
		QPixmap Node;
		QPixmap Artifact;
		QPixmap Interface;
		QPixmap Datatype;
		QPixmap Enum;
		QPixmap Entity;
		QPixmap Actor;
		QPixmap UseCase;
		QPixmap Public_Method;
		QPixmap Private_Method;
		QPixmap Protected_Method;
		QPixmap Public_Attribute;
		QPixmap Private_Attribute;
		QPixmap Protected_Attribute;
		QPixmap Diagram_Activity;
		QPixmap Diagram_Class;
		QPixmap Diagram_Collaboration;
		QPixmap Diagram_Component;
		QPixmap Diagram_Deployment;
		QPixmap Diagram_EntityRelationship;
		QPixmap Diagram_Sequence;
		QPixmap Diagram_State;
		QPixmap Diagram_Usecase;
	}
	m_Pixmaps;
	void contentsMouseReleaseEvent(QMouseEvent * me);
	void contentsMousePressEvent(QMouseEvent *me);
	void contentsMouseDoubleClickEvent(QMouseEvent * me);
	void focusOutEvent ( QFocusEvent * fe);
	bool goodName(QListViewItem *i);
	QDragObject* dragObject();
	void startDrag();
	void tidyChangeItem();
	bool acceptDrag (QDropEvent* event) const;
	void keyPressEvent(QKeyEvent *);

	/**
	 * This methods looks for a object in a folder an its subfolders recursive.
	 * @param item The folder entry of the list view.
	 * @param o The object to be find in the folder.
	 *
	 * @return The object if found else a NULL pointer.
	 */
	UMLListViewItem * findUMLObjectInFolder(UMLListViewItem *item, UMLObject *o);

	/**
	 * Searches through the tree for the item which represents the diagram given
	 * @param the diagram to search for
	 * @return the item which represents the diagram
	 */
	UMLListViewItem * findView(UMLView *v);

	/**
	 * Utility: Converts a diagram type enum to the equivalent list view type
	 */
	static Uml::ListView_Type convert_DT_LVT(Uml::Diagram_Type dt);

	/**
	 * Converts a list view type enum to the equivalent object type.
	 *
	 * @param lvt		The ListView_Type to convert.
	 * @return	The converted Object_Type if the listview type
	 *		has a Object_Type representation, else 0.
	 */
	static Uml::Object_Type convert_LVT_OT(Uml::ListView_Type lvt);

	/**
	 * 	Loads the pixmaps to use in the list items.
	 */
	void loadPixmaps();

	/**
	 * Deletes all child-items of @p parent.
	 */
	void deleteChildrenOf( QListViewItem *parent );

public slots:

	/**
	 * Creates a new item to represent a new diagram
	 * @param id the id of the new diagram
	 */
	void slotDiagramCreated(Uml::IDType id);

	/**
	 * renames a diagram in the list view
	 * @param the id of the renamed diagram
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
	void childObjectAdded(UMLObject* obj);

	/**
	 * Adds a new operation, attribute or template item to a classifier, identical to
	 * childObjectAdded(UMLObject* obj) but with an explicit parent.  Used by ClassWizard.
	 * @param obj the child object
	 * @param parent the parent object 
	 */
	void childObjectAdded(UMLObject* obj, UMLObject* parent);

	/**
	 * disconnects signals and removes the list view item
	 * @param object the object about to be removed
	 */
	void slotObjectRemoved(UMLObject* object);

	/**
	 * deletes the list view item
	 * @param obj the object to remove
	 */
	void childObjectRemoved(UMLObject* obj);

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
	 * 	Connects to the signal that @ref UMLApp emits when a
	 * 	cut operation is successful.
	 */
	void slotCutSuccessful();

	/**
	 *     When a user requests a new item we wait for the signal
	 *     to then verify the item.  The class only connects to the
	 *     rename signal while needed.
	 */
	bool slotItemRenamed(QListViewItem* item , int col);

 signals:

	/**
	 * change the current view
	 */
	void diagramSelected(Uml::IDType);

 private:
	/**
	 * Searches the tree for a diagram (view).
	 * Used by findView().
	 */
	UMLListViewItem* recursiveSearchForView(UMLListViewItem* folder,
						Uml::ListView_Type type, Uml::IDType id);

};

#endif
