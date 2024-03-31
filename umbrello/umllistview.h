/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLLISTVIEW_H
#define UMLLISTVIEW_H

#include "basictypes.h"
#include "umllistviewitem.h"
#include "umllistviewitemlist.h"
#include "icon_utils.h"
#include "umlobject.h"

#include <QDomDocument>
#include <QDomElement>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QXmlStreamWriter>

class QEvent;
class QMouseEvent;
class QFocusEvent;
class QKeyEvent;
class QDropEvent;
class UMLClassifier;
class UMLDoc;
class UMLView;
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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLListView : public QTreeWidget
{
    Q_OBJECT
public:
    explicit UMLListView(QWidget *parent = nullptr);
    ~UMLListView();

    void setDocument(UMLDoc * doc);
    UMLDoc * document() const;

    void init();
    void clean();

    void setView(UMLView* view);

    void setTitle(int column, const QString &text);

    UMLListViewItemList selectedItems() const;
    UMLListViewItemList selectedItemsRoot() const;
    int selectedItemsCount() const;

    UMLListViewItem* createDiagramItem(UMLView *view);

    bool createItem(UMLListViewItem *item);

    UMLListViewItem* findFolderForDiagram(Uml::DiagramType::Enum dt) const;

    UMLListViewItem* determineParentItem(UMLObject* object) const;
    UMLListViewItem* determineParentItem(UMLListViewItem::ListViewType lvt) const;

    static bool mayHaveChildItems(UMLObject::ObjectType type);

    void addNewItem(UMLListViewItem * parent, UMLListViewItem::ListViewType type);

    UMLListViewItem * findUMLObject(const UMLObject *p) const;
    UMLListViewItem * findView(UMLView *v);
    UMLListViewItem * findItem(Uml::ID::Type id);

    UMLListViewItem *rootView(UMLListViewItem::ListViewType type);

    void changeIconOf(UMLObject *o, Icon_Utils::IconType to);

    bool isUnique(UMLListViewItem * item, const QString &name) const;

    void setStartedCut(bool startedCut);
    void setStartedCopy(bool startedCopy);
    bool startedCopy() const;

    UMLListViewItem * moveObject(Uml::ID::Type srcId, UMLListViewItem::ListViewType srcType,
                                 UMLListViewItem *newParent);

    void closeDatatypesFolder();

    UMLListViewItem *theRootView() { return m_rv; }
    UMLListViewItem *theLogicalView() { return m_lv[Uml::ModelType::Logical]; }
    UMLListViewItem *theUseCaseView() { return m_lv[Uml::ModelType::UseCase]; }
    UMLListViewItem *theComponentView() { return m_lv[Uml::ModelType::Component]; }
    UMLListViewItem *theDeploymentView() { return m_lv[Uml::ModelType::Deployment]; }
    UMLListViewItem *theDatatypeFolder() { return m_datatypeFolder; }

    UMLListViewItem::ListViewType rootViewType(UMLListViewItem *item);

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement & element);
    bool loadChildrenFromXMI(UMLListViewItem * parent, QDomElement & element);

    friend QDebug operator<<(QDebug out, const UMLListView& view);

protected:
    bool event(QEvent *e);
    void mouseReleaseEvent(QMouseEvent *me);
    void mousePressEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent *me);
    void mouseDoubleClickEvent(QMouseEvent *me);
    void focusOutEvent (QFocusEvent *fe);
    void contextMenuEvent(QContextMenuEvent *event);
    UMLDragData* getDragData();

    bool acceptDrag(QDropEvent* event) const;
    void keyPressEvent(QKeyEvent* ke);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);
    void commitData(QWidget *editor);

    UMLListViewItem * findUMLObjectInFolder(UMLListViewItem *folder, UMLObject *obj);

    static bool isExpandable(UMLListViewItem::ListViewType lvt);

    void deleteChildrenOf(UMLListViewItem *parent);

    bool deleteItem(UMLListViewItem *temp);

    void childObjectAdded(UMLClassifierListItem* child, UMLClassifier* parent);

    void addAtContainer(UMLListViewItem *item, UMLListViewItem *parent);

public:
    Q_SLOT void slotDiagramCreated(Uml::ID::Type id);
    Q_SLOT void slotDiagramRenamed(Uml::ID::Type id);
    Q_SLOT void slotDiagramRemoved(Uml::ID::Type id);
    Q_SLOT void slotObjectCreated(UMLObject* object);
    Q_SLOT void slotObjectRemoved(UMLObject* object);
    Q_SLOT void connectNewObjectsSlots(UMLObject* object);
    Q_SLOT void childObjectAdded(UMLClassifierListItem* obj);
    Q_SLOT void childObjectRemoved(UMLClassifierListItem* obj);
    Q_SLOT void slotObjectChanged();
    Q_SLOT void slotMenuSelection(QAction* action, const QPoint &position = QPoint());
    Q_SLOT void slotDropped(QDropEvent* de, UMLListViewItem* target);
    Q_SLOT void expandAll(UMLListViewItem *item);
    Q_SLOT void collapseAll(UMLListViewItem *item);
    Q_SLOT void slotCutSuccessful();
    Q_SLOT void slotDeleteSelectedItems();

protected:
    Q_SLOT void slotItemSelectionChanged();
    Q_SLOT void slotExpanded(QTreeWidgetItem* item);
    Q_SLOT void slotCollapsed(QTreeWidgetItem* item);

private:
    UMLListViewItem* recursiveSearchForView(UMLListViewItem* folder,
                                            UMLListViewItem::ListViewType type, Uml::ID::Type id);

    void setBackgroundColor(const QColor & color);

    UMLListViewItem* m_rv;         ///< root view (home)
    UMLListViewItem* m_lv[Uml::ModelType::N_MODELTYPES];  ///< predefined list view roots
    UMLListViewItem* m_datatypeFolder;
    UMLListViewItem* m_settingsFolder; ///< settings root
    UMLDoc*          m_doc;
    bool             m_bStartedCut;
    bool             m_bStartedCopy;
    bool m_bCreatingChildObject;  ///< when creating an attribute or an operation to stop it adding a second listViewItem
    QPoint m_dragStartPosition;
    QMimeData* m_dragCopyData;
    QList<QTreeWidgetItem*> m_findItems;
};

#endif
