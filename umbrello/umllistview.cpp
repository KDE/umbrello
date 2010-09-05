/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2010                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "umllistview.h"

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

// kde includes
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kapplication.h>

// app includes
#include "actor.h"
#include "classifier.h"
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
#include "umlscene.h"

/**
 * Constructs the tree view.
 *
 * @param parent   The parent to this.
 * @param name     The internal name for this class.
 */
UMLListView::UMLListView(QWidget *parent, const char *name)
  : QTreeWidget(parent), m_pMenu(0), m_doc(UMLApp::app()->document())
{
    Q_UNUSED(name);
    //setup list view
    setBackgroundColor(Qt::white);
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
    sortByColumn(0,Qt::AscendingOrder);

    setEditTriggers(QAbstractItemView::EditKeyPressed);

    m_pMenu = 0;
    m_bStartedCut = m_bStartedCopy = false;
    m_bIgnoreCancelRename = true;
    m_bCreatingChildObject = false;
    m_rv = 0;
    for (int i = 0; i < Uml::N_MODELTYPES; ++i)
        m_lv[i] = 0;
    m_datatypeFolder = 0;
    m_editItem = 0;
    //setup slots/signals
    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem *)), this, SLOT(slotCollapsed(QTreeWidgetItem *)));
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)), this, SLOT(slotExpanded(QTreeWidgetItem *)));
    connect(UMLApp::app(), SIGNAL(sigCutSuccessful()), this, SLOT(slotCutSuccessful()));
    connect(this,SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(slotItemChanged(QTreeWidgetItem *, int)));
    connect(this,SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));
}

/**
 * Standard destructor.
 */
UMLListView::~UMLListView()
{
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
 *
 */
void UMLListView::slotItemChanged(QTreeWidgetItem * item, int column)
{
    Q_UNUSED(item); Q_UNUSED(column);
    if (m_editItem) {
        cancelRename(m_editItem);
    }
}

/**
 *
 */
void UMLListView::slotItemSelectionChanged()
{
    if (m_editItem) {
        cancelRename(m_editItem);
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
        UMLListViewItem * item = (UMLListViewItem*)itemAt(helpEvent->pos());
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
 *
 */
bool UMLListView::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() != QEvent::MouseButtonPress || qstrcmp("QHeader", metaObject()->className()) != 0)
        return QTreeWidget::eventFilter(o, e);
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::RightButton) {
        if (m_pMenu) {
            m_pMenu->hide();
            disconnect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuSel(QAction*)));
            delete m_pMenu;
        }
        UMLListViewItem * currItem = (UMLListViewItem*)currentItem();
        m_pMenu = new ListPopupMenu(this, Uml::lvt_Model, currItem->getUMLObject());
        m_pMenu->popup(me->globalPos());
        connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuSel(QAction*)));
        return true;
    }
    return QTreeWidget::eventFilter(o, e);
}

/**
 *
 */
void UMLListView::mousePressEvent(QMouseEvent *me)
{
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView)
        currentView->umlScene()->clearSelected();
    if (me->modifiers() != Qt::ShiftModifier)
        clearSelection();

    // Get the UMLListViewItem at the point where the mouse pointer was pressed
    UMLListViewItem * item = (UMLListViewItem*)itemAt(me->pos());

    const Qt::ButtonState button = me->button();

    if (!item || (button != Qt::RightButton && button != Qt::LeftButton)) {
        UMLApp::app()->docWindow()->updateDocumentation(true);
        return;
    }

    if (button == Qt::LeftButton) {
        UMLObject *o = item->getUMLObject();
        if (o)
            UMLApp::app()->docWindow()->showDocumentation(o, false);
        else
            UMLApp::app()->docWindow()->updateDocumentation(true);

        m_dragStartPosition = me->pos();
    }
    if (button == Qt::RightButton) {
        if (m_pMenu != 0) {
            m_pMenu->hide();
            disconnect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuSel(QAction*)));
            delete m_pMenu;
            m_pMenu = 0;
        }
        const Uml::ListView_Type type = item->getType();
        m_pMenu = new ListPopupMenu(this, type, item->getUMLObject());
        m_pMenu->popup(me->globalPos());
        connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuSel(QAction*)));
    }//end if right button

    QTreeWidget::mousePressEvent(me);
}

/**
 *
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
 *
 */
void UMLListView::mouseReleaseEvent(QMouseEvent *me)
{
    if (me->button() != Qt::LeftButton) {
        QTreeWidget::mouseReleaseEvent(me);
        return;
    }
    UMLListViewItem *item = dynamic_cast<UMLListViewItem*>(itemAt(me->pos()));
    if (item == 0 || !Model_Utils::typeIsDiagram(item->getType())) {
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
 *
 */
void UMLListView::keyPressEvent(QKeyEvent *ke)
{
    UMLView *view = UMLApp::app()->currentView();
    if (view && view->umlScene()->getSelectCount()) {
        // Widgets have been selected in the diagram area,
        // assume they handle the keypress.
        ke->accept();                 // munge and do nothing
    } else if (m_editItem) {
        const int k = ke->key();
        if (k == Qt::Key_Return) {
            endRename(m_editItem);
        }
        else {
            QWidget *editor = indexWidget(indexFromItem(m_editItem));
            if (editor)
                QCoreApplication::sendEvent(editor,ke);
        }
    }
    else {
        const int k = ke->key();
        if (k == Qt::Key_Delete || k == Qt::Key_Backspace) {
            // delete every selected item
            UMLListViewItemList selecteditems;
            getSelectedItemsRoot(selecteditems);
            foreach(UMLListViewItem *item , selecteditems) {
                deleteItem(dynamic_cast<UMLListViewItem*>(item));
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
    UMLListViewItem * currItem = (UMLListViewItem*)currentItem();
    if (!currItem) {
        uDebug() << "popupMenuSel invoked without currently selectedItem";
        return;
    }
    Uml::ListView_Type lvt = currItem->getType();
    Uml::Object_Type umlType = Uml::ot_UMLObject;
    ListPopupMenu::Menu_Type menuType = m_pMenu->getMenuType(action);
    QString name;

    switch (menuType) {
    case ListPopupMenu::mt_Class:
        addNewItem(currItem, Uml::lvt_Class);
        break;

    case ListPopupMenu::mt_Package:
        addNewItem(currItem, Uml::lvt_Package);
        break;

    case ListPopupMenu::mt_Subsystem:
        addNewItem(currItem, Uml::lvt_Subsystem);
        break;

    case ListPopupMenu::mt_Component:
        addNewItem(currItem, Uml::lvt_Component);
        break;

    case ListPopupMenu::mt_Node:
        addNewItem(currItem, Uml::lvt_Node);
        break;

    case ListPopupMenu::mt_Artifact:
        addNewItem(currItem, Uml::lvt_Artifact);
        break;

    case ListPopupMenu::mt_Interface:
        addNewItem(currItem, Uml::lvt_Interface);
        break;

    case ListPopupMenu::mt_Enum:
        addNewItem(currItem, Uml::lvt_Enum);
        break;

    case ListPopupMenu::mt_EnumLiteral:
        addNewItem(currItem, Uml::lvt_EnumLiteral);
        break;

    case ListPopupMenu::mt_Template:
        addNewItem(currItem, Uml::lvt_Template);
        break;

    case ListPopupMenu::mt_Entity:
        addNewItem(currItem, Uml::lvt_Entity);
        break;

    case ListPopupMenu::mt_Category:
        addNewItem(currItem, Uml::lvt_Category);
        break;

    case ListPopupMenu::mt_DisjointSpecialisation:
        {
            UMLCategory* catObj = static_cast<UMLCategory*>(currItem->getUMLObject());
            catObj->setType(UMLCategory::ct_Disjoint_Specialisation);
        }
        break;

    case ListPopupMenu::mt_OverlappingSpecialisation:
        {
            UMLCategory* catObj = static_cast<UMLCategory*>(currItem->getUMLObject());
            catObj->setType(UMLCategory::ct_Overlapping_Specialisation);
        }
        break;

    case ListPopupMenu::mt_Union:
        {
            UMLCategory* catObj = static_cast<UMLCategory*>(currItem->getUMLObject());
            catObj->setType(UMLCategory::ct_Union);
        }
        break;

    case ListPopupMenu::mt_Datatype:
        addNewItem(currItem, Uml::lvt_Datatype);
        break;

    case ListPopupMenu::mt_Actor:
        addNewItem(currItem, Uml::lvt_Actor);
        break;

    case ListPopupMenu::mt_UseCase:
        addNewItem(currItem, Uml::lvt_UseCase);
        break;

    case ListPopupMenu::mt_Attribute:
        addNewItem(currItem, Uml::lvt_Attribute);
        break;

    case ListPopupMenu::mt_EntityAttribute:
        addNewItem(currItem, Uml::lvt_EntityAttribute);
        break;

    case ListPopupMenu::mt_Operation:
        addNewItem(currItem, Uml::lvt_Operation);
        break;

    case ListPopupMenu::mt_UniqueConstraint:
        addNewItem(currItem, Uml::lvt_UniqueConstraint);
        break;

    case ListPopupMenu::mt_PrimaryKeyConstraint:
        addNewItem(currItem, Uml::lvt_PrimaryKeyConstraint);
        break;

    case ListPopupMenu::mt_ForeignKeyConstraint:
        addNewItem(currItem, Uml::lvt_ForeignKeyConstraint);
        break;

    case ListPopupMenu::mt_CheckConstraint:
        addNewItem(currItem, Uml::lvt_CheckConstraint);
        break;

    case ListPopupMenu::mt_Import_Classes:
        UMLApp::app()->slotImportClasses();
        break;

    case ListPopupMenu::mt_Import_Project:
        UMLApp::app()->slotImportProject();
        break;

    case ListPopupMenu::mt_Expand_All:
        expandAll(currItem);
        break;

    case ListPopupMenu::mt_Collapse_All:
        collapseAll(currItem);
        break;

    case ListPopupMenu::mt_Export_Image:
        m_doc->findView(currItem->getID())->umlScene()->getImageExporter()->exportView();
        break;

    case ListPopupMenu::mt_Externalize_Folder:
        {
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            UMLFolder *modelFolder = dynamic_cast<UMLFolder*>(current->getUMLObject());
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
            QString defaultFilename = current->getText().toLower();
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
            QString folderText = current->getText();
            folderText.remove(QRegExp("\\s*\\(.*$"));
            folderText.append(" (" + fileName + ')');
            current->setText(folderText);
            break;
        }

    case ListPopupMenu::mt_Internalize_Folder:
        {
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            UMLFolder *modelFolder = dynamic_cast<UMLFolder*>(current->getUMLObject());
            if (modelFolder == 0) {
                uError() << "modelFolder is 0";
                return;
            }
            modelFolder->setFolderFile(QString());
            // Recompute text of the folder
            QString folderText = current->getText();
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
            currItem->cancelRename(0);
            return;
        }

        { // ok, we are on another object, so find out on which one
            UMLObject * object = currItem->getUMLObject();
            if (!object) {
                uError() << "UMLObject of ... is null! Doing nothing.";
                return;
            }
            umlType = object->baseType();

            if (Model_Utils::typeIsCanvasWidget(lvt)) {
                object->showProperties(ClassPropDlg::page_gen);
            } else if (umlType == Uml::ot_EnumLiteral) {
                // Show the Enum Literal Dialog
                UMLEnumLiteral* selectedEnumLiteral = static_cast<UMLEnumLiteral*>(object);
                selectedEnumLiteral->showPropertiesDialog(this);

            } else if (umlType == Uml::ot_Attribute) {
                // show the attribute dialog
                UMLAttribute* selectedAttribute = static_cast<UMLAttribute*>(object);
                QPointer<UMLAttributeDialog> dialog = new UMLAttributeDialog(this, selectedAttribute);
                dialog->exec();
                delete dialog;
            } else if (umlType == Uml::ot_EntityAttribute) {
                // show the attribute dialog
                UMLEntityAttribute* selectedAttribute = static_cast<UMLEntityAttribute*>(object);
                QPointer<UMLEntityAttributeDialog> dialog = new UMLEntityAttributeDialog(this, selectedAttribute);
                dialog->exec();
                delete dialog;
            } else if (umlType == Uml::ot_Operation) {
                // show the operation dialog
                UMLOperation* selectedOperation = static_cast<UMLOperation*>(object);
                QPointer<UMLOperationDialog> dialog = new UMLOperationDialog(this, selectedOperation);
                dialog->exec();
                delete dialog;
            } else if (umlType == Uml::ot_Template) {
                // show the template dialog
                UMLTemplate* selectedTemplate = static_cast<UMLTemplate*>(object);
                QPointer<UMLTemplateDialog> dialog = new UMLTemplateDialog(this, selectedTemplate);
                dialog->exec();
                delete dialog;
            } else if (umlType == Uml::ot_UniqueConstraint) {
                // show the Unique Constraint dialog
                UMLUniqueConstraint* selectedUniqueConstraint = static_cast<UMLUniqueConstraint*>(object);
                QPointer<UMLUniqueConstraintDialog> dialog = new UMLUniqueConstraintDialog(this, selectedUniqueConstraint);
                dialog->exec();
                delete dialog;
            } else if (umlType == Uml::ot_ForeignKeyConstraint) {
                // show the Unique Constraint dialog
                UMLForeignKeyConstraint* selectedForeignKeyConstraint = static_cast<UMLForeignKeyConstraint*>(object);
                QPointer<UMLForeignKeyConstraintDialog> dialog = new UMLForeignKeyConstraintDialog(this, selectedForeignKeyConstraint);
                dialog->exec();
                delete dialog;
            } else if (umlType == Uml::ot_CheckConstraint) {
                // show the Check Constraint dialog
                UMLCheckConstraint* selectedCheckConstraint = static_cast<UMLCheckConstraint*>(object);
                QPointer<UMLCheckConstraintDialog> dialog = new UMLCheckConstraintDialog(this, selectedCheckConstraint);
                dialog->exec();
                delete dialog;
            } else {
                uWarning() << "calling properties on unknown type";
            }
            currItem->cancelRename(0);
        }
        break;

    case ListPopupMenu::mt_Logical_Folder:
        addNewItem(currItem, Uml::lvt_Logical_Folder);
        break;

    case ListPopupMenu::mt_UseCase_Folder:
        addNewItem(currItem, Uml::lvt_UseCase_Folder);
        break;

    case ListPopupMenu::mt_Component_Folder:
        addNewItem(currItem, Uml::lvt_Component_Folder);
        break;

    case ListPopupMenu::mt_Deployment_Folder:
        addNewItem(currItem, Uml::lvt_Deployment_Folder);
        break;

    case ListPopupMenu::mt_EntityRelationship_Folder:
        addNewItem(currItem, Uml::lvt_EntityRelationship_Folder);
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
            Uml::Diagram_Type dt = ListPopupMenu::convert_MT_DT(menuType);
            if (dt == Uml::dt_Undefined) {
                uWarning() << "unknown type" << menuType;
            } else {
                UMLObject* object = currItem->getUMLObject();
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
UMLListViewItem *UMLListView::findFolderForDiagram(Uml::Diagram_Type dt)
{
    UMLListViewItem *p = static_cast<UMLListViewItem*>(currentItem());
    if (p && Model_Utils::typeIsFolder(p->getType())
            && !Model_Utils::typeIsRootView(p->getType())) {
        return p;
    }
    switch (dt) {
    case Uml::dt_UseCase:
        p = m_lv[Uml::mt_UseCase];
        break;
    case Uml::dt_Component:
        p = m_lv[Uml::mt_Component];
        break;
    case Uml::dt_Deployment:
        p = m_lv[Uml::mt_Deployment];
        break;
    case Uml::dt_EntityRelationship:
        p = m_lv[Uml::mt_EntityRelationship];
        break;
    default:
        p = m_lv[Uml::mt_Logical];
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
    const Uml::Diagram_Type dt = v->umlScene()->type();
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
    Uml::ListView_Type lvt = Uml::lvt_Unknown;
    if (current)
        lvt = current->getType();
    Uml::Object_Type t = object->baseType();

    switch (t) {
    case Uml::ot_Attribute:
    case Uml::ot_Operation:
    case Uml::ot_Template:
    case Uml::ot_EnumLiteral:
    case Uml::ot_EntityAttribute:
    case Uml::ot_UniqueConstraint:
    case Uml::ot_ForeignKeyConstraint:
    case Uml::ot_CheckConstraint:
        //this will be handled by childObjectAdded
        return 0;
        break;
    case Uml::ot_Association:
    case Uml::ot_Role:
    case Uml::ot_Stereotype:
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
        } else if ((lvt == Uml::lvt_UseCase_Folder &&
                    (t == Uml::ot_Actor || t == Uml::ot_UseCase))
                   || (lvt == Uml::lvt_Component_Folder && t == Uml::ot_Component)
                   || (lvt == Uml::lvt_Deployment_Folder && t == Uml::ot_Node)
                   || (lvt == Uml::lvt_EntityRelationship_Folder && t == Uml::ot_Entity)) {
            parentItem = current;
        } else if (t == Uml::ot_Datatype) {
            parentItem = m_datatypeFolder;
        } else {
            Uml::Model_Type guess = Model_Utils::guessContainer(object);
            parentItem = m_lv[guess];
        }
    }
    break;
    }
    return parentItem;
}

/**
 * Return true if the given Object_Type permits child items.
 * A "child item" is anything that qualifies as a UMLClassifierListItem,
 * e.g. operations and attributes of classifiers.
 */
bool UMLListView::mayHaveChildItems(Uml::Object_Type type)
{
    bool retval = false;
    switch (type) {
    case Uml::ot_Class:
    case Uml::ot_Interface:
    case Uml::ot_Enum:
    case Uml::ot_Entity:  // CHECK: more?
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
        uDebug() << newItem->getType();
        uDebug() << object->name() << ", id= " << ID2STR(object->id())
            << ": item already exists.";
        Icon_Utils::Icon_Type icon = Model_Utils::convert_LVT_IT(newItem->getType());
        newItem->setIcon(icon);
        return;
    }
    UMLListViewItem* parentItem = determineParentItem(object);
    if (parentItem == 0)
        return;
    Uml::Object_Type type = object->baseType();

    connectNewObjectsSlots(object);
    const Uml::ListView_Type lvt = Model_Utils::convert_OT_LVT(object);
    QString name = object->name();
    if (type == Uml::ot_Folder) {
        UMLFolder *f = static_cast<UMLFolder*>(object);
        QString folderFile = f->folderFile();
        if (!folderFile.isEmpty())
            name.append(" (" + folderFile + ')');
    }
    newItem = new UMLListViewItem(parentItem, name, lvt, object);
    if (mayHaveChildItems(type)) {
        UMLClassifier *c = static_cast<UMLClassifier*>(object);
        UMLClassifierListItemList cListItems = c->getFilteredList(Uml::ot_UMLObject);
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
    Uml::Object_Type type = object->baseType();
    switch (type) {
    case Uml::ot_Class:
    case Uml::ot_Interface: {
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
    case Uml::ot_Enum: {
        UMLEnum *e = static_cast<UMLEnum*>(object);
        connect(e, SIGNAL(enumLiteralAdded(UMLClassifierListItem*)),
                this, SLOT(childObjectAdded(UMLClassifierListItem*)));
        connect(e, SIGNAL(enumLiteralRemoved(UMLClassifierListItem*)),
                this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
    }
    connect(object, SIGNAL(modified()), this, SLOT(slotObjectChanged()));
    break;
    case Uml::ot_Entity: {
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
    case Uml::ot_Datatype:
    case Uml::ot_Attribute:
    case Uml::ot_Operation:
    case Uml::ot_Template:
    case Uml::ot_EnumLiteral:
    case Uml::ot_EntityAttribute:
    case Uml::ot_UniqueConstraint:
    case Uml::ot_ForeignKeyConstraint:
    case Uml::ot_CheckConstraint:
    case Uml::ot_Package:
    case Uml::ot_Actor:
    case Uml::ot_UseCase:
    case Uml::ot_Component:
    case Uml::ot_Artifact:
    case Uml::ot_Node:
    case Uml::ot_Folder:
    case Uml::ot_Category:
        connect(object, SIGNAL(modified()), this, SLOT(slotObjectChanged()));
        break;
    case Uml::ot_UMLObject:
    case Uml::ot_Association:
    case Uml::ot_Stereotype:
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
    const QString text = child->toString(Uml::st_SigNoVis);
    UMLListViewItem *childItem = 0;
    UMLListViewItem *parentItem = findUMLObject(parent);
    if (parentItem == 0) {
        uDebug() << child->name() << ": parent " << parent->name()
        << " does not yet exist, creating it now.";
        const Uml::ListView_Type lvt = Model_Utils::convert_OT_LVT(parent);
        parentItem = new UMLListViewItem(m_lv[Uml::mt_Logical], parent->name(), lvt, parent);
    } else {
        childItem = parentItem->findChildObject(child);
    }
    if (childItem) {
        childItem->setText(text);
    } else {
        const Uml::ListView_Type lvt = Model_Utils::convert_OT_LVT(child);
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
    item->setText(v->umlScene()->name());
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
    connect(m_doc, SIGNAL(sigObjectCreated(UMLObject *)), this, SLOT(slotObjectCreated(UMLObject *)));
    connect(m_doc, SIGNAL(sigObjectRemoved(UMLObject *)), this, SLOT(slotObjectRemoved(UMLObject *)));
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
    UMLListViewItemList selecteditems;
    getSelectedItems(selecteditems);

    UMLListViewItemList  list;
    foreach(UMLListViewItem* item, selecteditems) {
        Uml::ListView_Type type = item->getType();
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
    for (int i=0; i < folder->childCount(); i++) {
        UMLListViewItem *item = folder->childItem(i);
        switch (item->getType()) {
        case Uml::lvt_Actor :
        case Uml::lvt_UseCase :
        case Uml::lvt_Class :
        case Uml::lvt_Package :
        case Uml::lvt_Subsystem :
        case Uml::lvt_Component :
        case Uml::lvt_Node :
        case Uml::lvt_Artifact :
        case Uml::lvt_Interface :
        case Uml::lvt_Datatype :
        case Uml::lvt_Enum :
        case Uml::lvt_Entity :
        case Uml::lvt_Category:
            if (item->getUMLObject() == obj)
                return item;
            break;
        case Uml::lvt_Logical_Folder :
        case Uml::lvt_UseCase_Folder :
        case Uml::lvt_Component_Folder :
        case Uml::lvt_Deployment_Folder :
        case Uml::lvt_EntityRelationship_Folder :
        case Uml::lvt_Datatype_Folder : {
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
    for (int i=0; i < topLevelItemCount(); i++) {
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
void UMLListView::changeIconOf(UMLObject *o, Icon_Utils::Icon_Type to)
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
        uWarning() << "returning 0 from UMLListView::findView()";
        return 0;
    }
    UMLListViewItem* item;
    Uml::Diagram_Type dType = v->umlScene()->type();
    Uml::ListView_Type type = Model_Utils::convert_DT_LVT(dType);
    Uml::IDType id = v->umlScene()->getID();
    if (dType == Uml::dt_UseCase) {
        item = m_lv[Uml::mt_UseCase];
    } else if (dType == Uml::dt_Component) {
        item = m_lv[Uml::mt_Component];
    } else if (dType == Uml::dt_Deployment) {
        item = m_lv[Uml::mt_Deployment];
    } else if (dType == Uml::dt_EntityRelationship) {
        item = m_lv[Uml::mt_EntityRelationship];
    } else {
        item = m_lv[Uml::mt_Logical];
    }
    for (int i=0; i < item->childCount(); i++) {
        UMLListViewItem* foundItem = recursiveSearchForView(item->childItem(i), type, id);
        if (foundItem)
            return foundItem;
    }
    uWarning() << "returning 0 at UMLListView::findView";
    return 0;
}

/**
 * Searches the tree for a diagram (view).
 * Warning: these method may return in some cases the wrong diagram 
 * Used by findView().
 */
UMLListViewItem* UMLListView::recursiveSearchForView(UMLListViewItem* listViewItem,
        Uml::ListView_Type type, Uml::IDType id)
{
    if (!listViewItem)
        return 0;
    // 
    //if (listViewItem->getType() == type && listViewItem->getID() == id)
    //    return listViewItem;

    if (Model_Utils::typeIsFolder(listViewItem->getType())) {
        for (int i=0; i < listViewItem->childCount(); i++) {
            UMLListViewItem* child = listViewItem->childItem(i);
            UMLListViewItem* resultListViewItem = recursiveSearchForView(child, type, id);
            if (resultListViewItem)
                return resultListViewItem;
        }
    } else {
        if (listViewItem->getType() == type && listViewItem->getID() == id)
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
        m_rv =  new UMLListViewItem(this, i18n("Views"), Uml::lvt_View);
        m_rv->setID("Views");
        //m_rv->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

        for (int i = 0; i < Uml::N_MODELTYPES; ++i) {
            Uml::Model_Type mt = (Uml::Model_Type)i;
            UMLFolder *sysFolder = m_doc->rootFolder(mt);
            Uml::ListView_Type lvt = Model_Utils::convert_MT_LVT(mt);
            m_lv[i] = new UMLListViewItem(m_rv, sysFolder->localName(), lvt, sysFolder);
        }
    } 
    UMLFolder *datatypeFolder = m_doc->datatypeFolder();
    if (!m_datatypeFolder)
        m_datatypeFolder = new UMLListViewItem(m_lv[Uml::mt_Logical], datatypeFolder->localName(),
                                           Uml::lvt_Datatype_Folder, datatypeFolder);

    m_rv->setOpen(true);
    for (int i = 0; i < Uml::N_MODELTYPES; ++i)
        m_lv[i]->setOpen(true);
    m_datatypeFolder->setOpen(false);

    //setup misc.
    delete m_pMenu;
    m_pMenu = 0;
    m_bStartedCut = m_bStartedCopy = false;
    m_bIgnoreCancelRename = true;
    m_bCreatingChildObject = false;
    headerItem()->setHidden(true);
}

/**
 *
 */
void UMLListView::clean()
{
    for (int i = 0; i < Uml::N_MODELTYPES; ++i)
        deleteChildrenOf(m_lv[i]);
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
 *
 */
void UMLListView::mouseDoubleClickEvent(QMouseEvent * me)
{
    UMLListViewItem * item = static_cast<UMLListViewItem *>(currentItem());
    if (!item || me->button() != Qt::LeftButton)
        return;
    //see if on view
    Uml::ListView_Type lvType = item->getType();
    if (Model_Utils::typeIsDiagram(lvType)) {
        UMLView * pView = m_doc->findView(item->getID());
        if (!pView)
            return;
        UMLApp::app()->docWindow()->updateDocumentation(false);
        pView->umlScene()->showPropDialog();
        UMLApp::app()->docWindow()->showDocumentation(pView, true);
        item->cancelRename(0);
        return;
    }
    //else see if an object
    UMLObject * object = item->getUMLObject();
    //continue only if we are on a UMLObject
    if (!object) {
        return;
    }

    Uml::Object_Type type = object->baseType();
    int page = ClassPropDlg::page_gen;
    if (Model_Utils::isClassifierListitem(type)) {
        object = (UMLObject *)object->parent();
    }
    //set what page to show
    switch (type) {

    case Uml::ot_Attribute:
        page = ClassPropDlg::page_att;
        break;
    case Uml::ot_Operation:
        page = ClassPropDlg::page_op;
        break;
    case Uml::ot_EntityAttribute:
        page = ClassPropDlg::page_entatt;
        break;
    case Uml::ot_UniqueConstraint:
    case Uml::ot_ForeignKeyConstraint:
    case Uml::ot_CheckConstraint:
        page = ClassPropDlg::page_constraint;
        break;
    default:
        page = ClassPropDlg::page_gen;
        break;
    }

    if (object) {
        object->showProperties(page);
    }
    item->cancelRename(0);  //double click can cause it to go into rename mode.
}

/**
 *
 */
bool UMLListView::acceptDrag(QDropEvent* event) const
{
    UMLListViewItem* item = (UMLListViewItem*)itemAt(event->pos());
    if (!item) {
        uDebug() << "itemAt(mouse position) returns 0";
        return false;
    }
    ((QTreeWidget*)this)->setCurrentItem((QTreeWidgetItem*)item);

    UMLDragData::LvTypeAndID_List list;
    if (! UMLDragData::getClip3TypeAndID(event->mimeData(), list)) {
        uDebug() << "UMLDragData::getClip3TypeAndID returns false";
        return false;
    }

    UMLDragData::LvTypeAndID_It it(list);
    UMLDragData::LvTypeAndID * data = 0;
    Uml::ListView_Type dstType = item->getType();
    bool accept = true;
    while (accept && it.hasNext()) {
        data = it.next();
        Uml::ListView_Type srcType = data->type;
        switch (srcType) {
        case Uml::lvt_Class:
        case Uml::lvt_Package:
        case Uml::lvt_Interface:
        case Uml::lvt_Enum:
            if (dstType == Uml::lvt_Logical_View ||
                    dstType == Uml::lvt_Class ||
                    dstType == Uml::lvt_Package) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == Uml::lvt_Logical_Folder);
            }
            break;
        case Uml::lvt_Attribute:
            if (dstType == Uml::lvt_Class) {
                accept = !item->isOwnParent(data->id);
            }
            break;
        case Uml::lvt_EntityAttribute:
            if (dstType == Uml::lvt_Entity) {
                accept = !item->isOwnParent(data->id);
            }
            break;
        case Uml::lvt_Operation:
            if (dstType == Uml::lvt_Class ||
                    dstType == Uml::lvt_Interface) {
                accept = !item->isOwnParent(data->id);
            }
            break;
        case Uml::lvt_Datatype:
            accept = (dstType == Uml::lvt_Logical_Folder ||
                      dstType == Uml::lvt_Datatype_Folder ||
                      dstType == Uml::lvt_Class ||
                      dstType == Uml::lvt_Interface ||
                      dstType == Uml::lvt_Package);
            break;
        case Uml::lvt_Class_Diagram:
        case Uml::lvt_Collaboration_Diagram:
        case Uml::lvt_State_Diagram:
        case Uml::lvt_Activity_Diagram:
        case Uml::lvt_Sequence_Diagram:
            accept = (dstType == Uml::lvt_Logical_Folder ||
                      dstType == Uml::lvt_Logical_View);
            break;
        case Uml::lvt_Logical_Folder:
            if (dstType == Uml::lvt_Logical_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == Uml::lvt_Logical_View);
            }
            break;
        case Uml::lvt_UseCase_Folder:
            if (dstType == Uml::lvt_UseCase_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == Uml::lvt_UseCase_View);
            }
            break;
        case Uml::lvt_Component_Folder:
            if (dstType == Uml::lvt_Component_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == Uml::lvt_Component_View);
            }
            break;
        case Uml::lvt_Deployment_Folder:
            if (dstType == Uml::lvt_Deployment_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == Uml::lvt_Deployment_View);
            }
            break;
        case Uml::lvt_EntityRelationship_Folder:
            if (dstType == Uml::lvt_EntityRelationship_Folder) {
                accept = !item->isOwnParent(data->id);
            } else {
                accept = (dstType == Uml::lvt_EntityRelationship_Model);
            }
            break;
        case Uml::lvt_Actor:
        case Uml::lvt_UseCase:
        case Uml::lvt_UseCase_Diagram:
            accept = (dstType == Uml::lvt_UseCase_Folder ||
                      dstType == Uml::lvt_UseCase_View);
            break;
        case Uml::lvt_Subsystem:
            accept = (dstType == Uml::lvt_Component_Folder ||
                      dstType == Uml::lvt_Subsystem);
            break;
        case Uml::lvt_Component:
            accept = (dstType == Uml::lvt_Component_Folder ||
                      dstType == Uml::lvt_Component ||
                      dstType == Uml::lvt_Subsystem);
            break;
        case Uml::lvt_Artifact:
        case Uml::lvt_Component_Diagram:
            accept = (dstType == Uml::lvt_Component_Folder ||
                      dstType == Uml::lvt_Component_View);
            break;
        case Uml::lvt_Node:
        case Uml::lvt_Deployment_Diagram:
            accept = (dstType == Uml::lvt_Deployment_Folder);
            break;
        case Uml::lvt_Entity:
        case Uml::lvt_EntityRelationship_Diagram:
        case Uml::lvt_Category:
            accept = (dstType == Uml::lvt_EntityRelationship_Folder);
            break;
        default:
            accept = false;
            break;
        }
    }

    //uDebug() << "dstType = " << dstType << ", accept=" << accept;
    return accept;
}

/**
 * Auxiliary method for moveObject(): Adds the model object at the proper
 * new container (package if nested, UMLDoc if at global level), and
 * updates the containment relationships in the model.
 */
void UMLListView::addAtContainer(UMLListViewItem *item, UMLListViewItem *parent)
{
    UMLCanvasObject *o = static_cast<UMLCanvasObject*>(item->getUMLObject());
    if (o == 0) {
        uDebug() << item->getText() << ": item's UMLObject is 0";
    } else if (Model_Utils::typeIsContainer(parent->getType())) {
        /**** TBC: Do this here?
                   If yes then remove that logic at the callers
                   and rename this method to moveAtContainer()
        UMLPackage *oldPkg = o->getUMLPackage();
        if (oldPkg)
            oldPkg->removeObject(o);
         *********/
        UMLPackage *pkg = static_cast<UMLPackage*>(parent->getUMLObject());
        o->setUMLPackage(pkg);
        pkg->addObject(o);
    } else {
        uError() << item->getText() << ": parent type is " << parent->getType();
    }
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView)
        currentView->umlScene()->updateContainment(o);
}

/**
 * Moves an object given is unique ID and listview type to an
 * other listview parent item.
 * Also takes care of the corresponding move in the model.
 */
UMLListViewItem * UMLListView::moveObject(Uml::IDType srcId, Uml::ListView_Type srcType,
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
        newParentObj = newParent->getUMLObject();
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

    Uml::ListView_Type newParentType = newParent->getType();
    uDebug() << "newParentType is " << newParentType;
    UMLListViewItem *newItem = 0;

    //make sure trying to place in correct location
    switch (srcType) {
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_Actor:
    case Uml::lvt_UseCase:
    case Uml::lvt_UseCase_Diagram:
        if (newParentType == Uml::lvt_UseCase_Folder ||
                newParentType == Uml::lvt_UseCase_View) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Artifact:
    case Uml::lvt_Component_Diagram:
        if (newParentType == Uml::lvt_Component_Folder ||
                newParentType == Uml::lvt_Component_View) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case Uml::lvt_Subsystem:
        if (newParentType == Uml::lvt_Component_Folder ||
                newParentType == Uml::lvt_Component_View ||
                newParentType == Uml::lvt_Subsystem) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case Uml::lvt_Component:
        if (newParentType == Uml::lvt_Component_Folder ||
                newParentType == Uml::lvt_Component_View ||
                newParentType == Uml::lvt_Component ||
                newParentType == Uml::lvt_Subsystem) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_Node:
    case Uml::lvt_Deployment_Diagram:
        if (newParentType == Uml::lvt_Deployment_Folder ||
                newParentType == Uml::lvt_Deployment_View) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case Uml::lvt_EntityRelationship_Folder:
    case Uml::lvt_Entity:
    case Uml::lvt_Category:
    case Uml::lvt_EntityRelationship_Diagram:
        if (newParentType == Uml::lvt_EntityRelationship_Folder ||
                newParentType == Uml::lvt_EntityRelationship_Model) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case Uml::lvt_Collaboration_Diagram:
    case Uml::lvt_Class_Diagram:
    case Uml::lvt_State_Diagram:
    case Uml::lvt_Activity_Diagram:
    case Uml::lvt_Sequence_Diagram:
    case Uml::lvt_Logical_Folder:
        if (newParentType == Uml::lvt_Logical_Folder ||
                newParentType == Uml::lvt_Logical_View) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            addAtContainer(newItem, newParent);
        }
        break;
    case Uml::lvt_Class:
    case Uml::lvt_Package:
    case Uml::lvt_Interface:
    case Uml::lvt_Enum:
    case Uml::lvt_Datatype:
        if (newParentType == Uml::lvt_Logical_Folder ||
                newParentType == Uml::lvt_Datatype_Folder ||
                newParentType == Uml::lvt_Logical_View ||
                newParentType == Uml::lvt_Class ||
                newParentType == Uml::lvt_Interface ||
                newParentType == Uml::lvt_Package) {
            newItem = move->deepCopy(newParent);
            if (m_doc->loading())         // deletion is not safe while loading
                move->setVisible(false);  // (the <listview> XMI may be corrupted)
            else
                delete move;
            UMLCanvasObject *o = static_cast<UMLCanvasObject*>(newItem->getUMLObject());
            if (o == 0) {
                uDebug() << "moveObject: newItem's UMLObject is 0";
            } else if (newParentObj == 0) {
                uError() << o->name() << ": newParentObj is 0";
            } else {
                UMLPackage *pkg = static_cast<UMLPackage*>(newParentObj);
                o->setUMLPackage(pkg);
                pkg->addObject(o);
            }
            UMLView *currentView = UMLApp::app()->currentView();
            if (currentView)
                currentView->umlScene()->updateContainment(o);
        }
        break;
    case Uml::lvt_Attribute:
    case Uml::lvt_Operation:
        if (newParentType == Uml::lvt_Class ||
                newParentType == Uml::lvt_Interface) {
            // update list view

            newItem = move->deepCopy(newParent);
            // we don't delete move right away, it will be deleted in slots,
            // called by subsequent steps
            //delete move;

            // update model objects
            m_bCreatingChildObject = true;

            UMLClassifier *oldParentClassifier = dynamic_cast<UMLClassifier*>(srcObj->parent());
            UMLClassifier *newParentClassifier = dynamic_cast<UMLClassifier*>(newParentObj);
            if (srcType == Uml::lvt_Attribute) {
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
        uDebug() << "item is 0 - doing nothing";
        return;
    }
    UMLDragData::LvTypeAndID_List srcList;
    if (! UMLDragData::getClip3TypeAndID(de->mimeData(), srcList)) {
        return;
    }
    UMLListViewItem *newParent = (UMLListViewItem*)item;
    uDebug() << "slotDropped: newParent->getText() is " << newParent->getText();
    UMLDragData::LvTypeAndID_It it(srcList);
    UMLDragData::LvTypeAndID * src = 0;
    while (it.hasNext()) {
        src = it.next();
        moveObject(src->id, src->type, newParent);
    }
}

/**
 * Get selected items.
 *
 * @param ItemList   List of UMLListViewItems returned.
 * @return           The number of selected items.
 */
int UMLListView::getSelectedItems(UMLListViewItemList &ItemList)
{
    UMLListViewItemIterator it(this);
    // iterate through all items of the list view
    for (; *it; ++it) {
        if ((*it)->isSelected()) {
            UMLListViewItem *item = (UMLListViewItem*)*it;
            ItemList.append(item);
        }
    }
    uDebug() << "selItems = " << ItemList.count();

    return (int)ItemList.count();
}

/**
 * Get selected items, but only root elements selected (without children).
 *
 * @param ItemList   List of UMLListViewItems returned.
 * @return           The number of selected items.
 */
int UMLListView::getSelectedItemsRoot(UMLListViewItemList &ItemList)
{
    QTreeWidgetItemIterator it(this);

    // iterate through all items of the list view
    for (; *it; ++it) {
        if ((*it)->isSelected()) {
            UMLListViewItem *item = (UMLListViewItem*)*it;
            // this is the trick, we select only the item with a parent unselected
            // since we can't select a child and its grandfather without its parent
            // we would be able to delete each item individually, without an invalid iterator
            if (item && item->parent() && item->parent()->isSelected() == false) {
                ItemList.append(item);
            }
        }
    }
    uDebug() << "selItems = " << ItemList.count();

    return (int)ItemList.count();
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
    Uml::ListView_Type lvt = Model_Utils::convert_DT_LVT(view->umlScene()->type());
    UMLListViewItem *parent = 0;
    UMLFolder *f = view->umlScene()->folder();
    if (f) {
        parent = findUMLObject(f);
        if (parent == 0)
            uError() << view->umlScene()->name() << ": findUMLObject(" << f->name() << ") returns 0";
    } else {
        uDebug() << view->umlScene()->name() << ": no parent folder set, using predefined folder";
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
 * parent is null the ListView Decides who is going to be the parent.
 */
UMLListViewItem* UMLListView::createItem(UMLListViewItem& Data, IDChangeLog& IDChanges,
        UMLListViewItem* parent /*= 0*/)
{
    UMLObject* pObject = 0;
    UMLListViewItem* item = 0;
    Uml::ListView_Type lvt = Data.getType();
    if (!parent) {
        parent = determineParentItem(lvt);
        if (!parent)
            return 0;
    }

    switch (lvt) {
    case Uml::lvt_Actor:
    case Uml::lvt_UseCase:
    case Uml::lvt_Class:
    case Uml::lvt_Package:
    case Uml::lvt_Subsystem:
    case Uml::lvt_Component:
    case Uml::lvt_Node:
    case Uml::lvt_Artifact:
    case Uml::lvt_Interface:
    case Uml::lvt_Datatype:
    case Uml::lvt_Enum:
    case Uml::lvt_Entity:
    case Uml::lvt_Category:
    case Uml::lvt_Logical_Folder:
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_EntityRelationship_Folder:
        /***
        int newID = IDChanges.findNewID(Data.getID());
        //if there is no ListViewItem associated with the new ID,
        //it could exist an Item already asocciated if the user chose to reuse an uml object
        if(!(item = findItem(newID))) {
                pObject = m_doc->findObjectById( IDChanges.findNewID(Data.getID()) );
                item = new UMLListViewItem(parent, Data.getText(), lvt, pObject);
        } ***/
        pObject = m_doc->findObjectById(Data.getID());
        item = new UMLListViewItem(parent, Data.getText(), lvt, pObject);
        break;
    case Uml::lvt_Datatype_Folder:
        item = new UMLListViewItem(parent, Data.getText(), lvt);
        break;
    case Uml::lvt_Attribute:
    case Uml::lvt_EntityAttribute:
    case Uml::lvt_Operation:
    case Uml::lvt_Template:
    case Uml::lvt_EnumLiteral:
    case Uml::lvt_UniqueConstraint:
    case Uml::lvt_PrimaryKeyConstraint:
    case Uml::lvt_ForeignKeyConstraint:
    case Uml::lvt_CheckConstraint: {
        UMLClassifier *pClass =  static_cast<UMLClassifier*>(parent->getUMLObject());
        Uml::IDType newID = IDChanges.findNewID(Data.getID());
        pObject = pClass->findChildObjectById(newID);
        if (pObject) {
            item = new UMLListViewItem(parent, Data.getText(), lvt, pObject);
        } else {
            item = 0;
        }
        break;
    }
    case Uml::lvt_UseCase_Diagram:
    case Uml::lvt_Sequence_Diagram:
    case Uml::lvt_Collaboration_Diagram:
    case Uml::lvt_Class_Diagram:
    case Uml::lvt_State_Diagram:
    case Uml::lvt_Activity_Diagram:
    case Uml::lvt_Component_Diagram:
    case Uml::lvt_Deployment_Diagram:
    case Uml::lvt_EntityRelationship_Diagram: {
        Uml::IDType newID = IDChanges.findNewID(Data.getID());
        UMLView* v = m_doc->findView(newID);
        if (v == 0) {
            return 0;
        }
        const Uml::ListView_Type lvt = Model_Utils::convert_DT_LVT(v->umlScene()->type());
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
 * Determine the parent ListViewItem given a ListView_Type.
 * This parent is used for creating new UMLListViewItems.
 *
 * @param lvt   The ListView_Type for which to lookup the parent.
 * @return  Pointer to the parent UMLListViewItem chosen.
 */
UMLListViewItem* UMLListView::determineParentItem(Uml::ListView_Type lvt) const
{
    UMLListViewItem* parent = 0;
    switch (lvt) {
    case Uml::lvt_Datatype:
        parent = m_datatypeFolder;
        break;
    case Uml::lvt_Actor:
    case Uml::lvt_UseCase:
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_UseCase_Diagram:
        parent = m_lv[Uml::mt_UseCase];
        break;
    case Uml::lvt_Component_Diagram:
    case Uml::lvt_Component:
    case Uml::lvt_Artifact:
        parent = m_lv[Uml::mt_Component];
        break;
    case Uml::lvt_Deployment_Diagram:
    case Uml::lvt_Node:
        parent = m_lv[Uml::mt_Deployment];
        break;
    case Uml::lvt_EntityRelationship_Diagram:
    case Uml::lvt_Entity:
    case Uml::lvt_Category:
        parent = m_lv[Uml::mt_EntityRelationship];
        break;
    default:
        if (Model_Utils::typeIsDiagram(lvt) || !Model_Utils::typeIsClassifierList(lvt))
            parent = m_lv[Uml::mt_Logical];
        break;
    }
    return parent;
}

/**
 *  Return the amount of items selected.
 */
int UMLListView::getSelectedCount()
{
    QTreeWidgetItemIterator it(this);
    int count = 0;
    // iterate through all items of the list view
    for (; *it; ++it) {
        if ((*it)->isSelected()) {
            count++;
        }
    }
    return count;
}

/**
 * Returns the document pointer. Called by the UMLListViewItem class.
 */
UMLDoc * UMLListView::document() const
{
    return m_doc;
}

/**
 *
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
 * Deployment, EntityRelationship.) Returns the ListView_Type
 * of the matching root view; if no match then continues the
 * search using the item's parent, then grandparent, and so forth.
 * Returns Uml::lvt_Unknown if no match at all is found.
 */
Uml::ListView_Type UMLListView::rootViewType(UMLListViewItem *item)
{
    if (item == m_rv)
        return Uml::lvt_View;
    if (item == m_lv[Uml::mt_Logical])
        return Uml::lvt_Logical_View;
    if (item == m_lv[Uml::mt_UseCase])
        return Uml::lvt_UseCase_View;
    if (item == m_lv[Uml::mt_Component])
        return Uml::lvt_Component_View;
    if (item == m_lv[Uml::mt_Deployment])
        return Uml::lvt_Deployment_View;
    if (item == m_lv[Uml::mt_EntityRelationship])
        return Uml::lvt_EntityRelationship_Model;
    UMLListViewItem *parent = dynamic_cast<UMLListViewItem*>(item->parent());
    if (parent)
        return rootViewType(parent);
    return Uml::lvt_Unknown;
}

/**
 * Return true if the given list view type can be expanded/collapsed.
 */
bool UMLListView::isExpandable(Uml::ListView_Type lvt)
{
    if (Model_Utils::typeIsRootView(lvt) || Model_Utils::typeIsFolder(lvt))
        return true;
    switch (lvt) {
    case Uml::lvt_Package:
    case Uml::lvt_Component:
    case Uml::lvt_Subsystem:
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
    if (isExpandable(myItem->getType())) {
        myItem->updateFolder();
    }
}

/**
 * Calls updateFolder() on the item to update the icon to closed.
 */
void UMLListView::slotCollapsed(QTreeWidgetItem * item)
{
    UMLListViewItem * myItem = static_cast<UMLListViewItem*>(item);
    if (isExpandable(myItem->getType())) {
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
        popupMenuSel(m_pMenu->getAction(ListPopupMenu::mt_Delete));
        //deletion code here
        m_bStartedCut = false;
    }
}

/**
 *
 */
void UMLListView::startUpdate()
{
    setSortingEnabled(false);
}

/**
 *
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
void UMLListView::addNewItem(UMLListViewItem *parentItem, Uml::ListView_Type type)
{
    if (type == Uml::lvt_Datatype) {
        parentItem = m_datatypeFolder;
    }

    UMLListViewItem * newItem = 0;
    parentItem->setOpen(true);

    Icon_Utils::Icon_Type icon = Model_Utils::convert_LVT_IT(type);

    QString name;
    if (Model_Utils::typeIsDiagram(type)) {
        Uml::Diagram_Type dt = Model_Utils::convert_LVT_DT(type);
        name = getUniqueDiagramName(dt);
        newItem = new UMLListViewItem(parentItem, name, type, Uml::id_None);
    } else {
        Uml::Object_Type ot = Model_Utils::convert_LVT_OT(type);
        if (ot == Uml::ot_UMLObject) {
            uDebug() << "no UMLObject for listview type " << type;
            return;
        }
        UMLPackage *parentPkg =
            dynamic_cast<UMLPackage*>(parentItem->getUMLObject());
        if (parentPkg == 0) {
            uError() << "type " << type << ": parentPkg is 0";
            return;
        }
        if (Model_Utils::typeIsClassifierList(type)) {
            UMLClassifier *parent = static_cast<UMLClassifier*>(parentPkg);
            name = parent->uniqChildName(ot);
        } else {
            name = Model_Utils::uniqObjectName(ot, parentPkg);
        }
        newItem = new UMLListViewItem(parentItem, name, type, (UMLObject*)0);
    }
    m_bIgnoreCancelRename = false;
    newItem->setIcon(icon);
    newItem->setOpen(true);
    newItem->setCreating(true);
}

/**
 * Called for informing the list view that an item was renamed.
 */
bool UMLListView::itemRenamed(UMLListViewItem  * item , int col)
{
    Q_UNUSED(col);
    //if true the item was cancel before this message
    if (m_bIgnoreCancelRename) {
        return true;
    }
    m_bIgnoreCancelRename = true;
    UMLListViewItem * renamedItem = static_cast< UMLListViewItem *>(item) ;
    Uml::ListView_Type type = renamedItem->getType();
    QString newText = renamedItem->text(0);
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
        if (type == Uml::lvt_Operation) {
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

    switch (type) {
    case Uml::lvt_Actor:
    case Uml::lvt_Class:
    case Uml::lvt_Package:
    case Uml::lvt_Logical_Folder:
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_EntityRelationship_Folder:
    case Uml::lvt_Subsystem:
    case Uml::lvt_Component:
    case Uml::lvt_Node:
    case Uml::lvt_Artifact:
    case Uml::lvt_Interface:
    case Uml::lvt_Datatype:
    case Uml::lvt_Enum:
    case Uml::lvt_Entity:
    case Uml::lvt_UseCase:
    case Uml::lvt_Category: {
        Uml::Object_Type ot = Model_Utils::convert_LVT_OT(type);
        if (! ot) {
            uError() << "internal error";
            return false;
        }
        UMLObject *o = createUMLObject(renamedItem, ot);
        if (type == Uml::lvt_Subsystem)
            o->setStereotype("subsystem");
        else if (Model_Utils::typeIsFolder(type))
            o->setStereotype("folder");
    }
    break;

    case Uml::lvt_Attribute:
    case Uml::lvt_EntityAttribute:
    case Uml::lvt_Operation:
    case Uml::lvt_Template:
    case Uml::lvt_EnumLiteral:
    case Uml::lvt_UniqueConstraint:
    case Uml::lvt_ForeignKeyConstraint:
    case Uml::lvt_CheckConstraint:

        return createChildUMLObject(renamedItem, Model_Utils::convert_LVT_OT(type));
        break;

    case Uml::lvt_PrimaryKeyConstraint: {

        bool result = createChildUMLObject(renamedItem, Model_Utils::convert_LVT_OT(type));
        UMLObject* obj = renamedItem->getUMLObject();
        UMLUniqueConstraint* uuc = static_cast<UMLUniqueConstraint*>(obj);
        UMLEntity* ent = static_cast<UMLEntity*>(uuc->parent());
        if (ent)
            ent->setAsPrimaryKey(uuc);

        return result;
    }
    break;

    case Uml::lvt_Class_Diagram:
        createDiagram(renamedItem, Uml::dt_Class);
        break;

    case Uml::lvt_UseCase_Diagram:
        createDiagram(renamedItem, Uml::dt_UseCase);
        break;

    case Uml::lvt_Sequence_Diagram:
        createDiagram(renamedItem, Uml::dt_Sequence);
        break;

    case Uml::lvt_Collaboration_Diagram:
        createDiagram(renamedItem, Uml::dt_Collaboration);
        break;

    case Uml::lvt_State_Diagram:
        createDiagram(renamedItem, Uml::dt_State);
        break;

    case Uml::lvt_Activity_Diagram:
        createDiagram(renamedItem, Uml::dt_Activity);
        break;

    case Uml::lvt_Component_Diagram:
        createDiagram(renamedItem, Uml::dt_Component);
        break;

    case Uml::lvt_Deployment_Diagram:
        createDiagram(renamedItem, Uml::dt_Deployment);
        break;

    case Uml::lvt_EntityRelationship_Diagram:
        createDiagram(renamedItem, Uml::dt_EntityRelationship);
        break;

    default:
        break;
    }
    return true;
}

/**
 * Creates a UMLObject out of the given list view item.
 */
UMLObject *UMLListView::createUMLObject(UMLListViewItem * item, Uml::Object_Type type)
{
    QString name = item->text(0);
    UMLObject * object = 0;
    switch (type) {
    case Uml::ot_UseCase:
        object = new UMLUseCase(name);
        break;

    case Uml::ot_Actor:
        object = new UMLActor(name);
        break;

    case Uml::ot_Class:
        object = new UMLClassifier(name);
        break;

    case Uml::ot_Package:
        object = new UMLPackage(name);
        break;

    case Uml::ot_Folder:
        object = new UMLFolder(name);
        break;

    case Uml::ot_Component:
        object = new UMLComponent(name);
        break;

    case Uml::ot_Node:
        object = new UMLNode(name);
        break;

    case Uml::ot_Artifact:
        object = new UMLArtifact(name);
        break;

    case Uml::ot_Interface: {
        UMLClassifier *c = new UMLClassifier(name);
        c->setBaseType(Uml::ot_Interface);
        object = c;
    }
    break;

    case Uml::ot_Datatype: {
        UMLClassifier *c = new UMLClassifier(name);
        c->setBaseType(Uml::ot_Datatype);
        object = c;
    }
    break;

    case Uml::ot_Enum:
        object = new UMLEnum(name);
        break;

    case Uml::ot_Entity:
        object = new UMLEntity(name);
        break;

    case Uml::ot_Category:
        object = new UMLCategory(name);
        break;

    default:
        uWarning() << "creating UML Object of unknown type";
        return 0;
    }

    UMLListViewItem * parentItem = static_cast<UMLListViewItem *>(item->parent());
    const Uml::ListView_Type lvt = parentItem->getType();
    if (! Model_Utils::typeIsContainer(lvt)) {
        uError() << object->name() << ": parentItem (" << lvt << " is not a container";
        delete object;
        return 0;
    }
    UMLPackage *pkg = static_cast<UMLPackage*>(parentItem->getUMLObject());
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
bool UMLListView::createChildUMLObject(UMLListViewItem * item, Uml::Object_Type type)
{
    m_bCreatingChildObject = true;
    QString text = item->text(0);
    UMLObject* parent = static_cast<UMLListViewItem *>(item->parent())->getUMLObject();
    if (!parent) {
        uError() << "parent UMLObject is 0";
        m_bCreatingChildObject = false;
        return false;
    }

    UMLObject* newObject = 0;
    if (type == Uml::ot_EnumLiteral) {
        UMLEnum *owningEnum = static_cast<UMLEnum*>(parent);
        newObject = owningEnum->createEnumLiteral(text);

        UMLEnumLiteral* enumLiteral = static_cast<UMLEnumLiteral*>(newObject);
        text = enumLiteral->toString(Uml::st_SigNoVis);
    } else if (type == Uml::ot_Template)  {
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
        text = tmplParm->toString(Uml::st_SigNoVis);
    } else if (type == Uml::ot_Attribute || type == Uml::ot_EntityAttribute)  {
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
        text = att->toString(Uml::st_SigNoVis);
    } else if (type == Uml::ot_Operation) {
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
        text = op->toString(Uml::st_SigNoVis);
    } else if (type == Uml::ot_UniqueConstraint || type == Uml::ot_ForeignKeyConstraint
               || type == Uml::ot_CheckConstraint) {

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
        case Uml::ot_UniqueConstraint:
            newObject = owningEntity->createUniqueConstraint(name);
            break;
        case Uml::ot_ForeignKeyConstraint:
            newObject = owningEntity->createForeignKeyConstraint(name);
            break;
        case Uml::ot_CheckConstraint:
            newObject = owningEntity->createCheckConstraint(name);
            break;
        default:
            break;
        }

        UMLEntityConstraint* uec = static_cast<UMLEntityConstraint*>(newObject);

        text = uec->toString(Uml::st_SigNoVis);
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
UMLView* UMLListView::createDiagram(UMLListViewItem * item, Uml::Diagram_Type type)
{
    QString name = item->text(0);
    UMLView * view = m_doc->findView(type, name);
    if (view) {
        delete item;
        return view;
    }
    UMLListViewItem *parentItem = static_cast<UMLListViewItem*>(item->parent());
    UMLFolder *parentFolder = dynamic_cast<UMLFolder*>(parentItem->getUMLObject());
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
    view->umlScene()->setOptionState(Settings::getOptionState());
    item->setID(view->umlScene()->getID());
    item->setText(name);
    view->umlScene()->activate();
    m_doc->changeCurrentView(view->umlScene()->getID());

    return view;
}

/**
 * Returns a unique name for a diagram.
 */
QString UMLListView::getUniqueDiagramName(Uml::Diagram_Type type)
{
    return m_doc->uniqViewName(type);
}

/**
 * Returns if the given name is unique for the given items type.
 */
bool UMLListView::isUnique(UMLListViewItem * item, const QString &name)
{
    UMLListViewItem * parentItem = static_cast<UMLListViewItem *>(item->parent());
    Uml::ListView_Type type = item->getType();
    switch (type) {
    case Uml::lvt_Class_Diagram:
        return !m_doc->findView(Uml::dt_Class, name);
        break;

    case Uml::lvt_Sequence_Diagram:
        return !m_doc->findView(Uml::dt_Sequence, name);
        break;

    case Uml::lvt_UseCase_Diagram:
        return !m_doc->findView(Uml::dt_UseCase, name);
        break;

    case Uml::lvt_Collaboration_Diagram:
        return !m_doc->findView(Uml::dt_Collaboration, name);
        break;

    case Uml::lvt_State_Diagram:
        return !m_doc->findView(Uml::dt_State, name);
        break;

    case Uml::lvt_Activity_Diagram:
        return !m_doc->findView(Uml::dt_Activity, name);
        break;

    case Uml::lvt_Component_Diagram:
        return !m_doc->findView(Uml::dt_Component, name);
        break;

    case Uml::lvt_Deployment_Diagram:
        return !m_doc->findView(Uml::dt_Deployment, name);
        break;

    case Uml::lvt_EntityRelationship_Diagram:
        return !m_doc->findView(Uml::dt_EntityRelationship, name);
        break;

    case Uml::lvt_Actor:
    case Uml::lvt_UseCase:
    case Uml::lvt_Node:
    case Uml::lvt_Artifact:
    case Uml::lvt_Category:
        return !m_doc->findUMLObject(name, Model_Utils::convert_LVT_OT(type));
        break;

    case Uml::lvt_Class:
    case Uml::lvt_Package:
    case Uml::lvt_Interface:
    case Uml::lvt_Datatype:
    case Uml::lvt_Enum:
    case Uml::lvt_Entity:
    case Uml::lvt_Component:
    case Uml::lvt_Subsystem:
    case Uml::lvt_Logical_Folder:
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_EntityRelationship_Folder: {
        Uml::ListView_Type lvt = parentItem->getType();
        if (!Model_Utils::typeIsContainer(lvt))
            return (m_doc->findUMLObject(name) == 0);
        UMLPackage *pkg = static_cast<UMLPackage*>(parentItem->getUMLObject());
        if (pkg == 0) {
            uError() << "internal error - "
            << "parent listviewitem is package but has no UMLObject";
            return true;
        }
        return (pkg->findObject(name) == 0);
        break;
    }

    case Uml::lvt_Template:
    case Uml::lvt_Attribute:
    case Uml::lvt_EntityAttribute:
    case Uml::lvt_Operation:
    case Uml::lvt_EnumLiteral:
    case Uml::lvt_UniqueConstraint:
    case Uml::lvt_PrimaryKeyConstraint:
    case Uml::lvt_ForeignKeyConstraint:
    case Uml::lvt_CheckConstraint: {
        UMLClassifier *parent = static_cast<UMLClassifier*>(parentItem->getUMLObject());
        return (parent->findChildObject(name) == 0);
        break;
    }

    default:
        break;
    }
    return false;
}

/**
 *
 */
void UMLListView::startRename(UMLListViewItem* item)
{
    if (m_editItem) {
        cancelRename(m_editItem);
    }
    openPersistentEditor(item, 0);
    m_editItem = item;
    m_bIgnoreCancelRename = true;
}

/**
 * Cancel rename event has occurred for the given item.
 */
void UMLListView::cancelRename(UMLListViewItem* item)
{
    // delete pointer first to lock slotItemChanged
    m_editItem = 0;
    closePersistentEditor(item, 0);
    if (!m_bIgnoreCancelRename) {
        delete item;
        m_bIgnoreCancelRename = true;
    }
}

/**
 *
 */
void UMLListView::endRename(UMLListViewItem* item)
{
    // delete pointer first to lock slotItemChanged
    m_editItem = 0;
    closePersistentEditor(item, 0);
    item->okRename(0);
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
    /*
        deleteChildrenOf( m_ucv );
        deleteChildrenOf( m_lv );
        deleteChildrenOf( m_cmpv );
        deleteChildrenOf( m_dplv );
     */
    QDomNode node = element.firstChild();
    QDomElement domElement = node.toElement();
    m_doc->writeToStatusBar(i18n("Loading listview..."));
    while (!domElement.isNull()) {
        if (domElement.tagName() == "listitem") {
            QString type = domElement.attribute("type", "-1");
            if (type == "-1")
                return false;
            Uml::ListView_Type lvType = (Uml::ListView_Type)type.toInt();
            if (lvType == Uml::lvt_View) {
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
        Uml::ListView_Type lvType = (Uml::ListView_Type)type.toInt();
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
                    uDebug() << " using id " << ID2STR(newID)
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
                UMLObject *umlParent = parent->getUMLObject();
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
            const Uml::Model_Type mt = Model_Utils::convert_LVT_MT(lvType);
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
        case Uml::lvt_Actor:
        case Uml::lvt_UseCase:
        case Uml::lvt_Class:
        case Uml::lvt_Interface:
        case Uml::lvt_Datatype:
        case Uml::lvt_Enum:
        case Uml::lvt_Entity:
        case Uml::lvt_Package:
        case Uml::lvt_Subsystem:
        case Uml::lvt_Component:
        case Uml::lvt_Node:
        case Uml::lvt_Artifact:
        case Uml::lvt_Logical_Folder:
        case Uml::lvt_UseCase_Folder:
        case Uml::lvt_Component_Folder:
        case Uml::lvt_Deployment_Folder:
        case Uml::lvt_EntityRelationship_Folder:
        case Uml::lvt_Category:
            item = findItem(nID);
            if (item == 0) {
                uError() << "INTERNAL ERROR: "
                << "findItem(id " << ID2STR(nID) << ") returns 0";
                /*
                if (pObject && pObject->getUMLPackage() &&
                        parent->getType() != Uml::lvt_Package) {
                    // Pre-1.2 file format:
                    // Objects were not nested in their packages.
                    // Synthesize the nesting here.
                    UMLPackage *umlpkg = pObject->getUMLPackage();
                    UMLListViewItem *pkgItem = findUMLObject(umlpkg);
                    if (pkgItem == 0) {
                        uDebug() << "synthesizing ListViewItem for package "
                            << ID2STR(umlpkg->getID());
                        pkgItem = new UMLListViewItem(parent, umlpkg->getName(),
                                                      Uml::lvt_Package, umlpkg);
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
                uDebug() << item->getText() << " parent "
                << parent->getText() << " (" << parent << ") != "
                << itmParent->getText() << " (" << itmParent << ")";
                if (item == m_datatypeFolder && itmParent == m_lv[Uml::mt_Logical]) {
                    uDebug() << "Reparenting the Datatypes folder is prohibited";
                } else {
                    UMLListViewItem *newItem = moveObject(nID, lvType, parent);
                    item = newItem;
                    if (item) {
                        uDebug() << "Attempted reparenting of " << item->getText()
                        << "(current parent: " << (itmParent ? itmParent->getText() : "0")
                        << ", new parent: " << parent->getText() << ")";
                    }
                }
            }
            break;
        case Uml::lvt_Attribute:
        case Uml::lvt_EntityAttribute:
        case Uml::lvt_Template:
        case Uml::lvt_Operation:
        case Uml::lvt_EnumLiteral:
        case Uml::lvt_UniqueConstraint:
        case Uml::lvt_PrimaryKeyConstraint:
        case Uml::lvt_ForeignKeyConstraint:
        case Uml::lvt_CheckConstraint:
            item = findItem(nID);
            if (item == 0) {
                uDebug() << "item " << ID2STR(nID) << " (of type "
                << lvType << ") does not yet exist...";
                UMLObject* umlObject = parent->getUMLObject();
                if (!umlObject) {
                    uDebug() << "And also the parent->getUMLObject() does not exist";
                    return false;
                }
                if (nID == Uml::id_None) {
                    uWarning() << "lvtype " << lvType << " has id -1";
                } else {
                    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(umlObject);
                    if (classifier) {
                        umlObject = classifier->findChildObjectById(nID);
                        if (umlObject) {
                            connectNewObjectsSlots(umlObject);
                            label = umlObject->name();
                            item = new UMLListViewItem(parent, label, lvType, umlObject);
                        } else {
                            uDebug() << "lvtype " << lvType << " child object "
                                << ID2STR(nID) << " not found";
                        }
                    } else {
                        uDebug() << "cast to classifier object failed";
                    }
                }
            }
            break;
        case Uml::lvt_Logical_View:
            item = m_lv[Uml::mt_Logical];
            break;
        case Uml::lvt_Datatype_Folder:
            item = m_datatypeFolder;
            break;
        case Uml::lvt_UseCase_View:
            item = m_lv[Uml::mt_UseCase];
            break;
        case Uml::lvt_Component_View:
            item = m_lv[Uml::mt_Component];
            break;
        case Uml::lvt_Deployment_View:
            item = m_lv[Uml::mt_Deployment];
            break;
        case Uml::lvt_EntityRelationship_Model:
            item = m_lv[Uml::mt_EntityRelationship];
            break;
        default:
            if (Model_Utils::typeIsDiagram(lvType)) {
                item = new UMLListViewItem(parent, label, lvType, nID);
            } else {
                uError() << "INTERNAL ERROR: unexpected listview type "
                << lvType << " (ID " << ID2STR(nID) << ")";
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
            << " of lvtype " << lvType;
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
UMLListViewItem *UMLListView::rootView(Uml::ListView_Type type)
{
    UMLListViewItem *theView = 0;
    switch (type) {
    case Uml::lvt_View:
        theView = m_rv;
        break;
    case Uml::lvt_Logical_View:
        theView = m_lv[Uml::mt_Logical];
        break;
    case Uml::lvt_UseCase_View:
        theView = m_lv[Uml::mt_UseCase];
        break;
    case Uml::lvt_Component_View:
        theView = m_lv[Uml::mt_Component];
        break;
    case Uml::lvt_Deployment_View:
        theView = m_lv[Uml::mt_Deployment];
        break;
    case Uml::lvt_EntityRelationship_Model:
        theView = m_lv[Uml::mt_EntityRelationship];
        break;
    case Uml::lvt_Datatype_Folder:   // @todo fix asymmetric naming
        theView = m_datatypeFolder;
        break;
    default:
        break;
    }
    return theView;
}

/**
 * Deletes all child-items of @p parent.
 */
void UMLListView::deleteChildrenOf(UMLListViewItem* parent)
{
    if (!parent) {
        return;
    }
    if (parent == m_lv[Uml::mt_Logical])
        return;//m_datatypeFolder = 0;
    for (int i=0; i < parent->childCount(); i++)
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
    UMLObject *object = temp->getUMLObject();
    Uml::ListView_Type lvt = temp->getType();
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
            /**
             * We cannot just delete canvasObj here: What if the object
             * is still being used by others (for example, as a parameter
             * or return type of an operation) ?
             * Deletion should not have been permitted in the first place
             * if the object still has users - but Umbrello is lacking
             * that logic.
             */
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
        UMLListViewItem* item = (UMLListViewItem*)itemAt(event->pos());
        if (!item) {
            uDebug() << "itemAt(mousePoint) returns 0";
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

#include "umllistview.moc"
