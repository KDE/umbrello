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

#include "umllistviewitemlist.h"
#include <klistview.h>
#include <qpixmap.h>

/**
 *	This is one of the main classes used in this program.
 *	Information is displayed here in a tree view.  No objects are created
 *	here.  A call to @ref UMLDoc make any additions/deletion or updates to
 *	objects.  This class will then wait for a signal before updating the tree view.
 *
 *	@short	Displays the list view for the program.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version 1.0
 */
 
namespace Umbrello
{
	class Diagram;
	class DiagramView;
} 
class IDChangeLog;
class ListPopupMenu;
class UMLClassifier;
class UMLDoc;
class UMLListViewItem;
class UMLView;

using Umbrello::Diagram;
using Umbrello::DiagramView;


class UMLListView : public KListView {
	Q_OBJECT
		friend class UMLClipboard;
 public:

	enum Icon_Type	{
		it_Home = 0,
		it_Folder_Green,
		it_Folder_Green_Open,
		it_Folder_Grey,
		it_Folder_Grey_Open,
		it_Folder_Red,
		it_Folder_Red_Open,
		it_Folder_Violet,
		it_Folder_Violet_Open,
		it_Diagram, //change to have different one for each type of diagram
		it_Class,
		it_Template,
		it_Package,
		it_Component,
		it_Node,
		it_Artifact,
		it_Interface,
		it_Actor,
		it_UseCase,
		it_Public_Method,
		it_Private_Method,
		it_Protected_Method,
		it_Public_Attribute,
		it_Private_Attribute,
		it_Protected_Attribute
	};

	/**
	 *	Constructs the tree view.
	 *
	 *	@param	parent	The parent to this.
	 *	@param	name	The internal name for this class.
	 */
	UMLListView(QWidget *parent,const char *name);

	/**
	 *	Standard deconstructor.
	 */
	~UMLListView();

	/**
	 *	Sets the document his is associated with.  This is important as
	 *	this is required as to setup the callbacks.
	 *
	 *	@param	d	The document to associate with this class.
	 */
	void setDocument(UMLDoc * d);

	/**
	 * Carries out initalisation of attributes in class.
	 */
	void init();

	/**
	 *	Set the current view to the given view.
	 *
	 *	@param	v	The current view.
	 */
	void setView(UMLView * v);

	/**
	 * Use to save or load this classes information
	 *
	 *	@param	s	Pointer to the datastream (file) to save/load from.
	 *	@param	archive	If true will save the classes information, else will
	 *                      load the information.
	 *
	 *	@return	Returns the status of the operation.
	 */
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	 * used by serialise()
	 */
	bool readChilds( UMLListViewItem* parent, QDataStream *s);

	long getClipSizeOf();

	void setLoading(bool _state) {
		loading = _state;
	}

	bool getSelectedItems(UMLListViewItemList &ItemList);

	/**
	 * Returns the amount of bytes needed to serialize Item to the clipboard
	 */
	long getClipSizeOfItemList(UMLListViewItemList* Items);

	/**
	 * Serialize Item to a QDataStream that will be put in the clipboard
	 */
	bool clipSerializeItemList(UMLListViewItemList* Items, QDataStream *s, const bool toClip);

	/**
	 * Creates a new UMLListViewItem from a UMLListViewItemData, if
	 * parent is null the ListView Decides who is going to be the
	 * parent
	 */
	UMLListViewItem* createItem(UMLListViewItemData& Data, IDChangeLog& IDChanges,
				    UMLListViewItem* parent = 0);

	/**
	 * 	Return the amount of items selected.
	 */
	int getSelectedCount();

	/**
	 *   Returns the correct pixmap for the given type.
	 */
	QPixmap & getPixmap( Icon_Type type );

	/**
	 *     Returns the document pointer.  Called by the UMLListViewItem class.
	 */
	UMLDoc * getDocument() {
		return m_doc;
	}

	/**
	 *     Adds a new item to the tree of the given type under the given parent.
	 *     Method will take care of signalling anyone needed on creation of new item.
	 *     e.g. UMLDoc if an UMLObject is created.
	 */
	void addNewItem( QListViewItem * parent, Uml::ListView_Type type );

	/**
	 *   Creates a UMLObject out of the given list view item.
	 */
	void createUMLObject( UMLListViewItem * item, Uml::UMLObject_Type type );

	/**
	 *   Creates a child UMLObject out of the given list view item.
	 */
	void createChildUMLObject( UMLListViewItem * item, Uml::UMLObject_Type type );

	/**
	 *   Creates a diagram out of the given list view item.
	 */
	void createDiagram( UMLListViewItem * item, Uml::Diagram_Type type );

	/**
	 *     Returns a unique name for a diagram.
	 */
	QString getUniqueDiagramName( Uml::Diagram_Type type );

	/**
	 *     Returns a unique name for a child umlobject.
	 */
	QString getUniqueChildUMLObjectName( UMLClassifier * parent, Uml::UMLObject_Type type );

	/**
	 *     Returns a unique name for an object.
	 */
	QString getUniqueUMLObjectName( Uml::UMLObject_Type type );

	/**
	 *     Returns if the given name is unique for the given items type.
	 */
	bool isUnique( UMLListViewItem * item, QString name );

	/**
	 *   Cancel rename event has occurred for the given item.
	 */
	void  cancelRename( QListViewItem * item );

	/**
	 * Set the variable m_bStartedCut
	 * to indicate that selection should be deleted 
	 * in slotCutSuccessful()
	 */
	void setStartedCut(bool startedCut);

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & element );

	bool loadChildrenFromXMI( UMLListViewItem * parent, QDomElement & element );

	

 protected:
	UMLListViewItem* rv;//root view
	UMLListViewItem* ucv;//use case view item
	UMLListViewItem* lv;//logical view item
	UMLListViewItem* componentView;//component view item
	UMLListViewItem* deploymentView;//deployment view item
	UMLListViewItem* diagramFolder;
	ListPopupMenu * menu;
	QString oldText, message;
	UMLDoc *m_doc;
	bool loading, m_bStartedCut, m_bIgnoreCancelRename;

	struct Pixmaps {
		QPixmap Home;
		QPixmap Folder_Green;
		QPixmap Folder_Green_Open;
		QPixmap Folder_Grey;
		QPixmap Folder_Grey_Open;
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
		QPixmap Actor;
		QPixmap UseCase;
		QPixmap Public_Method;
		QPixmap Private_Method;
		QPixmap Protected_Method;
		QPixmap Public_Attribute;
		QPixmap Private_Attribute;
		QPixmap Protected_Attribute;
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

	/**
	 *
	 * @param c The object to be find in the list view
	 */
	UMLListViewItem * findUMLObject(UMLObject *p) const;

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
	 * Searches through the tree for the item with the given id
	 * @param the id to search for
	 * @return the item with the given id
	 */
	UMLListViewItem * findItem(int id);

	/**
	 * Converts a diagram type enum to the equivalent list view type
	 */
	Uml::ListView_Type convert_DT_LVT(Uml::Diagram_Type dt);

	/**
	 * Converts an object type enum to the equivalent list view type
	 */
	Uml::ListView_Type convert_OT_LVT(Uml::UMLObject_Type ot);

	/**
	 * 	Loads the pixmaps to use in the list items.
	 */
	void loadPixmaps();

	/**
	 * returns true if the listview type is actor, usecase, class, package or interface
	 */
	static bool typeIsCanvasWidget(ListView_Type type);

	/**
	 * returns true if the listview type is logical, usecase or component folder
	 */
	static bool typeIsFolder(ListView_Type type);

	/**
	 * returns true if the listview type is a diagram
	 */
	static bool typeIsDiagram(ListView_Type type);

	/**
	 * returns true if the listview type is an attribute, operation or template
	 */
	static bool typeIsClassifierList(ListView_Type type);

	/**
	 * Deletes all child-items of @p parent.
	 */
	static void deleteChildrenOf( QListViewItem *parent );

	public slots:

		/**
		 * Creates a new item to represent a new diagram
		 * @param id the id of the new diagram
		 */
		void slotDiagramCreated(int id);

	/**
	 * Creates a new item to represent a new diagram
	 * @param d pointer to the new diagram
	 */
	void diagramCreated(Umbrello::Diagram* d);

	/**
	 * renames a diagram in the list view
	 * @param the id of the renamed diagram
	 */
	void slotDiagramRenamed(int id);

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
	 * @param obj the parent object 
	 */
	void childObjectAdded(UMLObject* obj);

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
	void slotDiagramRemoved(int id);

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
	 *   Reset the status bar
	 */
	void sigResetStatusbarProgress();

	/**
	 * change the current view
	 */
	void diagramSelected(int);

	/*
	 *
	 * Set the total range of the progressbar
	 *
	 * @param totalSteps Total range of the progressbar (0..totalSteps)
	 */
	void sigSetStatusbarProgressSteps(int totalSteps);

	/*
	 *
	 * Set the total range of the progressbar
	 *
	 * @param totalSteps Total range of the progressbar (0..totalStep
	 */
	void sigSetStatusbarProgress(int stepPosition);

};

#endif
