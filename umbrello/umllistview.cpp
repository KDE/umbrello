/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "umllistview.h"

// app includes
#include "actor.h"
#include "classifier.h"
#include "debug_utils.h"
#include "package.h"
#include "folder.h"
#include "component.h"
#include "node.h"
#include "artifact.h"
#include "enum.h"
#include "enumliteral.h"
#include "entity.h"
#include "category.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "template.h"
#include "operation.h"
#include "attribute.h"
#include "entityattribute.h"
#include "uniqueconstraint.h"
#include "foreignkeyconstraint.h"
#include "checkconstraint.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistviewitemlist.h"
#include "umllistviewitem.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlviewimageexporter.h"
#include "usecase.h"
#include "model_utils.h"
#include "uniqueid.h"
#include "idchangelog.h"
#include "umldragdata.h"
#include "classpropdlg.h"
#include "umlattributedialog.h"
#include "umlentityattributedialog.h"
#include "umloperationdialog.h"
#include "umltemplatedialog.h"
#include "umluniqueconstraintdialog.h"
#include "umlforeignkeyconstraintdialog.h"
#include "umlcheckconstraintdialog.h"

// kde includes
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kapplication.h>

// qt includes
#include <QtCore/QPointer>
#include <QtCore/QRegExp>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QEvent>
#include <QtGui/QFocusEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QToolTip>

/**
 * Constructs the tree view.
 *
 * @param parent   The parent to this.
 */
UMLListView::UMLListView(QWidget *parent)
  : QTreeWidget(parent),
    m_rv(0),
    m_datatypeFolder(0),
    m_menu(0),
    m_doc(UMLApp::app()->document()),
    m_bStartedCut(false),
    m_bStartedCopy(false),
    m_bCreatingChildObject(false),
    m_dragStartPosition(QPoint()),
    m_editItem(0)
{
    // setup list view
    setAcceptDrops(true);
    //setDropVisualizer(false);
    //setItemsMovable(true);
    //setItemsRenameable(true);
    //setSelectionModeExt(FileManager);
    setFocusPolicy(Qt::StrongFocus);
    setDragEnabled(true);
    //setColumnWidthMode(0, Manual);
    //setDefaultRenameAction(Accept);
    //setResizeMode(LastColumn);
    //header()->setClickEnabled(true);
    //add columns and initial items
    //addColumn(m_doc->name());
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    setEditTriggers(QAbstractItemView::EditKeyPressed);

    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_lv[i] = 0;
    }

    DEBUG_REGISTER(DBG_SRC);

    //setup slots/signals
    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(slotCollapsed(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(slotExpanded(QTreeWidgetItem*)));
    connect(UMLApp::app(), SIGNAL(sigCutSuccessful()), this, SLOT(slotCutSuccessful()));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotItemChanged(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));
}

/**
 * Standard destructor.
 */
UMLListView::~UMLListView()
{
    delete m_datatypeFolder;
}

/**
 * Sets the title.
 * @param column   column in which to write
 * @param text     the text to write
 */
void UMLListView::setTitle(int column, const QString &text)
{
    headerItem()->setText(column, text);
}

/**
 * Handler for item changed signals.
 */
void UMLListView::slotItemChanged(QTreeWidgetItem * item, int column)
{
    if (m_editItem) {
        DEBUG(DBG_SRC) << item->text(column);
        endRename(static_cast<UMLListViewItem*>(item));
    }
}

/**
 * Handlerfor item selection changed signals.
 */
void UMLListView::slotItemSelectionChanged()
{
    UMLListViewItem* currItem = static_cast<UMLListViewItem*>(currentItem());
    if (currItem && currItem->isSelected()) {
        DEBUG(DBG_SRC) << currItem->text(0);
        if (m_editItem) {
            if (m_editItem == currItem) {
                // clicked on the item which is just edited
                endRename(currItem);
            }
            else {
                // other item was selected during editing
                cancelRename(m_editItem);
            }
        }
    }
}

/**
 * Event handler for the tool tip event.
 * Works only for operations to show the signature.
 */
bool UMLListView::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);
        UMLListViewItem * item = static_cast<UMLListViewItem*>(itemAt(helpEvent->pos()));
        if (item) {
            QToolTip::showText(helpEvent->globalPos(), item->toolTip());
        } else {
            QToolTip::hideText();
            e->ignore();
        }
        return true;
    }
    return QTreeWidget::event(e);
}

/**
 * Event filter.
 */
bool UMLListView::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() != QEvent::MouseButtonPress || qstrcmp("QHeader", metaObject()->className()) != 0)
        return QTreeWidget::eventFilter(o, e);
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::RightButton) {
        if (m_menu) {
            m_menu->hide();
            disconnect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuSel(QAction*)));
            delete m_menu;
            m_menu = 0;
        }
        UMLListViewItem * currItem = static_cast<UMLListViewItem*>(currentItem());
        m_menu = new ListPopupMenu(this, UMLListViewItem::lvt_Model, currItem->umlObject());
        m_menu->popup(me->globalPos());
        connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuSel(QAction*)));
        return true;
    }
    return QTreeWidget::eventFilter(o, e);
}

/**
 * Handler for mouse press events.
 * @param me   the mouse event
 */
void UMLListView::mousePressEvent(QMouseEvent *me)
{
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView)
        currentView->umlScene()->clearSelected();
    if (me->modifiers() != Qt::ShiftModifier)
        clearSelection();

    // Get the UMLListViewItem at the point where the mouse pointer was pressed
    UMLListViewItem * item = static_cast<UMLListViewItem*>(itemAt(me->pos()));
    if (item) {
        DEBUG(DBG_SRC) << UMLListViewItem::toString(item->type());
    }
    else {
        DEBUG(DBG_SRC) << "item is NULL";
    }

    const Qt::ButtonState button = me->button();

    if (!item || (button != Qt::RightButton && button != Qt::LeftButton)) {
        UMLApp::app()->docWindow()->updateDocumentation(true);
        return;
    }

    if (button == Qt::LeftButton) {
        UMLObject *o = item->umlObject();
        if (o)
            UMLApp::app()->docWindow()->showDocumentation(o, false);
        else
            UMLApp::app()->docWindow()->updateDocumentation(true);

        m_dragStartPosition = me->pos();
    }

    if (button == Qt::RightButton) {
        if (m_menu != 0) {
            m_menu->hide();
            disconnect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuSel(QAction*)));
            delete m_menu;
            m_menu = 0;
        }
        const UMLListViewItem::ListViewType type = item->type();
        m_menu = new ListPopupMenu(this, type, item->umlObject());
        m_menu->popup(me->globalPos());
        connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuSel(QAction*)));
    }//end if right button

    QTreeWidget::mousePressEvent(me);
}

/**
 * Handler for mouse move events.
 * @param me   the mouse event
 */
void UMLListView::mouseMoveEvent(QMouseEvent* me)
{
    if (!(me->buttons() & Qt::LeftButton))
        return;
    if ((me->pos() - m_dragStartPosition).manhattanLength()
            < QApplication::startDragDistance())
        return;

    QDrag* drag = new QDrag(this);
    drag->setMimeData(getDragData());
    drag->exec(Qt::CopyAction);
}

/**
 * Handler for mouse release event.
 * @param me   the mouse event
 */
void UMLListView::mouseReleaseEvent(QMouseEvent *me)
{
    if (me->button() != Qt::LeftButton) {
        QTreeWidget::mouseReleaseEvent(me);
        return;
    }
    UMLListViewItem *item = static_cast<UMLListViewItem*>(itemAt(me->pos()));
    if (item == 0 || !Model_Utils::typeIsDiagram(item->type())) {
        QTreeWidget::mouseReleaseEvent(me);
        return;
    }
    // Switch to diagram on mouse release - not on mouse press
    // because the user might intend a drag-to-note.
    m_doc->changeCurrentView(item->getID());
    UMLApp::app()->docWindow()->showDocumentation(m_doc->findView(item->getID()), false);
    QTreeWidget::mouseReleaseEvent(me);
}

/**
 * Handler for key press events.
 * @param ke   the key event
 */
void UMLListView::keyPressEvent(QKeyEvent *ke)
{
    UMLView *view = UMLApp::app()->currentView();
    if (view && view->umlScene()->getSelectCount()) {
        // Widgets have been selected in the diagram area,
        // assume they handle the keypress.
        ke->accept();                 // munge and do nothing
    }
    else {
        const int k = ke->key();
        if (k == Qt::Key_F2) {
            // disable qt default key for item rename, it does not work yet
            ke->accept();
        } else if (k == Qt::Key_Delete || k == Qt::Key_Backspace) {
            // delete every selected item
            UMLListViewItemList itemsSelected = selectedItemsRoot();
            foreach(UMLListViewItem *item, itemsSelected) {
                deleteItem(item);
            }
        } else  {
            QTreeWidget::keyPressEvent(ke); // let parent handle it
        }
    }
}

/**
 * Called when a right mouse button menu has an item selected.
 * @param action   the selected action
 */
void UMLListView::popupMenuSel(QAction* action)
{
    UMLListViewItem * currItem = static_cast<UMLListViewItem*>(currentItem());
    if (!currItem) {
        DEBUG(DBG_SRC) << "Invoked without currently selectedItem!";
        return;
    }
    UMLListViewItem::ListViewType lvt = currItem->type();
    UMLObject::ObjectType umlType = UMLObject::ot_UMLObject;
    ListPopupMenu::MenuType menuType = m_menu->getMenuType(action);
    QString name;

    switch (menuType) {
    case ListPopupMenu::mt_Class:
        addNewItem(currItem, UMLListViewItem::lvt_Class);
        break;

    case ListPopupMenu::mt_Package:
        addNewItem(currItem, UMLListViewItem::lvt_Package);
        break;

    case ListPopupMenu::mt_Subsystem:
        addNewItem(currItem, UMLListViewItem::lvt_Subsystem);
        break;

    case ListPopupMenu::mt_Component:
        addNewItem(currItem, UMLListViewItem::lvt_Component);
        break;

    case ListPopupMenu::mt_Node:
        addNewItem(currItem, UMLListViewItem::lvt_Node);
        break;

    case ListPopupMenu::mt_Artifact:
        addNewItem(currItem, UMLListViewItem::lvt_Artifact);
        break;

    case ListPopupMenu::mt_Interface:
        addNewItem(currItem, UMLListViewItem::lvt_Interface);
        break;

    case ListPopupMenu::mt_Enum:
        addNewItem(currItem, UMLListViewItem::lvt_Enum);
        break;

    case ListPopupMenu::mt_EnumLiteral:
        addNewItem(currItem, UMLListViewItem::lvt_EnumLiteral);
        break;

    case ListPopupMenu::mt_Template:
        addNewItem(currItem, UMLListViewItem::lvt_Template);
        break;

    case ListPopupMenu::mt_Entity:
        addNewItem(currItem, UMLListViewItem::lvt_Entity);
        break;

    case ListPopupMenu::mt_Category:
        addNewItem(currItem, UMLListViewItem::lvt_Category);
        break;

    case ListPopupMenu::mt_DisjointSpecialisation:
        {
            UMLCategory* catObj = static_cast<UMLCategory*>(currItem->umlObject());
            catObj->setType(UMLCategory::ct_Disjoint_Specialisation);
        }
        break;

    case ListPopupMenu::mt_OverlappingSpecialisation:
        {
            UMLCategory* catObj = static_cast<UMLCategory*>(currItem->umlObject());
            catObj->setType(UMLCategory::ct_Overlapping_Specialisation);
        }
        break;

    case ListPopupMenu::mt_Union:
        {
            UMLCategory* catObj = static_cast<UMLCategory*>(currItem->umlObject());
            catObj->setType(UMLCategory::ct_Union);
        }
        break;

    case ListPopupMenu::mt_Datatype:
        addNewItem(currItem, UMLListViewItem::lvt_Datatype);
        break;

    case ListPopupMenu::mt_Actor:
        addNewItem(currItem, UMLListViewItem::lvt_Actor);
        break;

    case ListPopupMenu::mt_UseCase:
        addNewItem(currItem, UMLListViewItem::lvt_UseCase);
        break;

    case ListPopupMenu::mt_Attribute:
        addNewItem(currItem, UMLListViewItem::lvt_Attribute);
        break;

    case ListPopupMenu::mt_EntityAttribute:
        addNewItem(currItem, UMLListViewItem::lvt_EntityAttribute);
        break;

    case ListPopupMenu::mt_Operation:
        addNewItem(currItem, UMLListViewItem::lvt_Operation);
        break;

    case ListPopupMenu::mt_UniqueConstraint:
        addNewItem(currItem, UMLListViewItem::lvt_UniqueConstraint);
        break;

    case ListPopupMenu::mt_PrimaryKeyConstraint:
        addNewItem(currItem, UMLListViewItem::lvt_PrimaryKeyConstraint);
        break;

    case ListPopupMenu::mt_ForeignKeyConstraint:
        addNewItem(currItem, UMLListViewItem::lvt_ForeignKeyConstraint);
        break;

    case ListPopupMenu::mt_CheckConstraint:
        addNewItem(currItem, UMLListViewItem::lvt_CheckConstraint);
        break;

    case ListPopupMenu::mt_Expand_All:
        expandAll(currItem);
        break;

    case ListPopupMenu::mt_Collapse_All:
        collapseAll(currItem);
        break;

    case ListPopupMenu::mt_Export_Image:
        m_doc->findView(currItem->getID())->getImageExporter()->exportView();
        break;

    case ListPopupMenu::mt_Externalize_Folder:
        {
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            UMLFolder *modelFolder = dynamic_cast<UMLFolder*>(current->umlObject());
            if (modelFolder == 0) {
                uError() << "modelFolder is 0";
                return;
            }
            // configure & show the file dialog
            const QString rootDir(m_doc->url().directory());
            QPointer<KFileDialog> fileDialog = new KFileDialog(rootDir, "*.xml", this);
            fileDialog->setCaption(i18n("Externalize Folder"));
            fileDialog->setOperationMode(KFileDialog::Other);
            // set a sensible default filename
            QString defaultFilename = current->text(0).toLower();
            defaultFilename.replace(QRegExp("\\W+"), "_");
            defaultFilename.append(".xml");  // default extension
            fileDialog->setSelection(defaultFilename);
            KUrl selURL;
            if (fileDialog->exec() == QDialog::Accepted) {
                selURL = fileDialog->selectedUrl();
            }
            delete fileDialog;
            if (selURL.isEmpty())
                return;
            QString path = selURL.toLocalFile();
            QString fileName = path;
            if (fileName.startsWith(rootDir)) {
                fileName.remove(rootDir);
            } else {
                KMessageBox::error(
                    0,
                    i18n("Folder %1 must be relative to the main model directory, %2.", path, rootDir),
                    i18n("Path Error"));
                return;
            }
            QFile file(path);
            // Warn if file exists.
            if (file.exists()) {
                KMessageBox::error(
                    0,
                    i18n("File %1 already exists!\nThe existing file will be overwritten.", fileName),
                    i18n("File Exist"));
            }
            // Test if file is writable.
            if (file.open(QIODevice::WriteOnly)) {
                file.close();
            } else {
                KMessageBox::error(
                    0,
                    i18n("There was a problem saving file: %1", fileName),
                    i18n("Save Error"));
                return;
            }
            modelFolder->setFolderFile(fileName);
            // Recompute text of the folder
            QString folderText = current->text(0);
            folderText.remove(QRegExp("\\s*\\(.*$"));
            folderText.append(" (" + fileName + ')');
            current->setText(folderText);
            break;
        }

    case ListPopupMenu::mt_Internalize_Folder:
        {
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            UMLFolder *modelFolder = dynamic_cast<UMLFolder*>(current->umlObject());
            if (modelFolder == 0) {
                uError() << "modelFolder is 0";
                return;
            }
            modelFolder->setFolderFile(QString());
            // Recompute text of the folder
            QString folderText = current->text(0);
            folderText.remove(QRegExp("\\s*\\(.*$"));
            current->setText(folderText);
            break;
        }

    case ListPopupMenu::mt_Model:
        {
            bool ok = false;
            QString name = KInputDialog::getText(i18n("Enter Model Name"),
                                                 i18n("Enter the new name of the model:"),
                                                 m_doc->name(), &ok, UMLApp::app());
            if (ok) {
                setTitle(0, name);
                m_doc->setName(name);
            }
            break;
        }

    case ListPopupMenu::mt_Rename:
        startRename(currItem);
        break;

    case ListPopupMenu::mt_Delete:
        deleteItem(currItem);
        break;

    case ListPopupMenu::mt_Properties:
        // first check if we are on a diagram
        if (Model_Utils::typeIsDiagram(lvt)) {
            UMLView * pView = m_doc->findView(currItem->getID());
            if (!pView) {
                return;
            }
            UMLApp::app()->docWindow()->updateDocumentation(false);
            pView->umlScene()->showPropDialog();
            UMLApp::app()->docWindow()->showDocumentation(pView, true);
//:TODO:delete?            currItem->cancelRename(0);
            return;
        }

        { // ok, we are on another object, so find out on which one
            UMLObject * object = currItem->umlObject();
            if (!object) {
                uError() << "UMLObject of ... is null! Doing nothing.";
                return;
            }
            umlType = object->baseType();

            if (Model_Utils::typeIsCanvasWidget(lvt)) {
                object->showPropertiesPagedDialog(ClassPropDlg::page_gen);
            } else if (umlType == UMLObject::ot_EnumLiteral) {
                // Show the Enum Literal Dialog
                UMLEnumLiteral* selectedEnumLiteral = static_cast<UMLEnumLiteral*>(object);
                selectedEnumLiteral->showPropertiesDialog(this);

            } else if (umlType == UMLObject::ot_Attribute) {
                // show the attribute dialog
                UMLAttribute* selectedAttribute = static_cast<UMLAttribute*>(object);
                QPointer<UMLAttributeDialog> dialog = new UMLAttributeDialog(this, selectedAttribute);
                dialog->exec();
                delete dialog;
            } else if (umlType == UMLObject::ot_EntityAttribute) {
                // show the attribute dialog
                UMLEntityAttribute* selectedAttribute = static_cast<UMLEntityAttribute*>(object);
                QPointer<UMLEntityAttributeDialog> dialog = new UMLEntityAttributeDialog(this, selectedAttribute);
                dialog->exec();
                delete dialog;
            } else if (umlType == UMLObject::ot_Operation) {
                // show the operation dialog
                UMLOperation* selectedOperation = static_cast<UMLOperation*>(object);
                QPointer<UMLOperationDialog> dialog = new UMLOperationDialog(this, selectedOperation);
                dialog->exec();
                delete dialog;
            } else if (umlType == UMLObject::ot_Template) {
                // show the template dialog
                UMLTemplate* selectedTemplate = static_cast<UMLTemplate*>(object);
                QPointer<UMLTemplateDialog> dialog = new UMLTemplateDialog(this, selectedTemplate);
                dialog->exec();
                delete dialog;
            } else if (umlType == UMLObject::ot_UniqueConstraint) {
                // show the Unique Constraint dialog
                UMLUniqueConstraint* selectedUniqueConstraint = static_cast<UMLUniqueConstraint*>(object);
                QPointer<UMLUniqueConstraintDialog> dialog = new UMLUniqueConstraintDialog(this, selectedUniqueConstraint);
                dialog->exec();
                delete dialog;
            } else if (umlType == UMLObject::ot_ForeignKeyConstraint) {
                // show the Unique Constraint dialog
                UMLForeignKeyConstraint* selectedForeignKeyConstraint = static_cast<UMLForeignKeyConstraint*>(object);
                QPointer<UMLForeignKeyConstraintDialog> dialog = new UMLForeignKeyConstraintDialog(this, selectedForeignKeyConstraint);
                dialog->exec();
                delete dialog;
            } else if (umlType == UMLObject::ot_CheckConstraint) {
                // show the Check Constraint dialog
                UMLCheckConstraint* selectedCheckConstraint = static_cast<UMLCheckConstraint*>(object);
                QPointer<UMLCheckConstraintDialog> dialog = new UMLCheckConstraintDialog(this, selectedCheckConstraint);
                dialog->exec();
                delete dialog;
            } else {
                uWarning() << "calling properties on unknown type";
            }
//:TODO:delete?            currItem->cancelRename(0);
        }
        // Bug 268469: Changing the package of a class deletes the old widget.
        // By reloading the current item we are sure to not use a destroyed object
        currItem = (UMLListViewItem*)currentItem();
        if (currItem)
            currItem->cancelRename(0);
        break;

    case ListPopupMenu::mt_Logical_Folder:
        addNewItem(currItem, UMLListViewItem::lvt_Logical_Folder);
        break;

    case ListPopupMenu::mt_UseCase_Folder:
        addNewItem(currItem, UMLListViewItem::lvt_UseCase_Folder);
        break;

    case ListPopupMenu::mt_Component_Folder:
        addNewItem(currItem, UMLListViewItem::lvt_Component_Folder);
        break;

    case ListPopupMenu::mt_Deployment_Folder:
        addNewItem(currItem, UMLListViewItem::lvt_Deployment_Folder);
        break;

    case ListPopupMenu::mt_EntityRelationship_Folder:
        addNewItem(currItem, UMLListViewItem::lvt_EntityRelationship_Folder);
        break;

    case ListPopupMenu::mt_Cut:
        m_bStartedCut = true;
        m_bStartedCopy = false;
        UMLApp::app()->slotEditCut();
        break;

    case ListPopupMenu::mt_Copy:
        m_bStartedCut = false;
        m_bStartedCopy = true;
        UMLApp::app()->slotEditCopy();
        break;

    case ListPopupMenu::mt_Paste:
        UMLApp::app()->slotEditPaste();
        break;

    default:
        {
            Uml::DiagramType dt = ListPopupMenu::convert_MT_DT(menuType);
            if (dt == Uml::DiagramType::Undefined) {
                uWarning() << "unknown type" << menuType;
            } else {
                UMLObject* object = currItem->umlObject();
                UMLFolder* f = dynamic_cast<UMLFolder*>(object);
                if (f == 0) {
                    uError() << "menuType=" << menuType
                             << ": current item's UMLObject is not a UMLFolder";
                }
                else {
                    m_doc->createDiagram(f, dt);
                }
            }
        }
        break;
    }//end switch
}

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
UMLListViewItem *UMLListView::findFolderForDiagram(Uml::DiagramType dt)
{
    UMLListViewItem *p = static_cast<UMLListViewItem*>(currentItem());
    if (p && Model_Utils::typeIsFolder(p->type())
            && !Model_Utils::typeIsRootView(p->type())) {
        return p;
    }
    switch (dt) {
    case Uml::DiagramType::UseCase:
        p = m_lv[Uml::ModelType::UseCase];
        break;
    case Uml::DiagramType::Component:
        p = m_lv[Uml::ModelType::Component];
        break;
    case Uml::DiagramType::Deployment:
        p = m_lv[Uml::ModelType::Deployment];
        break;
    case Uml::DiagramType::EntityRelationship:
        p = m_lv[Uml::ModelType::EntityRelationship];
        break;
    default:
        p = m_lv[Uml::ModelType::Logical];
        break;
    }
    return p;
}

/**
 * Creates a new item to represent a new diagram.
 * @param id the id of the new diagram
 */
void UMLListView::slotDiagramCreated(Uml::IDType id)
{
    if (m_doc->loading()) {
        return;
    }
    UMLView *v = m_doc->findView(id);
    if (!v) {
        return;
    }
    const Uml::DiagramType dt = v->umlScene()->type();
    UMLListViewItem* p = findFolderForDiagram(dt);
    UMLListViewItem* item = new UMLListViewItem(p, v->umlScene()->name(), Model_Utils::convert_DT_LVT(dt), id);
    setSelected(item, true);
    UMLApp::app()->docWindow()->showDocumentation(v , false);
}

/**
 * Determine the parent ListViewItem given an UMLObject.
 *
 * @param object   Pointer to the UMLObject for which to look up the parent.
 * @return    Pointer to the parent UMLListViewItem chosen.
 *            Returns NULL on error (no parent could be determined.)
 */
UMLListViewItem* UMLListView::determineParentItem(UMLObject* object) const
{
    UMLListViewItem* parentItem = 0;
    UMLListViewItem* current = (UMLListViewItem*) currentItem();
    UMLListViewItem::ListViewType lvt = UMLListViewItem::lvt_Unknown;
    if (current)
        lvt = current->type();
    UMLObject::ObjectType t = object->baseType();

    switch (t) {
    case UMLObject::ot_Attribute:
    case UMLObject::ot_Operation:
    case UMLObject::ot_Template:
    case UMLObject::ot_EnumLiteral:
    case UMLObject::ot_EntityAttribute:
    case UMLObject::ot_UniqueConstraint:
    case UMLObject::ot_ForeignKeyConstraint:
    case UMLObject::ot_CheckConstraint:
        //this will be handled by childObjectAdded
        return 0;
        break;
    case UMLObject::ot_Association:
    case UMLObject::ot_Role:
    case UMLObject::ot_Stereotype:
        return 0;  // currently no representation in list view
        break;
    default: {
        UMLPackage *pkg = object->umlPackage();
        if (pkg) {
            UMLListViewItem* pkgItem = findUMLObject(pkg);
            if (pkgItem == 0)
                uError() << "could not find parent package " << pkg->name();
            else
                parentItem = pkgItem;
        } else if ((lvt == UMLListViewItem::lvt_UseCase_Folder &&
                    (t == UMLObject::ot_Actor || t == UMLObject::ot_UseCase))
                   || (lvt == UMLListViewItem::lvt_Component_Folder && t == UMLObject::ot_Component)
                   || (lvt == UMLListViewItem::lvt_Deployment_Folder && t == UMLObject::ot_Node)
                   || (lvt == UMLListViewItem::lvt_EntityRelationship_Folder && t == UMLObject::ot_Entity)) {
            parentItem = current;
        } else if (t == UMLObject::ot_Datatype) {
            parentItem = m_datatypeFolder;
        } else {
            Uml::ModelType guess = Model_Utils::guessContainer(object);
            parentItem = m_lv[guess];
        }
    }
    break;
    }
    return parentItem;
}

/**
 * Return true if the given ObjectType permits child items.
 * A "child item" is anything that qualifies as a UMLClassifierListItem,
 * e.g. operations and attributes of classifiers.
 */
bool UMLListView::mayHaveChildItems(UMLObject::ObjectType type)
{
    bool retval = false;
    switch (type) {
    case UMLObject::ot_Class:
    case UMLObject::ot_Interface:
    case UMLObject::ot_Enum:
    case UMLObject::ot_Entity:  // CHECK: more?
        retval = true;
        break;
    default:
        break;
    }
    return retval;
}

/**
 * Creates a new list view item and connects the appropriate signals/slots.
 * @param object the newly created object
 */
void UMLListView::slotObjectCreated(UMLObject* object)
{
    if (m_bCreatingChildObject) {
        // @todo eliminate futile signal traffic
        // e.g. we get here thru various indirections from
        // ClassifierListPage::slot{Up,Down}Clicked()
        return;
    }
    UMLListViewItem* newItem = findUMLObject(object);

    if (newItem) {
        DEBUG(DBG_SRC) << newItem->type();
        DEBUG(DBG_SRC) << object->name() << ", id= " << ID2STR(object->id())
                       << ": item already exists.";
        Icon_Utils::IconType icon = Model_Utils::convert_LVT_IT(newItem->type());
        newItem->setIcon(icon);
        return;
    }
    UMLListViewItem* parentItem = determineParentItem(object);
    if (parentItem == 0)
        return;
    UMLObject::ObjectType type = object->baseType();

    connectNewObjectsSlots(object);
    const UMLListViewItem::ListViewType lvt = Model_Utils::convert_OT_LVT(object);
    QString name = object->name();
    if (type == UMLObject::ot_Folder) {
        UMLFolder *f = static_cast<UMLFolder*>(object);
        QString folderFile = f->folderFile();
        if (!folderFile.isEmpty())
            name.append(" (" + folderFile + ')');
    }
    newItem = new UMLListViewItem(parentItem, name, lvt, object);
    if (mayHaveChildItems(type)) {
        UMLClassifier *c = static_cast<UMLClassifier*>(object);
        UMLClassifierListItemList cListItems = c->getFilteredList(UMLObject::ot_UMLObject);
        foreach(UMLClassifierListItem *cli, cListItems)
            childObjectAdded(cli, c);
    }
    if (m_doc->loading())
        return;
    scrollToItem(newItem);
    newItem->setOpen(true);
    clearSelection();
    setSelected(newItem, true);
    UMLApp::app()->docWindow()->showDocumentation(object, false);
}

/**
 * Connect some signals into slots in the list view for newly created UMLObjects.
 */
void UMLListView::connectNewObjectsSlots(UMLObject* object)
{
    UMLObject::ObjectType type = object->baseType();
    switch (type) {
    case UMLObject::ot_Class:
    case UMLObject::ot_Interface: {
        UMLClassifier *c = static_cast<UMLClassifier*>(object);
        connect(c, SIGNAL(attributeAdded(UMLClassifierListItem*)),
                this, SLOT(childObjectAdded(UMLClassifierListItem*)));
        connect(c, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
        connect(c, SIGNAL(operationAdded(UMLClassifierListItem*)),
                this, SLOT(childObjectAdded(UMLClassifierListItem*)));
        connect(c, SIGNAL(operationRemoved(UMLClassifierListItem*)),
                this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
        connect(c, SIGNAL(templateAdded(UMLClassifierListItem*)),
                this, SLOT(childObjectAdded(UMLClassifierListItem*)));
        connect(c, SIGNAL(templateRemoved(UMLClassifierListItem*)),
                this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
        connect(object, SIGNAL(modified()), this, SLOT(slotObjectChanged()));
    }
    break;
    case UMLObject::ot_Enum: {
        UMLEnum *e = static_cast<UMLEnum*>(object);
        connect(e, SIGNAL(enumLiteralAdded(UMLClassifierListItem*)),
                this, SLOT(childObjectAdded(UMLClassifierListItem*)));
        connect(e, SIGNAL(enumLiteralRemoved(UMLClassifierListItem*)),
                this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
    }
    connect(object, SIGNAL(modified()), this, SLOT(slotObjectChanged()));
    break;
    case UMLObject::ot_Entity: {
        UMLEntity *ent = static_cast<UMLEntity*>(object);
        connect(ent, SIGNAL(entityAttributeAdded(UMLClassifierListItem*)),
                this, SLOT(childObjectAdded(UMLClassifierListItem*)));
        connect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
        connect(ent, SIGNAL(entityConstraintAdded(UMLClassifierListItem*)),
                this, SLOT(childObjectAdded(UMLClassifierListItem*)));
        connect(ent, SIGNAL(entityConstraintRemoved(UMLClassifierListItem*)),
                this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
    }
    connect(object, SIGNAL(modified()), this, SLOT(slotObjectChanged()));
    break;
    case UMLObject::ot_Datatype:
    case UMLObject::ot_Attribute:
    case UMLObject::ot_Operation:
    case UMLObject::ot_Template:
    case UMLObject::ot_EnumLiteral:
    case UMLObject::ot_EntityAttribute:
    case UMLObject::ot_UniqueConstraint:
    case UMLObject::ot_ForeignKeyConstraint:
    case UMLObject::ot_CheckConstraint:
    case UMLObject::ot_Package:
    case UMLObject::ot_Actor:
    case UMLObject::ot_UseCase:
    case UMLObject::ot_Component:
    case UMLObject::ot_Artifact:
    case UMLObject::ot_Node:
    case UMLObject::ot_Folder:
    case UMLObject::ot_Category:
        connect(object, SIGNAL(modified()), this, SLOT(slotObjectChanged()));
        break;
    case UMLObject::ot_UMLObject:
    case UMLObject::ot_Association:
    case UMLObject::ot_Stereotype:
        break;
    default:
        uWarning() << "unknown type in connectNewObjectsSlots";
        break;
    }
}

/**
 * Calls updateObject() on the item representing the sending object
 * no parameters, uses sender() to work out which object called the slot.
 */
void UMLListView::slotObjectChanged()
{
    if (m_doc->loading()) { //needed for class wizard
        return;
    }
    UMLObject* obj = const_cast<UMLObject*>(dynamic_cast<const UMLObject*>(sender()));
    UMLListViewItem* item = findUMLObject(obj);
    if (item) {
        item->updateObject();
    }
}

/**
 * Adds a new operation, attribute or template item to a classifier.
 * @param obj the child object
 */
void UMLListView::childObjectAdded(UMLClassifierListItem* obj)
{
    UMLClassifier *parent = const_cast<UMLClassifier*>(dynamic_cast<const UMLClassifier*>(sender()));
    childObjectAdded(obj, parent);
}

/**
 * Adds a new operation, attribute or template item to a classifier, identical to
 * childObjectAdded(obj) but with an explicit parent.
 * @param child the child object
 * @param parent the parent object
 */
void UMLListView::childObjectAdded(UMLClassifierListItem* child, UMLClassifier* parent)
{
    if (m_bCreatingChildObject)
        return;
    const QString text = child->toString(Uml::SignatureType::SigNoVis);
    UMLListViewItem *childItem = 0;
    UMLListViewItem *parentItem = findUMLObject(parent);
    if (parentItem == 0) {
        DEBUG(DBG_SRC) << child->name() << ": parent " << parent->name()
                       << " does not yet exist, creating it now.";
        const UMLListViewItem::ListViewType lvt = Model_Utils::convert_OT_LVT(parent);
        parentItem = new UMLListViewItem(m_lv[Uml::ModelType::Logical], parent->name(), lvt, parent);
    } else {
        childItem = parentItem->findChildObject(child);
    }
    if (childItem) {
        childItem->setText(text);
    } else {
        const UMLListViewItem::ListViewType lvt = Model_Utils::convert_OT_LVT(child);
        childItem = new UMLListViewItem(parentItem, text, lvt, child);
        if (! m_doc->loading()) {
            scrollToItem(childItem);
            clearSelection();
            setSelected(childItem, true);
        }
        connectNewObjectsSlots(child);
    }
}

/**
 * Deletes the list view item.
 * @param obj the object to remove
 */
void UMLListView::childObjectRemoved(UMLClassifierListItem* obj)
{
    UMLClassifier *parent = const_cast<UMLClassifier*>(dynamic_cast<const UMLClassifier*>(sender()));
    UMLListViewItem *parentItem = findUMLObject(parent);
    if (parentItem == 0) {
        uError() << obj->name() << ": cannot find parent UMLListViewItem";
        return;
    }
    parentItem->deleteChildItem(obj);
}

/**
 * Renames a diagram in the list view
 * @param id    the id of the renamed diagram
 */
void UMLListView::slotDiagramRenamed(Uml::IDType id)
{
    UMLListViewItem* item;
    UMLView* v = m_doc->findView(id);
    if ((item = findView(v)) == 0) {
        uError() << "UMLDoc::findView(" << ID2STR(id) << ") returns 0";
        return;
    }
    item->setText(v->name());
}

/**
 * Sets the document his is associated with.  This is important as
 * this is required as to setup the callbacks.
 *
 * @param doc   The document to associate with this class.
 */
void UMLListView::setDocument(UMLDoc *doc)
{
    if (m_doc && m_doc != doc) {
        //disconnect signals from old doc and reset view
    }
    m_doc = doc;

    connect(m_doc, SIGNAL(sigDiagramCreated(Uml::IDType)), this, SLOT(slotDiagramCreated(Uml::IDType)));
    connect(m_doc, SIGNAL(sigDiagramRemoved(Uml::IDType)), this, SLOT(slotDiagramRemoved(Uml::IDType)));
    connect(m_doc, SIGNAL(sigDiagramRenamed(Uml::IDType)), this, SLOT(slotDiagramRenamed(Uml::IDType)));
    connect(m_doc, SIGNAL(sigObjectCreated(UMLObject*)),   this, SLOT(slotObjectCreated(UMLObject*)));
    connect(m_doc, SIGNAL(sigObjectRemoved(UMLObject*)),   this, SLOT(slotObjectRemoved(UMLObject*)));
}

/**
 * Disconnects signals and removes the list view item.
 * @param object the object about to be removed
 */
void UMLListView::slotObjectRemoved(UMLObject* object)
{
    if (m_doc->loading()) { //needed for class wizard
        return;
    }
    disconnect(object, SIGNAL(modified()), this, SLOT(slotObjectChanged()));
    UMLListViewItem* item = findItem(object->id());
    delete item;
    UMLApp::app()->docWindow()->updateDocumentation(true);
}

/**
 * Removes the item representing a diagram.
 * @param id the id of the diagram
 */
void UMLListView::slotDiagramRemoved(Uml::IDType id)
{
    UMLListViewItem* item = findItem(id);
    delete item;
    UMLApp::app()->docWindow()->updateDocumentation(true);
}

/**
 *
 */
UMLDragData* UMLListView::getDragData()
{
    UMLListViewItemList itemsSelected = selectedItems();

    UMLListViewItemList  list;
    foreach(UMLListViewItem* item, itemsSelected) {
        UMLListViewItem::ListViewType type = item->type();
        if (!Model_Utils::typeIsCanvasWidget(type) && !Model_Utils::typeIsDiagram(type)
                && !Model_Utils::typeIsClassifierList(type)) {
            return 0;
        }
        list.append(item);
    }

    UMLDragData *t = new UMLDragData(list, this);

    return t;
}

/**
 * This methods looks for a object in a folder an its subfolders recursive.
 * @param folder   The folder entry of the list view.
 * @param obj      The object to be found in the folder.
 * @return The object if found else a NULL pointer.
 */
UMLListViewItem * UMLListView::findUMLObjectInFolder(UMLListViewItem* folder, UMLObject* obj)
{
    for (int i=0; i < folder->childCount(); ++i) {
        UMLListViewItem *item = folder->childItem(i);
        switch (item->type()) {
        case UMLListViewItem::lvt_Actor :
        case UMLListViewItem::lvt_UseCase :
        case UMLListViewItem::lvt_Class :
        case UMLListViewItem::lvt_Package :
        case UMLListViewItem::lvt_Subsystem :
        case UMLListViewItem::lvt_Component :
        case UMLListViewItem::lvt_Node :
        case UMLListViewItem::lvt_Artifact :
        case UMLListViewItem::lvt_Interface :
        case UMLListViewItem::lvt_Datatype :
        case UMLListViewItem::lvt_Enum :
        case UMLListViewItem::lvt_Entity :
        case UMLListViewItem::lvt_Category:
            if (item->umlObject() == obj)
                return item;
            break;
        case UMLListViewItem::lvt_Logical_Folder :
        case UMLListViewItem::lvt_UseCase_Folder :
        case UMLListViewItem::lvt_Component_Folder :
        case UMLListViewItem::lvt_Deployment_Folder :
        case UMLListViewItem::lvt_EntityRelationship_Folder :
        case UMLListViewItem::lvt_Datatype_Folder : {
            UMLListViewItem *temp = findUMLObjectInFolder(item, obj);
            if (temp)
                return temp;
        }
        default:
            break;
        }
    }
    return 0;
}

/**
 * Find an UMLObject in the listview.
 *
 * @param p   Pointer to the object to find in the list view.
 * @return    Pointer to the UMLObject found or NULL if not found.
 */
UMLListViewItem * UMLListView::findUMLObject(const UMLObject *p) const
{
    for (int i=0; i < topLevelItemCount(); ++i) {
        UMLListViewItem *item = static_cast<UMLListViewItem*>(topLevelItem(i));
        UMLListViewItem *testItem = item->findUMLObject(p);
        if (testItem)
            return testItem;
    }
    return 0;
}

/**
 * Changes the icon for the given UMLObject to the given icon.
 */
void UMLListView::changeIconOf(UMLObject *o, Icon_Utils::IconType to)
{
    UMLListViewItem *item = findUMLObject(o);
    if (item)
        item->setIcon(to);
}

/**
 * Searches through the tree for the item which represents the diagram given.
 * @param v  the diagram to search for
 * @return the item which represents the diagram
 */
UMLListViewItem* UMLListView::findView(UMLView* v)
{
    if (!v) {
        uWarning() << "returning 0 - param is 0.";
        return 0;
    }
    UMLListViewItem* item;
    Uml::DiagramType dType = v->umlScene()->type();
    UMLListViewItem::ListViewType type = Model_Utils::convert_DT_LVT(dType);
    Uml::IDType id = v->umlScene()->getID();
    if (dType == Uml::DiagramType::UseCase) {
        item = m_lv[Uml::ModelType::UseCase];
    } else if (dType == Uml::DiagramType::Component) {
        item = m_lv[Uml::ModelType::Component];
    } else if (dType == Uml::DiagramType::Deployment) {
        item = m_lv[Uml::ModelType::Deployment];
    } else if (dType == Uml::DiagramType::EntityRelationship) {
        item = m_lv[Uml::ModelType::EntityRelationship];
    } else {
        item = m_lv[Uml::ModelType::Logical];
    }
    for (int i=0; i < item->childCount(); i++) {
        UMLListViewItem* foundItem = recursiveSearchForView(item->childItem(i), type, id);
        if (foundItem) {
            return foundItem;
        }
    }
    uWarning() << "returning 0";
    DEBUG(DBG_SRC) << "but was looking for " << *item;
    return 0;
}

/**
 * Searches the tree for a diagram (view).
 * Warning: these method may return in some cases the wrong diagram
 * Used by findView().
 */
UMLListViewItem* UMLListView::recursiveSearchForView(UMLListViewItem* listViewItem,
        UMLListViewItem::ListViewType type, Uml::IDType id)
{
    if (!listViewItem)
        return 0;

    if (Model_Utils::typeIsFolder(listViewItem->type())) {
        for (int i=0; i < listViewItem->childCount(); i++) {
            UMLListViewItem* child = listViewItem->childItem(i);
            UMLListViewItem* resultListViewItem = recursiveSearchForView(child, type, id);
            if (resultListViewItem)
                return resultListViewItem;
        }
    } else {
        if (listViewItem->type() == type && listViewItem->getID() == id)
            return listViewItem;
    }
    return 0;
}

/**
 * Searches through the tree for the item with the given ID.
 *
 * @param id   The ID to search for.
 * @return     The item with the given ID or 0 if not found.
 */
UMLListViewItem* UMLListView::findItem(Uml::IDType id)
{
    UMLListViewItem *topLevel = static_cast<UMLListViewItem*>(topLevelItem(0));
    UMLListViewItem *item = topLevel->findItem(id);
    if (item)
        return item;
    return 0;
}

/**
 * Carries out initalisation of attributes in class.
 * This method is called more than once during an instance's lifetime (by UMLDoc)!
 * So we must not allocate any memory before freeing the previously allocated one
 * or do connect()s.
 */
void UMLListView::init()
{
    if (m_rv == 0) {
        m_rv =  new UMLListViewItem(this, i18n("Views"), UMLListViewItem::lvt_View);
        m_rv->setID("Views");
        //m_rv->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

        for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
            Uml::ModelType mt = Uml::ModelType::Value(i);
            UMLFolder *sysFolder = m_doc->rootFolder(mt);
            UMLListViewItem::ListViewType lvt = Model_Utils::convert_MT_LVT(mt);
            m_lv[i] = new UMLListViewItem(m_rv, sysFolder->localName(), lvt, sysFolder);
        }
    } else {
        for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i)
            deleteChildrenOf(m_lv[i]);
    }
    UMLFolder *datatypeFolder = m_doc->datatypeFolder();
    if (!m_datatypeFolder) {
        m_datatypeFolder = new UMLListViewItem(m_lv[Uml::ModelType::Logical], datatypeFolder->localName(),
                                           UMLListViewItem::lvt_Datatype_Folder, datatypeFolder);
    }
    m_rv->setOpen(true);
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_lv[i]->setOpen(true);
    }
    m_datatypeFolder->setOpen(false);

    //setup misc.
    delete m_menu;
    m_menu = 0;
    m_bStartedCut = m_bStartedCopy = false;
    m_bCreatingChildObject = false;
    headerItem()->setHidden(true);
}

/**
 * Remove all items and subfolders of the main folders.
 * Special case: The datatype folder, child of the logical view, is not deleted.
 */
void UMLListView::clean()
{
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        deleteChildrenOf(m_lv[i]);
    }
    //deleteChildrenOf(m_datatypeFolder);
}

/**
 * Set the current view to the given view.
 *
 * @param view   The current view.
 */
void UMLListView::setView(UMLView * view)
{
    if (!view)
        return;
    UMLListViewItem * temp = findView(view);
    if (temp)
        setSelected(temp, true);
}

/**
 * Event handler for mouse double click.
 */
void UMLListView::mouseDoubleClickEvent(QMouseEvent * me)
{
    UMLListViewItem * item = static_cast<UMLListViewItem *>(currentItem());
    if (!item || me->button() != Qt::LeftButton)
        return;
    //see if on view
    UMLListViewItem::ListViewType lvType = item->type();
    if (Model_Utils::typeIsDiagram(lvType)) {
        UMLView * pView = m_doc->findView(item->getID());
        if (!pView)
            return;
        UMLApp::app()->docWindow()->updateDocumentation(false);
        pView->umlScene()->showPropDialog();
        UMLApp::app()->docWindow()->showDocumentation(pView, true);
//:TODO:delete?        item->cancelRename(0);
        return;
    }
    //else see if an object
    UMLObject * object = item->umlObject();
    //continue only if we are on a UMLObject
    if (!object) {
        return;
    }

    UMLObject::ObjectType type = object->baseType();
    int page = ClassPropDlg::page_gen;
    if (Model_Utils::isClassifierListitem(type)) {
        object = (UMLObject *)object->parent();
    }
    //set what page to show
    switch (type) {

    case UMLObject::ot_Attribute:
        page = ClassPropDlg::page_att;
        break;
    case UMLObject::ot_Operation:
        page = ClassPropDlg::page_op;
        break;
    case UMLObject::ot_EntityAttribute:
        page = ClassPropDlg::page_entatt;
        break;
    case UMLObject::ot_UniqueConstraint:
    case UMLObject::ot_ForeignKeyConstraint:
    case UMLObject::ot_CheckConstraint:
        page = ClassPropDlg::page_constraint;
        break;
    default:
        page = ClassPropDlg::page_gen;
        break;
    }

    if (object) {
        object->showPropertiesPagedDialog(page);
    }
//:TODO:delete?    item->cancelRename(0);  //double click can cause it to go into rename mode.
}

/**
 * Event handler for accepting drag request.
 * @param event   the drop event
 * @return success state
 */
bool UMLListView::acceptDrag(QDropEvent* event) const
{
    UMLListViewItem* item = (UMLListViewItem*)itemAt(event->pos());
    if (!item) {
        DEBUG(DBG_SRC) << "itemAt(mouse position) returns 0";
        return false;
    }
    ((QTreeWidget*)this)->setCurrentItem((QTreeWidgetItem*)item);

    UMLDragData::LvTypeAndID_List list;
    if (! UMLDragData::getClip3TypeAndID(event->mimeData(), list)) {
        DEBUG(DBG_SRC) << "UMLDragData::getClip3TypeAndID returns false";
        return false;
    }

    UMLDragData::LvTypeAndID_It it(list);
    UMLDragData::LvTypeAndID * data = 0;
    UMLListViewItem::ListViewType dstType = item->type();
    bool accept = true;
    while (accept && it.hasNext()) {
        data = it.next();
        UMLListViewItem::ListViewType srcType = data->type;
        switch (srcType) {
        case UMLListViewItem::lvt_Class:
        case UMLListViewItem::lvt_Package:
        case UMLListViewItem::lvt_Interface:
        case UMLListViewItem::lvt_Enum:
            if (dstType == UMLListViewItem::lvt_Logical_View ||
                    dstType == UMLListViewItem::lvt_Class ||
                    dstType == UMLListViewItem::lvt_Package) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == UMLListViewItem::lvt_Logical_Folder);
            }
            break;
        case UMLListViewItem::lvt_Attribute:
            if (dstType == UMLListViewItem::lvt_Class) {
                accept = !item->isOwnParent(data->id);
            }
            break;
        case UMLListViewItem::lvt_EntityAttribute:
            if (dstType == UMLListViewItem::lvt_Entity) {
                accept = !item->isOwnParent(data->id);
            }
            break;
        case UMLListViewItem::lvt_Operation:
            if (dstType == UMLListViewItem::lvt_Class ||
                    dstType == UMLListViewItem::lvt_Interface) {
                accept = !item->isOwnParent(data->id);
            }
            break;
        case UMLListViewItem::lvt_Datatype:
            accept = (dstType == UMLListViewItem::lvt_Logical_Folder ||
                      dstType == UMLListViewItem::lvt_Datatype_Folder ||
                      dstType == UMLListViewItem::lvt_Class ||
                      dstType == UMLListViewItem::lvt_Interface ||
                      dstType == UMLListViewItem::lvt_Package);
            break;
        case UMLListViewItem::lvt_Class_Diagram:
        case UMLListViewItem::lvt_Collaboration_Diagram:
        case UMLListViewItem::lvt_State_Diagram:
        case UMLListViewItem::lvt_Activity_Diagram:
        case UMLListViewItem::lvt_Sequence_Diagram:
            accept = (dstType == UMLListViewItem::lvt_Logical_Folder ||
                      dstType == UMLListViewItem::lvt_Logical_View);
            break;
        case UMLListViewItem::lvt_Logical_Folder:
            if (dstType == UMLListViewItem::lvt_Logical_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == UMLListViewItem::lvt_Logical_View);
            }
            break;
        case UMLListViewItem::lvt_UseCase_Folder:
            if (dstType == UMLListViewItem::lvt_UseCase_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == UMLListViewItem::lvt_UseCase_View);
            }
            break;
        case UMLListViewItem::lvt_Component_Folder:
            if (dstType == UMLListViewItem::lvt_Component_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == UMLListViewItem::lvt_Component_View);
            }
            break;
        case UMLListViewItem::lvt_Deployment_Folder:
            if (dstType == UMLListViewItem::lvt_Deployment_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == UMLListViewItem::lvt_Deployment_View);
            }
            break;
        case UMLListViewItem::lvt_EntityRelationship_Folder:
            if (dstType == UMLListViewItem::lvt_EntityRelationship_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == UMLListViewItem::lvt_EntityRelationship_Model);
            }
            break;
        case UMLListViewItem::lvt_Actor:
        case UMLListViewItem::lvt_UseCase:
        case UMLListViewItem::lvt_UseCase_Diagram:
            accept = (dstType == UMLListViewItem::lvt_UseCase_Folder ||
                      dstType == UMLListViewItem::lvt_UseCase_View);
            break;
        case UMLListViewItem::lvt_Subsystem:
            accept = (dstType == UMLListViewItem::lvt_Component_Folder ||
                      dstType == UMLListViewItem::lvt_Subsystem);
            break;
        case UMLListViewItem::lvt_Component:
            accept = (dstType == UMLListViewItem::lvt_Component_Folder ||
                      dstType == UMLListViewItem::lvt_Component ||
                      dstType == UMLListViewItem::lvt_Subsystem);
            break;
        case UMLListViewItem::lvt_Artifact:
        case UMLListViewItem::lvt_Component_Diagram:
            accept = (dstType == UMLListViewItem::lvt_Component_Folder ||
                      dstType == UMLListViewItem::lvt_Component_View);
            break;
        case UMLListViewItem::lvt_Node:
        case UMLListViewItem::lvt_Deployment_Diagram:
            accept = (dstType == UMLListViewItem::lvt_Deployment_Folder);
            break;
        case UMLListViewItem::lvt_Entity:
        case UMLListViewItem::lvt_EntityRelationship_Diagram:
        case UMLListViewItem::lvt_Category:
            accept = (dstType == UMLListViewItem::lvt_EntityRelationship_Folder);
            break;
        default:
            accept = false;
            break;
        }
    }

    //DEBUG(DBG_SRC) << "dstType = " << dstType << ", accept=" << accept;
    return accept;
}

/**
 * Auxiliary method for moveObject(): Adds the model object at the proper
 * new container (package if nested, UMLDoc if at global level), and
 * updates the containment relationships in the model.
 */
void UMLListView::addAtContainer(UMLListViewItem *item, UMLListViewItem *parent)
{
    UMLCanvasObject *o = static_cast<UMLCanvasObject*>(item->umlObject());
    if (o == 0) {
        DEBUG(DBG_SRC) << item->text(0) << ": item's UMLObject is 0";
    } else if (Model_Utils::typeIsContainer(parent->type())) {
        /**** TBC: Do this here?
                   If yes then remove that logic at the callers
                   and rename this method to moveAtContainer()
        UMLPackage *oldPkg = o->getUMLPackage();
        if (oldPkg)
            oldPkg->removeObject(o);
         *********/
        UMLPackage *pkg = static_cast<UMLPackage*>(parent->umlObject());
        o->setUMLPackage(pkg);
        pkg->addObject(o);
    } else {
        uError() << item->text(0) << ": parent type is " << parent->type();
    }
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView)
        currentView->updateContainment(o);
}

/**
 * Moves an object given is unique ID and listview type to an
 * other listview parent item.
 * Also takes care of the corresponding move in the model.
 */
UMLListViewItem * UMLListView::moveObject(Uml::IDType srcId, UMLListViewItem::ListViewType srcType,
        UMLListViewItem *newParent)
{
    if (newParent == 0)
        return 0;
    UMLListViewItem * move = findItem(srcId);
    if (move == 0)
        return 0;

    UMLObject *newParentObj = 0;
    // Remove the source object at the old parent package.
    UMLObject *srcObj = m_doc->findObjectById(srcId);
    if (srcObj) {
        newParentObj = newParent->umlObject();
        if (srcObj == newParentObj) {
            uError() << srcObj->name() << ": Cannot move onto self";
            return 0;
        }
        UMLPackage *srcPkg = srcObj->umlPackage();
        if (srcPkg) {
            if (srcPkg == newParentObj) {
                uError() << srcObj->name() << ": Object is already in target package";
                return 0;
            }
            srcPkg->removeObject(srcObj);
        }
    }

    UMLListViewItem::ListViewType newParentType = newParent->type();
    DEBUG(DBG_SRC) << "newParentType is " << newParentType;
    UMLListViewItem *newItem = 0;

    //make sure trying to place in correct location
    switch (srcType) {
    case UMLListViewItem::lvt_UseCase_Folder:
    case UMLListViewItem::lvt_Actor:
    case UMLListViewItem::lvt_UseCase:
    case UMLListViewItem::lvt_UseCase_Diagram:
        if (newParentType == UMLListViewItem::lvt_UseCase_Folder ||
                newParentType == UMLListViewItem::lvt_UseCase_View) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case UMLListViewItem::lvt_Component_Folder:
    case UMLListViewItem::lvt_Artifact:
    case UMLListViewItem::lvt_Component_Diagram:
        if (newParentType == UMLListViewItem::lvt_Component_Folder ||
                newParentType == UMLListViewItem::lvt_Component_View) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case UMLListViewItem::lvt_Subsystem:
        if (newParentType == UMLListViewItem::lvt_Component_Folder ||
                newParentType == UMLListViewItem::lvt_Component_View ||
                newParentType == UMLListViewItem::lvt_Subsystem) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case UMLListViewItem::lvt_Component:
        if (newParentType == UMLListViewItem::lvt_Component_Folder ||
                newParentType == UMLListViewItem::lvt_Component_View ||
                newParentType == UMLListViewItem::lvt_Component ||
                newParentType == UMLListViewItem::lvt_Subsystem) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case UMLListViewItem::lvt_Deployment_Folder:
    case UMLListViewItem::lvt_Node:
    case UMLListViewItem::lvt_Deployment_Diagram:
        if (newParentType == UMLListViewItem::lvt_Deployment_Folder ||
                newParentType == UMLListViewItem::lvt_Deployment_View) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case UMLListViewItem::lvt_EntityRelationship_Folder:
    case UMLListViewItem::lvt_Entity:
    case UMLListViewItem::lvt_Category:
    case UMLListViewItem::lvt_EntityRelationship_Diagram:
        if (newParentType == UMLListViewItem::lvt_EntityRelationship_Folder ||
                newParentType == UMLListViewItem::lvt_EntityRelationship_Model) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case UMLListViewItem::lvt_Collaboration_Diagram:
    case UMLListViewItem::lvt_Class_Diagram:
    case UMLListViewItem::lvt_State_Diagram:
    case UMLListViewItem::lvt_Activity_Diagram:
    case UMLListViewItem::lvt_Sequence_Diagram:
    case UMLListViewItem::lvt_Logical_Folder:
        if (newParentType == UMLListViewItem::lvt_Logical_Folder ||
                newParentType == UMLListViewItem::lvt_Logical_View) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case UMLListViewItem::lvt_Class:
    case UMLListViewItem::lvt_Package:
    case UMLListViewItem::lvt_Interface:
    case UMLListViewItem::lvt_Enum:
    case UMLListViewItem::lvt_Datatype:
        if (newParentType == UMLListViewItem::lvt_Logical_Folder ||
                newParentType == UMLListViewItem::lvt_Datatype_Folder ||
                newParentType == UMLListViewItem::lvt_Logical_View ||
                newParentType == UMLListViewItem::lvt_Class ||
                newParentType == UMLListViewItem::lvt_Interface ||
                newParentType == UMLListViewItem::lvt_Package) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            UMLCanvasObject *o = static_cast<UMLCanvasObject*>(newItem->umlObject());
            if (o == 0) {
                DEBUG(DBG_SRC) << "moveObject: newItem's UMLObject is 0";
            } else if (newParentObj == 0) {
                uError() << o->name() << ": newParentObj is 0";
            } else {
                UMLPackage *pkg = static_cast<UMLPackage*>(newParentObj);
                o->setUMLPackage(pkg);
                pkg->addObject(o);
            }
            UMLView *currentView = UMLApp::app()->currentView();
            if (currentView)
                currentView->updateContainment(o);
        }
        break;
    case UMLListViewItem::lvt_Attribute:
    case UMLListViewItem::lvt_Operation:
        if (newParentType == UMLListViewItem::lvt_Class ||
                newParentType == UMLListViewItem::lvt_Interface) {
            // update list view

            newItem = move->deepCopy(newParent);
            // we don't delete move right away, it will be deleted in slots,
            // called by subsequent steps
            //delete move;

            // update model objects
            m_bCreatingChildObject = true;

            UMLClassifier *oldParentClassifier = dynamic_cast<UMLClassifier*>(srcObj->parent());
            UMLClassifier *newParentClassifier = dynamic_cast<UMLClassifier*>(newParentObj);
            if (srcType == UMLListViewItem::lvt_Attribute) {
                UMLAttribute *att = dynamic_cast<UMLAttribute*>(srcObj);
                // We can't use the existing 'att' directly
                // because its parent is fixed to the old classifier
                // and we have no way of changing that:
                // QObject does not permit changing the parent().
                if (att == 0) {
                    uError() << "moveObject internal error: srcObj "
                        << srcObj->name() << " is not a UMLAttribute";
                } else if (oldParentClassifier->takeItem(att) == -1) {
                    uError() << "moveObject: oldParentClassifier->takeItem(att "
                        << att->name() << ") returns 0";
                } else {
                    const QString& nm = att->name();
                    UMLAttribute *newAtt = newParentClassifier->createAttribute(nm,
                                           att->getType(),
                                           att->visibility(),
                                           att->getInitialValue());
                    newItem->setUMLObject(newAtt);
                    newParent->addClassifierListItem(newAtt, newItem);

                    connectNewObjectsSlots(newAtt);
                    // Let's not forget to update the DocWindow::m_pObject
                    // because the old one is about to be physically deleted !
                    UMLApp::app()->docWindow()->showDocumentation(newAtt, true);
                    delete att;
                }
            } else {
                UMLOperation *op = dynamic_cast<UMLOperation*>(srcObj);
                // We can't use the existing 'op' directly
                // because its parent is fixed to the old classifier
                // and we have no way of changing that:
                // QObject does not permit changing the parent().
                if (op && oldParentClassifier->takeItem(op) != -1) {
                    bool isExistingOp;
                    Model_Utils::NameAndType_List ntDummyList;
                    // We need to provide a dummy NameAndType_List
                    // else UMLClassifier::createOperation will
                    // bring up an operation dialog.
                    UMLOperation *newOp = newParentClassifier->createOperation(
                                              op->name(), &isExistingOp, &ntDummyList);
                    newOp->setType(op->getType());
                    newOp->setVisibility(op->visibility());
                    UMLAttributeList parmList = op->getParmList();
                    foreach(UMLAttribute* parm, parmList) {
                        UMLAttribute *newParm = new UMLAttribute(newParentClassifier,
                                parm->name(),
                                Uml::id_None,
                                parm->visibility(),
                                parm->getType(),
                                parm->getInitialValue());
                        newParm->setParmKind(parm->getParmKind());
                        newOp->addParm(newParm);
                    }
                    newItem->setUMLObject(newOp);
                    newParent->addClassifierListItem(newOp, newItem);

                    connectNewObjectsSlots(newOp);

                    // Let's not forget to update the DocWindow::m_pObject
                    // because the old one is about to be physically deleted !
                    UMLApp::app()->docWindow()->showDocumentation(newOp, true);
                    delete op;
                } else {
                    uError() << "moveObject: oldParentClassifier->takeItem(op) returns 0";
                }
            }
            m_bCreatingChildObject = false;
        }
        break;
    default:
        break;
    }
    return newItem;
}

/**
 * Something has been dragged and dropped onto the list view.
 */
void UMLListView::slotDropped(QDropEvent* de, UMLListViewItem* parent, UMLListViewItem* item)
{
    Q_UNUSED(parent);
    item = (UMLListViewItem *)currentItem();
    if (!item) {
        DEBUG(DBG_SRC) << "item is 0 - doing nothing";
        return;
    }
    UMLDragData::LvTypeAndID_List srcList;
    if (! UMLDragData::getClip3TypeAndID(de->mimeData(), srcList)) {
        return;
    }
    UMLListViewItem *newParent = (UMLListViewItem*)item;
    DEBUG(DBG_SRC) << "slotDropped: newParent->text(0) is " << newParent->text(0);
    UMLDragData::LvTypeAndID_It it(srcList);
    UMLDragData::LvTypeAndID * src = 0;
    while (it.hasNext()) {
        src = it.next();
        moveObject(src->id, src->type, newParent);
    }
}

/**
 * Get selected items.
 * @return   the list of selected items
 */
UMLListViewItemList UMLListView::selectedItems()
{
    UMLListViewItemList itemList;
    UMLListViewItemIterator it(this);
    // iterate through all items of the list view
    for (; *it; ++it) {
        if ((*it)->isSelected()) {
            UMLListViewItem *item = (UMLListViewItem*)*it;
            itemList.append(item);
        }
    }
    // DEBUG(DBG_SRC) << "selected items = " << itemList.count();

    return itemList;
}

/**
 * Get selected items, but only root elements selected (without children).
 * @return   the list of selected root items
 */
UMLListViewItemList UMLListView::selectedItemsRoot()
{
    UMLListViewItemList itemList;
    QTreeWidgetItemIterator it(this);

    // iterate through all items of the list view
    for (; *it; ++it) {
        if ((*it)->isSelected()) {
            UMLListViewItem *item = (UMLListViewItem*)*it;
            // this is the trick, we select only the item with a parent unselected
            // since we can't select a child and its grandfather without its parent
            // we would be able to delete each item individually, without an invalid iterator
            if (item && item->parent() && item->parent()->isSelected() == false) {
                itemList.append(item);
            }
        }
    }

    return itemList;
}

/**
 * Create a listview item for an existing diagram.
 *
 * @param view   The existing diagram.
 */
UMLListViewItem* UMLListView::createDiagramItem(UMLView *view)
{
    if (!view) {
        return 0;
    }
    UMLListViewItem::ListViewType lvt = Model_Utils::convert_DT_LVT(view->umlScene()->type());
    UMLListViewItem *parent = 0;
    UMLFolder *f = view->umlScene()->folder();
    if (f) {
        parent = findUMLObject(f);
        if (parent == 0)
            uError() << view->umlScene()->name() << ": findUMLObject(" << f->name() << ") returns 0";
    } else {
        DEBUG(DBG_SRC) << view->umlScene()->name() << ": no parent folder set, using predefined folder";
    }
    if (parent == 0) {
        parent = determineParentItem(lvt);
        lvt = Model_Utils::convert_DT_LVT(view->umlScene()->type());
    }
    UMLListViewItem *item = new UMLListViewItem(parent, view->umlScene()->name(), lvt, view->umlScene()->getID());
    return item;
}

/**
 * CHECK - This is perhaps redundant since the
 *         UMLListViewItemData => UMLListViewItem merge.
 * Creates a new UMLListViewItem from a UMLListViewItem, if
 * parent is null the ListView decides who is going to be the parent.
 */
UMLListViewItem* UMLListView::createItem(UMLListViewItem& Data, IDChangeLog& IDChanges,
        UMLListViewItem* parent /*= 0*/)
{
    UMLObject* pObject = 0;
    UMLListViewItem* item = 0;
    UMLListViewItem::ListViewType lvt = Data.type();
    if (!parent) {
        parent = determineParentItem(lvt);
        if (!parent)
            return 0;
    }

    switch (lvt) {
    case UMLListViewItem::lvt_Actor:
    case UMLListViewItem::lvt_UseCase:
    case UMLListViewItem::lvt_Class:
    case UMLListViewItem::lvt_Package:
    case UMLListViewItem::lvt_Subsystem:
    case UMLListViewItem::lvt_Component:
    case UMLListViewItem::lvt_Node:
    case UMLListViewItem::lvt_Artifact:
    case UMLListViewItem::lvt_Interface:
    case UMLListViewItem::lvt_Datatype:
    case UMLListViewItem::lvt_Enum:
    case UMLListViewItem::lvt_Entity:
    case UMLListViewItem::lvt_Category:
    case UMLListViewItem::lvt_Logical_Folder:
    case UMLListViewItem::lvt_UseCase_Folder:
    case UMLListViewItem::lvt_Component_Folder:
    case UMLListViewItem::lvt_Deployment_Folder:
    case UMLListViewItem::lvt_EntityRelationship_Folder:
        /***
        int newID = IDChanges.findNewID(Data.getID());
        //if there is no ListViewItem associated with the new ID,
        //it could exist an Item already asocciated if the user chose to reuse an uml object
        if(!(item = findItem(newID))) {
                pObject = m_doc->findObjectById( IDChanges.findNewID(Data.getID()) );
                item = new UMLListViewItem(parent, Data.text(0), lvt, pObject);
        } ***/
        pObject = m_doc->findObjectById(Data.getID());
        item = new UMLListViewItem(parent, Data.text(0), lvt, pObject);
        break;
    case UMLListViewItem::lvt_Datatype_Folder:
        item = new UMLListViewItem(parent, Data.text(0), lvt);
        break;
    case UMLListViewItem::lvt_Attribute:
    case UMLListViewItem::lvt_EntityAttribute:
    case UMLListViewItem::lvt_Operation:
    case UMLListViewItem::lvt_Template:
    case UMLListViewItem::lvt_EnumLiteral:
    case UMLListViewItem::lvt_UniqueConstraint:
    case UMLListViewItem::lvt_PrimaryKeyConstraint:
    case UMLListViewItem::lvt_ForeignKeyConstraint:
    case UMLListViewItem::lvt_CheckConstraint: {
        UMLClassifier *pClass =  static_cast<UMLClassifier*>(parent->umlObject());
        Uml::IDType newID = IDChanges.findNewID(Data.getID());
        pObject = pClass->findChildObjectById(newID);
        if (pObject) {
            item = new UMLListViewItem(parent, Data.text(0), lvt, pObject);
        } else {
            item = 0;
        }
        break;
    }
    case UMLListViewItem::lvt_UseCase_Diagram:
    case UMLListViewItem::lvt_Sequence_Diagram:
    case UMLListViewItem::lvt_Collaboration_Diagram:
    case UMLListViewItem::lvt_Class_Diagram:
    case UMLListViewItem::lvt_State_Diagram:
    case UMLListViewItem::lvt_Activity_Diagram:
    case UMLListViewItem::lvt_Component_Diagram:
    case UMLListViewItem::lvt_Deployment_Diagram:
    case UMLListViewItem::lvt_EntityRelationship_Diagram: {
        Uml::IDType newID = IDChanges.findNewID(Data.getID());
        UMLView* v = m_doc->findView(newID);
        if (v == 0) {
            return 0;
        }
        const UMLListViewItem::ListViewType lvt = Model_Utils::convert_DT_LVT(v->umlScene()->type());
        item = new UMLListViewItem(parent, v->umlScene()->name(), lvt, newID);
    }
    break;
    default:
        uWarning() << "createItem() called on unknown type";
        break;
    }
    return item;
}

/**
 * Determine the parent ListViewItem given a ListViewType.
 * This parent is used for creating new UMLListViewItems.
 *
 * @param lvt   The ListViewType for which to lookup the parent.
 * @return  Pointer to the parent UMLListViewItem chosen.
 */
UMLListViewItem* UMLListView::determineParentItem(UMLListViewItem::ListViewType lvt) const
{
    UMLListViewItem* parent = 0;
    switch (lvt) {
    case UMLListViewItem::lvt_Datatype:
        parent = m_datatypeFolder;
        break;
    case UMLListViewItem::lvt_Actor:
    case UMLListViewItem::lvt_UseCase:
    case UMLListViewItem::lvt_UseCase_Folder:
    case UMLListViewItem::lvt_UseCase_Diagram:
        parent = m_lv[Uml::ModelType::UseCase];
        break;
    case UMLListViewItem::lvt_Component_Diagram:
    case UMLListViewItem::lvt_Component:
    case UMLListViewItem::lvt_Artifact:
        parent = m_lv[Uml::ModelType::Component];
        break;
    case UMLListViewItem::lvt_Deployment_Diagram:
    case UMLListViewItem::lvt_Node:
        parent = m_lv[Uml::ModelType::Deployment];
        break;
    case UMLListViewItem::lvt_EntityRelationship_Diagram:
    case UMLListViewItem::lvt_Entity:
    case UMLListViewItem::lvt_Category:
        parent = m_lv[Uml::ModelType::EntityRelationship];
        break;
    default:
        if (Model_Utils::typeIsDiagram(lvt) || !Model_Utils::typeIsClassifierList(lvt))
            parent = m_lv[Uml::ModelType::Logical];
        break;
    }
    return parent;
}

/**
 *  Return the amount of items selected.
 */
int UMLListView::selectedItemsCount()
{
    UMLListViewItemList items = selectedItems();
    return items.count();
}

/**
 * Returns the document pointer. Called by the UMLListViewItem class.
 */
UMLDoc * UMLListView::document() const
{
    return m_doc;
}

/**
 * Event handler for lost focus.
 * @param fe   the focus event
 */
void UMLListView::focusOutEvent(QFocusEvent * fe)
{
    Qt::FocusReason reason = fe->reason();
    if (reason != Qt::PopupFocusReason && reason != Qt::MouseFocusReason) {
        clearSelection();
        //triggerUpdate();
    }
    //repaint();

    QTreeWidget::focusOutEvent(fe);
}

/**
 * Determines the root listview type of the given UMLListViewItem.
 * Starts at the given item, compares it against each of the
 * predefined root views (Root, Logical, UseCase, Component,
 * Deployment, EntityRelationship.) Returns the ListViewType
 * of the matching root view; if no match then continues the
 * search using the item's parent, then grandparent, and so forth.
 * Returns UMLListViewItem::lvt_Unknown if no match at all is found.
 */
UMLListViewItem::ListViewType UMLListView::rootViewType(UMLListViewItem *item)
{
    if (item == m_rv)
        return UMLListViewItem::lvt_View;
    if (item == m_lv[Uml::ModelType::Logical])
        return UMLListViewItem::lvt_Logical_View;
    if (item == m_lv[Uml::ModelType::UseCase])
        return UMLListViewItem::lvt_UseCase_View;
    if (item == m_lv[Uml::ModelType::Component])
        return UMLListViewItem::lvt_Component_View;
    if (item == m_lv[Uml::ModelType::Deployment])
        return UMLListViewItem::lvt_Deployment_View;
    if (item == m_lv[Uml::ModelType::EntityRelationship])
        return UMLListViewItem::lvt_EntityRelationship_Model;
    UMLListViewItem *parent = dynamic_cast<UMLListViewItem*>(item->parent());
    if (parent)
        return rootViewType(parent);
    return UMLListViewItem::lvt_Unknown;
}

/**
 * Return true if the given list view type can be expanded/collapsed.
 */
bool UMLListView::isExpandable(UMLListViewItem::ListViewType lvt)
{
    if (Model_Utils::typeIsRootView(lvt) || Model_Utils::typeIsFolder(lvt))
        return true;
    switch (lvt) {
    case UMLListViewItem::lvt_Package:
    case UMLListViewItem::lvt_Component:
    case UMLListViewItem::lvt_Subsystem:
        return true;
        break;
    default:
        break;
    }
    return false;
}

/**
 * Calls updateFolder() on the item to update the icon to open.
 */
void UMLListView::slotExpanded(QTreeWidgetItem * item)
{
    UMLListViewItem * myItem = static_cast<UMLListViewItem*>(item);
    if (isExpandable(myItem->type())) {
        myItem->updateFolder();
    }
}

/**
 * Calls updateFolder() on the item to update the icon to closed.
 */
void UMLListView::slotCollapsed(QTreeWidgetItem * item)
{
    UMLListViewItem * myItem = static_cast<UMLListViewItem*>(item);
    if (isExpandable(myItem->type())) {
        myItem->updateFolder();
    }
}

/**
 *  Connects to the signal that @ref UMLApp emits when a
 *  cut operation is successful.
 */
void UMLListView::slotCutSuccessful()
{
    if (m_bStartedCut) {
        popupMenuSel(m_menu->getAction(ListPopupMenu::mt_Delete));
        //deletion code here
        m_bStartedCut = false;
    }
}

/**
 * TODO: still in use?
 */
void UMLListView::startUpdate()
{
    setSortingEnabled(false);
}

/**
 * TODO: still in use?
 */
void UMLListView::endUpdate()
{
    setSortingEnabled(true);
}

/**
 * Adds a new item to the tree of the given type under the given parent.
 * Method will take care of signalling anyone needed on creation of new item.
 * e.g. UMLDoc if an UMLObject is created.
 */
void UMLListView::addNewItem(UMLListViewItem *parentItem, UMLListViewItem::ListViewType type)
{
    if (type == UMLListViewItem::lvt_Datatype) {
        parentItem = m_datatypeFolder;
    }

    UMLListViewItem * newItem = 0;
    parentItem->setOpen(true);

    Icon_Utils::IconType icon = Model_Utils::convert_LVT_IT(type);

    QString name;
    if (Model_Utils::typeIsDiagram(type)) {
        Uml::DiagramType dt = Model_Utils::convert_LVT_DT(type);
        name = uniqueDiagramName(dt);
        newItem = new UMLListViewItem(parentItem, name, type, Uml::id_None);
    } else {
        UMLObject::ObjectType ot = Model_Utils::convert_LVT_OT(type);
        if (ot == UMLObject::ot_UMLObject) {
            DEBUG(DBG_SRC) << "no UMLObject for type " << UMLListViewItem::toString(type);
            return;
        }
        UMLPackage *parentPkg =
            dynamic_cast<UMLPackage*>(parentItem->umlObject());
        if (parentPkg == 0) {
            uError() << "UMLListView::addNewItem - "
                     << UMLListViewItem::toString(type) << ": parentPkg is 0";
            return;
        }
        if (Model_Utils::typeIsClassifierList(type)) {
            UMLClassifier *parent = static_cast<UMLClassifier*>(parentPkg);
            name = parent->uniqChildName(ot);
        } else {
            name = Model_Utils::uniqObjectName(ot, parentPkg);
        }
        newItem = new UMLListViewItem(parentItem, name, type, (UMLObject *)0);
        createItem(newItem, type);
    }
    newItem->setIcon(icon);
    newItem->setOpen(true);
    newItem->setCreating(true);
}

/**
 * Called for informing the list view that an item was renamed.
 */
bool UMLListView::itemRenamed(UMLListViewItem * item, int col)
{
    DEBUG(DBG_SRC) << item->text(col);
    UMLListViewItem * renamedItem = static_cast< UMLListViewItem *>(item) ;
    UMLListViewItem::ListViewType type = renamedItem->type();
    QString newText = renamedItem->text(col);
    renamedItem->setCreating(false);

    // If the type is empty then delete it.
    if (newText.isEmpty() || newText.contains(QRegExp("^\\s+$"))) {
        KMessageBox::error(
            0,
            i18n("The name you entered was invalid.\nCreation process has been canceled."),
            i18n("Name Not Valid"));
        return false;
    }

    if (!isUnique(renamedItem, newText)) {
        //if operation ask if ok not to be unique i.e overloading
        if (type == UMLListViewItem::lvt_Operation) {
            if (KMessageBox::warningYesNo(
                        0,
                        i18n("The name you entered was not unique.\nIs this what you wanted?"),
                        i18n("Name Not Unique"), KGuiItem(i18n("Use Name")), KGuiItem(i18n("Enter New Name"))) == KMessageBox::No) {
                return false;
            }
        } else {
            KMessageBox::error(
                0,
                i18n("The name you entered was not unique.\nCreation process has been canceled."),
                i18n("Name Not Unique"));
            return false;
        }
    }
    return createItem(renamedItem, type);
}

bool UMLListView::createItem(UMLListViewItem *item, UMLListViewItem::ListViewType type)
{
    switch (type) {
    case UMLListViewItem::lvt_Actor:
    case UMLListViewItem::lvt_Class:
    case UMLListViewItem::lvt_Package:
    case UMLListViewItem::lvt_Logical_Folder:
    case UMLListViewItem::lvt_UseCase_Folder:
    case UMLListViewItem::lvt_Component_Folder:
    case UMLListViewItem::lvt_Deployment_Folder:
    case UMLListViewItem::lvt_EntityRelationship_Folder:
    case UMLListViewItem::lvt_Subsystem:
    case UMLListViewItem::lvt_Component:
    case UMLListViewItem::lvt_Node:
    case UMLListViewItem::lvt_Artifact:
    case UMLListViewItem::lvt_Interface:
    case UMLListViewItem::lvt_Datatype:
    case UMLListViewItem::lvt_Enum:
    case UMLListViewItem::lvt_Entity:
    case UMLListViewItem::lvt_UseCase:
    case UMLListViewItem::lvt_Category: {
        UMLObject::ObjectType ot = Model_Utils::convert_LVT_OT(type);
        if (! ot) {
            uError() << "internal error";
            return false;
        }
        UMLObject *o = createUMLObject(item, ot);
        if (type == UMLListViewItem::lvt_Subsystem)
            o->setStereotype("subsystem");
        else if (Model_Utils::typeIsFolder(type))
            o->setStereotype("folder");
    }
    break;

    case UMLListViewItem::lvt_Attribute:
    case UMLListViewItem::lvt_EntityAttribute:
    case UMLListViewItem::lvt_Operation:
    case UMLListViewItem::lvt_Template:
    case UMLListViewItem::lvt_EnumLiteral:
    case UMLListViewItem::lvt_UniqueConstraint:
    case UMLListViewItem::lvt_ForeignKeyConstraint:
    case UMLListViewItem::lvt_CheckConstraint:

        return createChildUMLObject(item, Model_Utils::convert_LVT_OT(type));
        break;

    case UMLListViewItem::lvt_PrimaryKeyConstraint: {

        bool result = createChildUMLObject(item, Model_Utils::convert_LVT_OT(type));
        UMLObject* obj = item->umlObject();
        UMLUniqueConstraint* uuc = static_cast<UMLUniqueConstraint*>(obj);
        UMLEntity* ent = static_cast<UMLEntity*>(uuc->parent());
        if (ent)
            ent->setAsPrimaryKey(uuc);

        return result;
    }
    break;

    case UMLListViewItem::lvt_Class_Diagram:
        createDiagram(item, Uml::DiagramType::Class);
        break;

    case UMLListViewItem::lvt_UseCase_Diagram:
        createDiagram(item, Uml::DiagramType::UseCase);
        break;

    case UMLListViewItem::lvt_Sequence_Diagram:
        createDiagram(item, Uml::DiagramType::Sequence);
        break;

    case UMLListViewItem::lvt_Collaboration_Diagram:
        createDiagram(item, Uml::DiagramType::Collaboration);
        break;

    case UMLListViewItem::lvt_State_Diagram:
        createDiagram(item, Uml::DiagramType::State);
        break;

    case UMLListViewItem::lvt_Activity_Diagram:
        createDiagram(item, Uml::DiagramType::Activity);
        break;

    case UMLListViewItem::lvt_Component_Diagram:
        createDiagram(item, Uml::DiagramType::Component);
        break;

    case UMLListViewItem::lvt_Deployment_Diagram:
        createDiagram(item, Uml::DiagramType::Deployment);
        break;

    case UMLListViewItem::lvt_EntityRelationship_Diagram:
        createDiagram(item, Uml::DiagramType::EntityRelationship);
        break;

    default:
        break;
    }
    return true;
}

/**
 * Creates a UMLObject out of the given list view item.
 */
UMLObject *UMLListView::createUMLObject(UMLListViewItem * item, UMLObject::ObjectType type)
{
    QString name = item->text(0);
    UMLObject * object = 0;
    switch (type) {
    case UMLObject::ot_UseCase:
        object = new UMLUseCase(name);
        break;

    case UMLObject::ot_Actor:
        object = new UMLActor(name);
        break;

    case UMLObject::ot_Class:
        object = new UMLClassifier(name);
        break;

    case UMLObject::ot_Package:
        object = new UMLPackage(name);
        break;

    case UMLObject::ot_Folder:
        object = new UMLFolder(name);
        break;

    case UMLObject::ot_Component:
        object = new UMLComponent(name);
        break;

    case UMLObject::ot_Node:
        object = new UMLNode(name);
        break;

    case UMLObject::ot_Artifact:
        object = new UMLArtifact(name);
        break;

    case UMLObject::ot_Interface: {
        UMLClassifier *c = new UMLClassifier(name);
        c->setBaseType(UMLObject::ot_Interface);
        object = c;
    }
    break;

    case UMLObject::ot_Datatype: {
        UMLClassifier *c = new UMLClassifier(name);
        c->setBaseType(UMLObject::ot_Datatype);
        object = c;
    }
    break;

    case UMLObject::ot_Enum:
        object = new UMLEnum(name);
        break;

    case UMLObject::ot_Entity:
        object = new UMLEntity(name);
        break;

    case UMLObject::ot_Category:
        object = new UMLCategory(name);
        break;

    default:
        uWarning() << "creating UML Object of unknown type";
        return 0;
    }

    UMLListViewItem * parentItem = static_cast<UMLListViewItem *>(item->parent());
    const UMLListViewItem::ListViewType lvt = parentItem->type();
    if (! Model_Utils::typeIsContainer(lvt)) {
        uError() << object->name() << ": parentItem (" << lvt << " is not a container";
        delete object;
        return 0;
    }
    UMLPackage *pkg = static_cast<UMLPackage*>(parentItem->umlObject());
    object->setUMLPackage(pkg);
    pkg->addObject(object);
    connectNewObjectsSlots(object);
    item->setUMLObject(object);
    item->setText(name);
    return object;
}

/**
 * Creates a child UMLObject out of the given list view item.
 */
bool UMLListView::createChildUMLObject(UMLListViewItem * item, UMLObject::ObjectType type)
{
    m_bCreatingChildObject = true;
    QString text = item->text(0);
    UMLObject* parent = static_cast<UMLListViewItem *>(item->parent())->umlObject();
    if (!parent) {
        uError() << "parent UMLObject is 0";
        m_bCreatingChildObject = false;
        return false;
    }

    UMLObject* newObject = 0;
    if (type == UMLObject::ot_EnumLiteral) {
        UMLEnum *owningEnum = static_cast<UMLEnum*>(parent);
        newObject = owningEnum->createEnumLiteral(text);

        UMLEnumLiteral* enumLiteral = static_cast<UMLEnumLiteral*>(newObject);
        text = enumLiteral->toString(Uml::SignatureType::SigNoVis);
    } else if (type == UMLObject::ot_Template)  {
        UMLClassifier *owningClassifier = static_cast<UMLClassifier*>(parent);
        Model_Utils::NameAndType nt;
        Model_Utils::Parse_Status st = Model_Utils::parseTemplate(text, nt, owningClassifier);
        if (st) {
            KMessageBox::error(0,
                               Model_Utils::psText(st),
                               i18n("Creation canceled"));
            m_bCreatingChildObject = false;
            return false;
        }
        newObject = owningClassifier->createTemplate(nt.m_name);
        UMLTemplate *tmplParm = static_cast<UMLTemplate*>(newObject);
        tmplParm->setType(nt.m_type);
        text = tmplParm->toString(Uml::SignatureType::SigNoVis);
    } else if (type == UMLObject::ot_Attribute || type == UMLObject::ot_EntityAttribute)  {
        UMLClassifier *owningClass = static_cast<UMLClassifier*>(parent);
        Model_Utils::NameAndType nt;
        Uml::Visibility vis;
        Model_Utils::Parse_Status st;
        st = Model_Utils::parseAttribute(text, nt, owningClass, &vis);
        if (st) {
            KMessageBox::error(0,
                               Model_Utils::psText(st),
                               i18n("Creation canceled"));
            m_bCreatingChildObject = false;
            return false;
        }
        newObject = owningClass->createAttribute(nt.m_name, nt.m_type, vis, nt.m_initialValue);
        UMLAttribute *att = static_cast<UMLAttribute*>(newObject);
        att->setParmKind(nt.m_direction);
        text = att->toString(Uml::SignatureType::SigNoVis);
    } else if (type == UMLObject::ot_Operation) {
        UMLClassifier *owningClassifier = static_cast<UMLClassifier*>(parent);
        Model_Utils::OpDescriptor od;
        Model_Utils::Parse_Status st = Model_Utils::parseOperation(text, od, owningClassifier);
        if (st) {
            KMessageBox::error(0,
                               Model_Utils::psText(st),
                               i18n("Creation canceled"));
            m_bCreatingChildObject = false;
            return false;
        }
        bool isExistingOp = false;
        newObject = owningClassifier->createOperation(od.m_name, &isExistingOp, &od.m_args);
        if (newObject == 0 || isExistingOp) {
            if (isExistingOp)
                KMessageBox::error(
                    0,
                    i18n("The name you entered was not unique.\nCreation process has been canceled."),
                    i18n("Name Not Unique"));
            m_bCreatingChildObject = false;
            return false;
        }
        UMLOperation *op = static_cast<UMLOperation*>(newObject);
        if (od.m_pReturnType) {
            op->setType(od.m_pReturnType);
        }
        text = op->toString(Uml::SignatureType::SigNoVis);
    } else if (type == UMLObject::ot_UniqueConstraint || type == UMLObject::ot_ForeignKeyConstraint
               || type == UMLObject::ot_CheckConstraint) {

        UMLEntity *owningEntity = static_cast<UMLEntity*>(parent);

        QString name;
        Model_Utils::Parse_Status st = Model_Utils::parseConstraint(text, name, owningEntity);
        if (st) {
            KMessageBox::error(0,
                               Model_Utils::psText(st),
                               i18n("Creation canceled"));
            m_bCreatingChildObject = false;
            return false;
        }

        switch (type) {
        case UMLObject::ot_UniqueConstraint:
            newObject = owningEntity->createUniqueConstraint(name);
            break;
        case UMLObject::ot_ForeignKeyConstraint:
            newObject = owningEntity->createForeignKeyConstraint(name);
            break;
        case UMLObject::ot_CheckConstraint:
            newObject = owningEntity->createCheckConstraint(name);
            break;
        default:
            break;
        }

        UMLEntityConstraint* uec = static_cast<UMLEntityConstraint*>(newObject);

        text = uec->toString(Uml::SignatureType::SigNoVis);
    } else  {
        uError() << "called for type " << type << " (ignored)";
        m_bCreatingChildObject = false;
        return false;
    }

    // make changes to the object visible to this umllistviewitem
    connectNewObjectsSlots(newObject);
    item->setUMLObject(newObject);
    item->setText(text);
    scrollToItem(item);

    // as it's a ClassifierListItem add it to the childObjectMap of the parent
    UMLClassifierListItem* classifierListItem = static_cast<UMLClassifierListItem*>(newObject);
    static_cast<UMLListViewItem*>(item->parent())->addClassifierListItem(classifierListItem, item);

    m_bCreatingChildObject = false;

    if (! m_doc->loading())
        m_doc->setModified();
    return true;
}

/**
 * Creates a diagram out of the given list view item.
 */
UMLView* UMLListView::createDiagram(UMLListViewItem * item, Uml::DiagramType type)
{
    QString name = item->text(0);
    DEBUG(DBG_SRC) << name << " / type=" << type.toString();
    UMLView * view = m_doc->findView(type, name);
    if (view) {
        delete item;
        return view;
    }
    UMLListViewItem *parentItem = static_cast<UMLListViewItem*>(item->parent());
    UMLFolder *parentFolder = dynamic_cast<UMLFolder*>(parentItem->umlObject());
    if (parentFolder == 0) {
        uError() << name << ": parent UMLObject is not a UMLFolder";
        delete item;
        return 0;
    }
    view = new UMLView(parentFolder);
    view->umlScene()->setName(name);
    view->umlScene()->setType(type);
    view->umlScene()->setID(UniqueID::gen());
    m_doc->addView(view);
    view->umlScene()->setOptionState(Settings::optionState());
    item->setID(view->umlScene()->getID());
    item->setText(0, name);
    view->umlScene()->activate();
    m_doc->changeCurrentView(view->umlScene()->getID());

    return view;
}

/**
 * Returns a unique name for a diagram.
 */
QString UMLListView::uniqueDiagramName(Uml::DiagramType type)
{
    return m_doc->uniqueViewName(type);
}

/**
 * Returns if the given name is unique for the given items type.
 */
bool UMLListView::isUnique(UMLListViewItem * item, const QString &name)
{
    UMLListViewItem * parentItem = static_cast<UMLListViewItem *>(item->parent());
    UMLListViewItem::ListViewType type = item->type();
    switch (type) {
    case UMLListViewItem::lvt_Class_Diagram:
        return !m_doc->findView(Uml::DiagramType::Class, name);
        break;

    case UMLListViewItem::lvt_Sequence_Diagram:
        return !m_doc->findView(Uml::DiagramType::Sequence, name);
        break;

    case UMLListViewItem::lvt_UseCase_Diagram:
        return !m_doc->findView(Uml::DiagramType::UseCase, name);
        break;

    case UMLListViewItem::lvt_Collaboration_Diagram:
        return !m_doc->findView(Uml::DiagramType::Collaboration, name);
        break;

    case UMLListViewItem::lvt_State_Diagram:
        return !m_doc->findView(Uml::DiagramType::State, name);
        break;

    case UMLListViewItem::lvt_Activity_Diagram:
        return !m_doc->findView(Uml::DiagramType::Activity, name);
        break;

    case UMLListViewItem::lvt_Component_Diagram:
        return !m_doc->findView(Uml::DiagramType::Component, name);
        break;

    case UMLListViewItem::lvt_Deployment_Diagram:
        return !m_doc->findView(Uml::DiagramType::Deployment, name);
        break;

    case UMLListViewItem::lvt_EntityRelationship_Diagram:
        return !m_doc->findView(Uml::DiagramType::EntityRelationship, name);
        break;

    case UMLListViewItem::lvt_Actor:
    case UMLListViewItem::lvt_UseCase:
    case UMLListViewItem::lvt_Node:
    case UMLListViewItem::lvt_Artifact:
    case UMLListViewItem::lvt_Category:
        return !m_doc->findUMLObject(name, Model_Utils::convert_LVT_OT(type));
        break;

    case UMLListViewItem::lvt_Class:
    case UMLListViewItem::lvt_Package:
    case UMLListViewItem::lvt_Interface:
    case UMLListViewItem::lvt_Datatype:
    case UMLListViewItem::lvt_Enum:
    case UMLListViewItem::lvt_Entity:
    case UMLListViewItem::lvt_Component:
    case UMLListViewItem::lvt_Subsystem:
    case UMLListViewItem::lvt_Logical_Folder:
    case UMLListViewItem::lvt_UseCase_Folder:
    case UMLListViewItem::lvt_Component_Folder:
    case UMLListViewItem::lvt_Deployment_Folder:
    case UMLListViewItem::lvt_EntityRelationship_Folder: {
        UMLListViewItem::ListViewType lvt = parentItem->type();
        if (!Model_Utils::typeIsContainer(lvt))
            return (m_doc->findUMLObject(name) == 0);
        UMLPackage *pkg = static_cast<UMLPackage*>(parentItem->umlObject());
        if (pkg == 0) {
            uError() << "internal error - "
                     << "parent listviewitem is package but has no UMLObject";
            return true;
        }
        return (pkg->findObject(name) == 0);
        break;
    }

    case UMLListViewItem::lvt_Template:
    case UMLListViewItem::lvt_Attribute:
    case UMLListViewItem::lvt_EntityAttribute:
    case UMLListViewItem::lvt_Operation:
    case UMLListViewItem::lvt_EnumLiteral:
    case UMLListViewItem::lvt_UniqueConstraint:
    case UMLListViewItem::lvt_PrimaryKeyConstraint:
    case UMLListViewItem::lvt_ForeignKeyConstraint:
    case UMLListViewItem::lvt_CheckConstraint: {
        UMLClassifier *parent = static_cast<UMLClassifier*>(parentItem->umlObject());
        return (parent->findChildObject(name) == 0);
        break;
    }

    default:
        break;
    }
    return false;
}

/**
 * Renaming of an item has started.
 * @param item   the item which will be renamed
 */
void UMLListView::startRename(UMLListViewItem* item)
{
    if (item) {
        DEBUG(DBG_SRC) << item->text(0);
        if (m_editItem) {
            cancelRename(m_editItem);
        }
        item->startRename(0);
        openPersistentEditor(item, 0);
        m_editItem = item;
    }
    else {
        uError() << "Called without an item!";
    }
}

/**
 * Cancel rename event has occurred for the given item.
 */
void UMLListView::cancelRename(UMLListViewItem* item)
{
    if (item) {
        DEBUG(DBG_SRC) << item->text(0);
        // delete pointer first to lock slotItemChanged
        m_editItem = 0;
        closePersistentEditor(item, 0);
    }
    else {
        uError() << "Called without an item!";
    }
}

/**
 * Renaming of an item has ended.
 * @param item   the item which was renamed or not
 */
void UMLListView::endRename(UMLListViewItem* item)
{
    if (item) {
        DEBUG(DBG_SRC) << item->text(0);
        // delete pointer first to lock slotItemChanged
        m_editItem = 0;
        closePersistentEditor(item, 0);
        item->okRename(0);
    }
    else {
        uError() << "Called without an item!";
    }
}

/**
 *
 */
void UMLListView::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement listElement = qDoc.createElement("listview");
    m_rv->saveToXMI(qDoc, listElement);
    qElement.appendChild(listElement);
}

/**
 *
 */
bool UMLListView::loadFromXMI(QDomElement & element)
{
    QDomNode node = element.firstChild();
    QDomElement domElement = node.toElement();
    m_doc->writeToStatusBar(i18n("Loading listview..."));
    while (!domElement.isNull()) {
        if (domElement.tagName() == "listitem") {
            QString type = domElement.attribute("type", "-1");
            if (type == "-1")
                return false;
            UMLListViewItem::ListViewType lvType = (UMLListViewItem::ListViewType)type.toInt();
            if (lvType == UMLListViewItem::lvt_View) {
                if (!loadChildrenFromXMI(m_rv, domElement))
                    return false;
            } else
                return false;
        }
        node = node.nextSibling();
        domElement = node.toElement();

    }//end while
    return true;
}

/**
 *
 */
bool UMLListView::loadChildrenFromXMI(UMLListViewItem * parent, QDomElement & element)
{
    QDomNode node = element.firstChild();
    QDomElement domElement = node.toElement();
    while (!domElement.isNull()) {
        node = domElement.nextSibling();
        if (domElement.tagName() != "listitem") {
            domElement = node.toElement();
            continue;
        }
        QString id = domElement.attribute("id", "-1");
        QString type = domElement.attribute("type", "-1");
        QString label = domElement.attribute("label", "");
        QString open = domElement.attribute("open", "1");
        if (type == "-1")
            return false;
        UMLListViewItem::ListViewType lvType = (UMLListViewItem::ListViewType)type.toInt();
        bool bOpen = (bool)open.toInt();
        Uml::IDType nID = STR2ID(id);
        UMLObject * pObject = 0;
        UMLListViewItem * item = 0;
        if (nID != Uml::id_None) {
            // The following is an ad hoc hack for the copy/paste code.
            // The clip still contains the old children although new
            // UMLCLassifierListItems have already been created.
            // If the IDChangeLog finds new IDs this means we are in
            // copy/paste and need to adjust the child listitems to the
            // new UMLCLassifierListItems.
            IDChangeLog *idchanges = m_doc->changeLog();
            if (idchanges) {
                Uml::IDType newID = idchanges->findNewID(nID);
                if (newID != Uml::id_None) {
                    DEBUG(DBG_SRC) << " using id " << ID2STR(newID)
                                   << " instead of " << ID2STR(nID);
                    nID = newID;
                }
            }
            /************ End of hack for copy/paste code ************/

            pObject = m_doc->findObjectById(nID);
            if (pObject) {
                if (label.isEmpty())
                    label = pObject->name();
            } else if (Model_Utils::typeIsFolder(lvType)) {
                // Synthesize the UMLFolder here
                UMLObject *umlParent = parent->umlObject();
                UMLPackage *parentPkg = dynamic_cast<UMLPackage*>(umlParent);
                if (parentPkg == 0) {
                    uError() << "umlParent(" << umlParent << ") is not a UMLPackage";
                    domElement = node.toElement();
                    continue;
                }
                UMLFolder *f = new UMLFolder(label, nID);
                f->setUMLPackage(parentPkg);
                parentPkg->addObject(f);
                pObject = f;
                item = new UMLListViewItem(parent, label, lvType, pObject);
                // Moving all relevant UMLObjects to the new UMLFolder is done below,
                // in the switch(lvType)
            }
        } else if (Model_Utils::typeIsRootView(lvType)) {
            // Predefined folders did not have their ID set.
            const Uml::ModelType mt = Model_Utils::convert_LVT_MT(lvType);
            nID = m_doc->rootFolder(mt)->id();
        } else if (Model_Utils::typeIsFolder(lvType)) {
            // Pre-1.2 format: Folders did not have their ID set.
            // Pull a new ID now.
            nID = UniqueID::get();
        } else {
            uError() << "item of type " << type << " has no ID, skipping.";
            domElement = node.toElement();
            continue;
        }

        switch (lvType) {
        case UMLListViewItem::lvt_Actor:
        case UMLListViewItem::lvt_UseCase:
        case UMLListViewItem::lvt_Class:
        case UMLListViewItem::lvt_Interface:
        case UMLListViewItem::lvt_Datatype:
        case UMLListViewItem::lvt_Enum:
        case UMLListViewItem::lvt_Entity:
        case UMLListViewItem::lvt_Package:
        case UMLListViewItem::lvt_Subsystem:
        case UMLListViewItem::lvt_Component:
        case UMLListViewItem::lvt_Node:
        case UMLListViewItem::lvt_Artifact:
        case UMLListViewItem::lvt_Logical_Folder:
        case UMLListViewItem::lvt_UseCase_Folder:
        case UMLListViewItem::lvt_Component_Folder:
        case UMLListViewItem::lvt_Deployment_Folder:
        case UMLListViewItem::lvt_EntityRelationship_Folder:
        case UMLListViewItem::lvt_Category:
            item = findItem(nID);
            if (item == 0) {
                uError() << "INTERNAL ERROR: "
                    << "findItem(id " << ID2STR(nID) << ") returns 0";
                /*
                if (pObject && pObject->getUMLPackage() &&
                        parent->type() != UMLListViewItem::lvt_Package) {
                    // Pre-1.2 file format:
                    // Objects were not nested in their packages.
                    // Synthesize the nesting here.
                    UMLPackage *umlpkg = pObject->getUMLPackage();
                    UMLListViewItem *pkgItem = findUMLObject(umlpkg);
                    if (pkgItem == 0) {
                        DEBUG(DBG_SRC) << "synthesizing ListViewItem for package "
                                       << ID2STR(umlpkg->getID());
                        pkgItem = new UMLListViewItem(parent, umlpkg->getName(),
                                                      UMLListViewItem::lvt_Package, umlpkg);
                        pkgItem->setOpen(true);
                    }
                    item = new UMLListViewItem(pkgItem, label, lvType, pObject);
                } else {
                    item = new UMLListViewItem(parent, label, lvType, pObject);
                }
                 */
            } else if (parent != item->parent()) {
                // The existing item was created by the slot event triggered
                // by the loading of the corresponding model object from the
                // XMI file.
                // This early creation is done in order to support the loading
                // of foreign XMI files that do not have the umbrello specific
                // <listview> tag.
                // However, now that we encountered the real <listview> info,
                // we need to delete the existing item: Its parent is always
                // one of the default predefined folders, but the actual
                // listview item might be located in a user created folder.
                // Thanks to Achim Spangler for spotting the problem.
                UMLListViewItem *itmParent = dynamic_cast<UMLListViewItem*>(item->parent());
                DEBUG(DBG_SRC) << item->text(0) << " parent "
                               << parent->text(0) << " (" << parent << ") != "
                               << itmParent->text(0) << " (" << itmParent << ")";
                if (item == m_datatypeFolder && itmParent == m_lv[Uml::ModelType::Logical]) {
                    DEBUG(DBG_SRC) << "Reparenting the Datatypes folder is prohibited";
                } else {
                    UMLListViewItem *newItem = moveObject(nID, lvType, parent);
                    item = newItem;
                    if (item) {
                        DEBUG(DBG_SRC) << "Attempted reparenting of " << item->text(0)
                                       << "(current parent: " << (itmParent ? itmParent->text(0) : "0")
                                       << ", new parent: " << parent->text(0) << ")";
                    }
                }
            }
            break;
        case UMLListViewItem::lvt_Attribute:
        case UMLListViewItem::lvt_EntityAttribute:
        case UMLListViewItem::lvt_Template:
        case UMLListViewItem::lvt_Operation:
        case UMLListViewItem::lvt_EnumLiteral:
        case UMLListViewItem::lvt_UniqueConstraint:
        case UMLListViewItem::lvt_PrimaryKeyConstraint:
        case UMLListViewItem::lvt_ForeignKeyConstraint:
        case UMLListViewItem::lvt_CheckConstraint:
            item = findItem(nID);
            if (item == 0) {
                DEBUG(DBG_SRC) << "item " << ID2STR(nID) << " (of type "
                               << UMLListViewItem::toString(lvType) << ") does not yet exist...";
                UMLObject* umlObject = parent->umlObject();
                if (!umlObject) {
                    DEBUG(DBG_SRC) << "And also the parent->umlObject() does not exist";
                    return false;
                }
                if (nID == Uml::id_None) {
                    uWarning() << "lvtype " << UMLListViewItem::toString(lvType) << " has id -1";
                } else {
                    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(umlObject);
                    if (classifier) {
                        umlObject = classifier->findChildObjectById(nID);
                        if (umlObject) {
                            connectNewObjectsSlots(umlObject);
                            label = umlObject->name();
                            item = new UMLListViewItem(parent, label, lvType, umlObject);
                        } else {
                            DEBUG(DBG_SRC) << "lvtype " << UMLListViewItem::toString(lvType)
                                           << " child object " << ID2STR(nID) << " not found";
                        }
                    } else {
                        DEBUG(DBG_SRC) << "cast to classifier object failed";
                    }
                }
            }
            break;
        case UMLListViewItem::lvt_Logical_View:
            item = m_lv[Uml::ModelType::Logical];
            break;
        case UMLListViewItem::lvt_Datatype_Folder:
            item = m_datatypeFolder;
            break;
        case UMLListViewItem::lvt_UseCase_View:
            item = m_lv[Uml::ModelType::UseCase];
            break;
        case UMLListViewItem::lvt_Component_View:
            item = m_lv[Uml::ModelType::Component];
            break;
        case UMLListViewItem::lvt_Deployment_View:
            item = m_lv[Uml::ModelType::Deployment];
            break;
        case UMLListViewItem::lvt_EntityRelationship_Model:
            item = m_lv[Uml::ModelType::EntityRelationship];
            break;
        default:
            if (Model_Utils::typeIsDiagram(lvType)) {
                item = new UMLListViewItem(parent, label, lvType, nID);
            } else {
                uError() << "INTERNAL ERROR: unexpected listview type "
                    << UMLListViewItem::toString(lvType) << " (ID " << ID2STR(nID) << ")";
            }
            break;
        }//end switch

        if (item)  {
            item->setOpen((bool)bOpen);
            if (!loadChildrenFromXMI(item, domElement)) {
                return false;
            }
        } else {
            uWarning() << "unused list view item " << ID2STR(nID)
                       << " of lvtype " << UMLListViewItem::toString(lvType);
        }
        domElement = node.toElement();
    }//end while
    return true;
}

/**
 * Open all items in the list view.
 */
void UMLListView::expandAll(UMLListViewItem  *item)
{
    if (!item) item = static_cast<UMLListViewItem*>(topLevelItem(0));
    for (int i = 0; i < item->childCount(); i++)  {
        expandAll(item->childItem(i));
    }
    item->setExpanded(true);
}

/**
 * Close all items in the list view.
 */
void UMLListView::collapseAll(UMLListViewItem  *item)
{
    if (!item) item = static_cast<UMLListViewItem*>(topLevelItem(0));
    for (int i = 0; i < item->childCount(); i++)  {
        collapseAll(item->childItem(i));
    }
    item->setExpanded(false);
}

/**
 * Set the variable m_bStartedCut
 * to indicate that selection should be deleted
 * in slotCutSuccessful().
 */
void UMLListView::setStartedCut(bool startedCut)
{
    m_bStartedCut = startedCut;
}

/**
 * Set the variable m_bStartedCopy.
 * NB: While m_bStartedCut is reset as soon as the Cut operation is done,
 *     the variable m_bStartedCopy is reset much later - upon pasting.
 */
void UMLListView::setStartedCopy(bool startedCopy)
{
    m_bStartedCopy = startedCopy;
}

/**
 * Return the variable m_bStartedCopy.
 */
bool UMLListView::startedCopy() const
{
    return m_bStartedCopy;
}

/**
 * Returns the corresponding view if the listview type is one of the root views,
 * Root/Logical/UseCase/Component/Deployment/EntityRelation View.
 */
UMLListViewItem *UMLListView::rootView(UMLListViewItem::ListViewType type)
{
    UMLListViewItem *theView = 0;
    switch (type) {
    case UMLListViewItem::lvt_View:
        theView = m_rv;
        break;
    case UMLListViewItem::lvt_Logical_View:
        theView = m_lv[Uml::ModelType::Logical];
        break;
    case UMLListViewItem::lvt_UseCase_View:
        theView = m_lv[Uml::ModelType::UseCase];
        break;
    case UMLListViewItem::lvt_Component_View:
        theView = m_lv[Uml::ModelType::Component];
        break;
    case UMLListViewItem::lvt_Deployment_View:
        theView = m_lv[Uml::ModelType::Deployment];
        break;
    case UMLListViewItem::lvt_EntityRelationship_Model:
        theView = m_lv[Uml::ModelType::EntityRelationship];
        break;
    case UMLListViewItem::lvt_Datatype_Folder:   // @todo fix asymmetric naming
        theView = m_datatypeFolder;
        break;
    default:
        break;
    }
    return theView;
}

/**
 * Deletes all child-items of @p parent.
 * Do it in reverse order, because of the index.
 */
void UMLListView::deleteChildrenOf(UMLListViewItem* parent)
{
    if (!parent) {
        return;
    }
    if (parent == m_lv[Uml::ModelType::Logical]) {
        delete m_datatypeFolder;
        m_datatypeFolder = 0;
    }
    for (int i = parent->childCount() - 1; i >= 0; --i)
        parent->removeChild(parent->child(i));
}

/**
 *
 */
void UMLListView::closeDatatypesFolder()
{
    m_datatypeFolder->setOpen(false);
}

/**
 * Delete a listview item.
 * @param temp a non-null UMLListViewItem, for example: (UMLListViewItem*)currentItem()
 * @return     true if correctly deleted
 */
bool UMLListView::deleteItem(UMLListViewItem *temp)
{
    if (!temp)
        return false;
    UMLObject *object = temp->umlObject();
    UMLListViewItem::ListViewType lvt = temp->type();
    if (Model_Utils::typeIsDiagram(lvt)) {
        m_doc->removeDiagram(temp->getID());
    } else if (temp == m_datatypeFolder) {
        // we can't delete the datatypeFolder because umbrello will crash without a special handling
        return false;
    } else if (Model_Utils::typeIsCanvasWidget(lvt) || Model_Utils::typeIsClassifierList(lvt)) {
        UMLPackage *nmSpc = dynamic_cast<UMLPackage*>(object);
        if (nmSpc) {
            UMLObjectList contained = nmSpc->containedObjects();
            if (contained.count()) {
                KMessageBox::error(
                    0,
                    i18n("The folder must be emptied before it can be deleted."),
                    i18n("Folder Not Empty"));
                return false;
            }
        }
        UMLCanvasObject *canvasObj = dynamic_cast<UMLCanvasObject*>(object);
        if (canvasObj) {
            // We cannot just delete canvasObj here: What if the object
            // is still being used by others (for example, as a parameter
            // or return type of an operation) ?
            // Deletion should not have been permitted in the first place
            // if the object still has users - but Umbrello is lacking
            // that logic.
            canvasObj->removeAllChildObjects();
        }
        if (object) {
            m_doc->removeUMLObject(object);
            // Physical deletion of `temp' will be done by Qt signal, see
            // UMLDoc::removeUMLObject()
        } else {
            delete temp;
        }
    } else {
        uWarning() << "mt_Delete called with unknown type";
    }
    return true;
}

/**
 *
 */
void UMLListView::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
    QTreeWidget::dragEnterEvent(event);
}

/**
 *
 */
void UMLListView::dragMoveEvent(QDragMoveEvent* event)
{
    event->accept();
    QTreeWidget::dragMoveEvent( event );
}

/**
 *
 */
void UMLListView::dropEvent(QDropEvent* event)
{
    if ( !acceptDrag( event ) ) {
        event->ignore();
    }
    else {
        UMLListViewItem* item = static_cast<UMLListViewItem*>(itemAt(event->pos()));
        if (!item) {
            DEBUG(DBG_SRC) << "itemAt(mousePoint) returns 0";
            event->ignore();
            return;
        }
        slotDropped(event, 0, item);
    }
    QTreeWidget::dropEvent( event );
}

/**
 * Set the background color.
 * @param color   the new background color
 */
void UMLListView::setBackgroundColor(const QColor & color)
{
    QPalette palette;
    palette.setColor(backgroundRole(), color);
    setPalette(palette);
}

/**
 * Overloading operator for debugging output.
 */
QDebug operator<<(QDebug out, const UMLListView& view)
{
    UMLListViewItem* header = static_cast<UMLListViewItem*>(view.headerItem());
    if (header) {
        out << *header;
        for(int indx = 0;  indx < header->childCount(); ++indx) {
            UMLListViewItem* item = static_cast<UMLListViewItem*>(header->child(indx));
            if (item) {
                out << indx << " - " << *item << endl;
            }
            else {
                out << indx << " - " << "<null>" << endl;
            }
        }
    }
    else {
        out << "<null>";
    }
    return out.space();
}

#include "umllistview.moc"
