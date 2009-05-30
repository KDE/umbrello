/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLLISTVIEW_H
#define UMLLISTVIEW_H

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include <k3listview.h>

#include "umlnamespace.h"
#include "umllistviewitemlist.h"
#include "icon_utils.h"

class QEvent;
class QMouseEvent;
class QFocusEvent;
class QKeyEvent;
class QDropEvent;
class IDChangeLog;
class ListPopupMenu;
class UMLClassifier;
class UMLDoc;
class UMLListViewItem;
class UMLView;
class UMLObject;
class UMLClassifierListItem;
class UMLDragData;

/**
 * This is one of the main classes used in this program.
 * Information is displayed here in a tree view. No objects are created
 * here. A call to @ref UMLDoc make any additions/deletion or updates to
 * objects. This class will then wait for a signal before updating the tree view.
 *
 * @short  Displays the list view for the program.
 * @author Paul Hensgen    <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLListView : public K3ListView
{
    Q_OBJECT
public:

    UMLListView(QWidget *parent,const char *name);
    ~UMLListView();

    void setDocument(UMLDoc * doc);

    void init();

    void setView(UMLView* view);

    int getSelectedItems(UMLListViewItemList &ItemList);
    int getSelectedItemsRoot(UMLListViewItemList &ItemList);

    UMLListViewItem* createDiagramItem(UMLView *view);

    UMLListViewItem* createItem(UMLListViewItem& Data, IDChangeLog& IDChanges,
                                UMLListViewItem* parent = 0);

    UMLListViewItem* findFolderForDiagram(Uml::Diagram_Type dt);

    UMLListViewItem* determineParentItem(UMLObject* object) const;
    UMLListViewItem* determineParentItem(Uml::ListView_Type lvt) const;

    static bool mayHaveChildItems(Uml::Object_Type type);

    int getSelectedCount();

    UMLDoc * getDocument() const;

    void addNewItem( UMLListViewItem * parent, Uml::ListView_Type type );

    UMLListViewItem * findUMLObject(const UMLObject *p) const;
    UMLListViewItem * findView(UMLView *v);
    UMLListViewItem * findItem(Uml::IDType id);

    UMLListViewItem *rootView(Uml::ListView_Type type);

    void changeIconOf(UMLObject *o, Icon_Utils::Icon_Type to);

    UMLObject *createUMLObject( UMLListViewItem * item, Uml::Object_Type type );
    bool createChildUMLObject( UMLListViewItem * item, Uml::Object_Type type );
    UMLView* createDiagram( UMLListViewItem * item, Uml::Diagram_Type type );

    QString getUniqueDiagramName( Uml::Diagram_Type type );

    bool isUnique( UMLListViewItem * item, const QString &name );

    void cancelRename( Q3ListViewItem * item );

    void setStartedCut(bool startedCut);
    void setStartedCopy(bool startedCopy);
    bool startedCopy() const;

    UMLListViewItem * moveObject(Uml::IDType srcId, Uml::ListView_Type srcType,
                                 UMLListViewItem *newParent);

    bool itemRenamed(Q3ListViewItem* item , int col);

    void closeDatatypesFolder();

    UMLListViewItem *theRootView() { return m_rv; }
    UMLListViewItem *theLogicalView() { return m_lv[Uml::mt_Logical]; }
    UMLListViewItem *theUseCaseView() { return m_lv[Uml::mt_UseCase]; }
    UMLListViewItem *theComponentView() { return m_lv[Uml::mt_Component]; }
    UMLListViewItem *theDeploymentView() { return m_lv[Uml::mt_Deployment]; }
    UMLListViewItem *theDatatypeFolder() { return m_datatypeFolder; }

    Uml::ListView_Type rootViewType(UMLListViewItem *item);

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement);
    bool loadFromXMI( QDomElement & element );
    bool loadChildrenFromXMI( UMLListViewItem * parent, QDomElement & element );

protected:

    UMLListViewItem* m_rv;                     // root view (home)
    UMLListViewItem* m_lv[Uml::N_MODELTYPES];  // predefined list view roots
    UMLListViewItem* m_datatypeFolder;
    ListPopupMenu*   m_pMenu;
    UMLDoc*          m_doc;
    bool m_bStartedCut, m_bStartedCopy, m_bIgnoreCancelRename;

    /**
     * Used when creating an attribute or an operation to stop it adding a second listViewItem.
     */
    bool m_bCreatingChildObject;

    bool eventFilter(QObject *o, QEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent * me);
    void contentsMousePressEvent(QMouseEvent *me);
    void contentsMouseMoveEvent(QMouseEvent* me);
    void contentsMouseDoubleClickEvent(QMouseEvent * me);
    void focusOutEvent ( QFocusEvent * fe);

    UMLDragData* getDragData();

    bool acceptDrag (QDropEvent* event) const;
    void keyPressEvent(QKeyEvent *);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

    UMLListViewItem * findUMLObjectInFolder(UMLListViewItem *folder, UMLObject *obj);

    static bool isExpandable(Uml::ListView_Type lvt);

    void deleteChildrenOf( Q3ListViewItem *parent );

    bool deleteItem( UMLListViewItem *temp );

    void childObjectAdded(UMLClassifierListItem* child, UMLClassifier* parent);

    void addAtContainer(UMLListViewItem *item, UMLListViewItem *parent);

public slots:

    void slotDiagramCreated(Uml::IDType id);
    void slotDiagramRenamed(Uml::IDType id);
    void slotDiagramRemoved(Uml::IDType id);
    void slotObjectCreated(UMLObject* object);
    void slotObjectRemoved(UMLObject* object);

    void connectNewObjectsSlots(UMLObject* object);

    void childObjectAdded(UMLClassifierListItem* obj);
    void childObjectRemoved(UMLClassifierListItem* obj);

    void slotObjectChanged();

    void popupMenuSel(QAction* action);

    void slotDropped(QDropEvent* de, Q3ListViewItem* parent, Q3ListViewItem* item);

    void slotExpanded(Q3ListViewItem* item);
    void slotCollapsed(Q3ListViewItem* item);

    void expandAll(Q3ListViewItem *item);
    void collapseAll(Q3ListViewItem *item);

    void slotCutSuccessful();

private:

    UMLListViewItem* recursiveSearchForView(UMLListViewItem* folder,
                                            Uml::ListView_Type type, Uml::IDType id);

    void setBackgroundColor(const QColor & color);

    QPoint m_dragStartPosition;

};

#endif
