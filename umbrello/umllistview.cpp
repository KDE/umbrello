/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umllistview.h"

// app includes
#include "actor.h"
#include "classifier.h"
#include "cmds.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "package.h"
#include "folder.h"
#include "component.h"
#include "node.h"
#include "artifact.h"
#include "enum.h"
#include "enumliteral.h"
#include "entity.h"
#include "category.h"
#include "datatype.h"
#include "docwindow.h"
#include "layoutgenerator.h"
#include "umllistviewpopupmenu.h"
#include "template.h"
#include "operation.h"
#include "attribute.h"
#include "entityattribute.h"
#include "instance.h"
#include "instanceattribute.h"
#include "uniqueconstraint.h"
#include "foreignkeyconstraint.h"
#include "checkconstraint.h"
#include "uml.h"
#include "umlclipboard.h"
#include "umldoc.h"
#include "umllistviewitemlist.h"
#include "umllistviewitem.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlviewimageexporter.h"
#include "usecase.h"
#include "model_utils.h"
#include "models/diagramsmodel.h"
#include "optionstate.h"
#include "uniqueid.h"
#include "idchangelog.h"
#include "umldragdata.h"
#include "classpropertiesdialog.h"
#include "umlattributedialog.h"
#include "umlentityattributedialog.h"
#include "umloperationdialog.h"
#include "umltemplatedialog.h"
#include "umluniqueconstraintdialog.h"
#include "umlforeignkeyconstraintdialog.h"
#include "umlcheckconstraintdialog.h"
#include "object_factory.h"

// kde includes
#if QT_VERSION < 0x050000
#include <kfiledialog.h>
#endif
#include <KLocalizedString>
#include <KMessageBox>
#if QT_VERSION < 0x050000
#include <ktabwidget.h>
#endif

// qt includes
#include <QApplication>
#include <QDrag>
#include <QDropEvent>
#include <QEvent>
#if QT_VERSION >= 0x050000
#include <QFileDialog>
#endif
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointer>
#include <QRegExp>
#include <QPoint>
#include <QRect>
#include <QToolTip>
#include <QXmlStreamWriter>

DEBUG_REGISTER(UMLListView)

/**
 * Constructs the tree view.
 *
 * @param parent   The parent to this.
 */
UMLListView::UMLListView(QWidget *parent)
  : QTreeWidget(parent),
    m_rv(nullptr),
    m_datatypeFolder(nullptr),
    m_settingsFolder(nullptr),
    m_doc(UMLApp::app()->document()),
    m_bStartedCut(false),
    m_bStartedCopy(false),
    m_bCreatingChildObject(false),
    m_dragStartPosition(QPoint()),
    m_dragCopyData(nullptr)
{
    // setup list view
    setAcceptDrops(true);
    //setDropVisualizer(false);
    //setItemsMovable(true);
    //setItemsRenameable(true);
    setSelectionMode(ExtendedSelection);
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
        m_lv[i] = nullptr;
    }

    //setup slots/signals
    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(slotCollapsed(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(slotExpanded(QTreeWidgetItem*)));
    connect(UMLApp::app(), SIGNAL(sigCutSuccessful()), this, SLOT(slotCutSuccessful()));
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));
}

/**
 * Standard destructor.
 */
UMLListView::~UMLListView()
{
    clean(); // m_lv
    delete m_datatypeFolder;
    delete m_settingsFolder;
    delete m_rv;
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
 * Handler for item selection changed signals.
 */
void UMLListView::slotItemSelectionChanged()
{
    UMLListViewItem* currItem = static_cast<UMLListViewItem*>(currentItem());
    if (currItem && currItem->isSelected()) {
        DEBUG(DBG_SRC) << "UMLListView selection changed to" << currItem->text(0);
        // Update current view to selected object's view
        if (Model_Utils::typeIsDiagram(currItem->type())) {
            // If the user navigates to a diagram, load the diagram just like what
            // would happen when clicking on it (includes saving/showing the documentation)
            m_doc->changeCurrentView(currItem->ID());
        } else {
            // If the user navigates to any other item, save the current object's
            // documentation and show selected object's documentation
            UMLApp::app()->docWindow()->showDocumentation(currItem->umlObject(), true);
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
 * Handler for mouse press events.
 * @param me   the mouse event
 */
void UMLListView::mousePressEvent(QMouseEvent *me)
{
    UMLView *currentView = UMLApp::app()->currentView();
    Q_ASSERT(currentView);
    UMLScene *scene = currentView->umlScene();
    Q_ASSERT(scene);
    scene->clearSelected();
    if (me->modifiers() != Qt::ShiftModifier)
        clearSelection();

    // Get the UMLListViewItem at the point where the mouse pointer was pressed
    UMLListViewItem * item = static_cast<UMLListViewItem*>(itemAt(me->pos()));
    if (item) {
        DEBUG(DBG_SRC) << "QMouseEvent on" << UMLListViewItem::toString(item->type());
    }
    else {
        DEBUG(DBG_SRC) << "QMouseEvent on empty space";
    }

    const Qt::MouseButton button = me->button();

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

    QTreeWidget::mousePressEvent(me);
}

/**
 * Handler for mouse move events.
 * @param me   the mouse event
 */
void UMLListView::mouseMoveEvent(QMouseEvent* me)
{
    if (!(me->buttons() & Qt::LeftButton)) {
        DEBUG(DBG_SRC) << "not LeftButton (no action)";
        return;
    }
    if ((me->pos() - m_dragStartPosition).manhattanLength()
            < QApplication::startDragDistance()) {
        DEBUG(DBG_SRC) << "pos change since dragStart is below startDragDistance threshold (no action)";
        return;
    }

    DEBUG(DBG_SRC) << "initiating drag";

    // Store a copy of selected list items in case the user
    // will ctrl-drag (basically just copy/paste) an item
    //
    // The QDrag mime data is used for moving items onto the diagram
    // or internally in the tree view
    UMLClipboard clipboard;
    if ((m_dragCopyData = clipboard.copy(false)) == 0) {
        // This should never happen, this is just like using ctrl+c on the list view item
        uError() << "Unable to obtain mime data for copy-drag operation";
    }

    QDrag* drag = new QDrag(this);
    drag->setMimeData(getDragData());
    drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::MoveAction);
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
    m_doc->changeCurrentView(item->ID());
    UMLView *view = m_doc->findView(item->ID());
    if (view && view->umlScene())
        UMLApp::app()->docWindow()->showDocumentation(view->umlScene(), false);
    QTreeWidget::mouseReleaseEvent(me);
}

/**
 * Handler for key press events.
 * @param ke   the key event
 */
void UMLListView::keyPressEvent(QKeyEvent *ke)
{
    QTreeWidget::keyPressEvent(ke); // let parent handle it
    const int k = ke->key();
    if (k == Qt::Key_Delete || k == Qt::Key_Backspace) {
        slotDeleteSelectedItems();
    } else if (k == Qt::Key_F3) {
        // preliminary support for layout generator
        LayoutGenerator r;
        if (!r.generate(UMLApp::app()->currentView()->umlScene()))
            return;
        r.apply(UMLApp::app()->currentView()->umlScene());
    }
}

/**
 * Called when a right mouse button menu has an item selected.
 * @param action   the selected action
 * @param position the position of the menu on the diagram (only used for multi selection "Show")
 */
void UMLListView::slotMenuSelection(QAction* action, const QPoint &position)
{
    UMLListViewItem * currItem = static_cast<UMLListViewItem*>(currentItem());
    if (!currItem) {
        DEBUG(DBG_SRC) << "Invoked without currently selectedItem!";
        return;
    }
    UMLListViewItem::ListViewType lvt = currItem->type();
    ListPopupMenu::MenuType menuType = ListPopupMenu::typeFromAction(action);

    switch (menuType) {
    case ListPopupMenu::mt_Activity_Diagram:
        addNewItem(currItem, UMLListViewItem::lvt_Activity_Diagram);
        break;

    case ListPopupMenu::mt_Class_Diagram:
        addNewItem(currItem, UMLListViewItem::lvt_Class_Diagram);
        break;

    case ListPopupMenu::mt_Collaboration_Diagram:
        addNewItem(currItem, UMLListViewItem::lvt_Collaboration_Diagram);
        break;

    case ListPopupMenu::mt_Component_Diagram:
        addNewItem(currItem, UMLListViewItem::lvt_Component_Diagram);
        break;

    case ListPopupMenu::mt_Deployment_Diagram:
        addNewItem(currItem, UMLListViewItem::lvt_Deployment_Diagram);
        break;

    case ListPopupMenu::mt_EntityRelationship_Diagram:
        addNewItem(currItem, UMLListViewItem::lvt_EntityRelationship_Diagram);
        break;

    case ListPopupMenu::mt_Sequence_Diagram:
        addNewItem(currItem, UMLListViewItem::lvt_Sequence_Diagram);
        break;

    case ListPopupMenu::mt_State_Diagram:
        addNewItem(currItem, UMLListViewItem::lvt_State_Diagram);
        break;

    case ListPopupMenu::mt_UseCase_Diagram:
        addNewItem(currItem, UMLListViewItem::lvt_UseCase_Diagram);
        break;

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

    case ListPopupMenu::mt_Port:
        if (Settings::optionState().generalState.uml2)
            addNewItem(currItem, UMLListViewItem::lvt_Port);
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

    case ListPopupMenu::mt_Instance:
        addNewItem(currItem, UMLListViewItem::lvt_Instance);
        break;

    case ListPopupMenu::mt_Category:
        addNewItem(currItem, UMLListViewItem::lvt_Category);
        break;

    case ListPopupMenu::mt_DisjointSpecialisation:
        {
            UMLCategory* catObj = currItem->umlObject()->asUMLCategory();
            catObj->setType(UMLCategory::ct_Disjoint_Specialisation);
        }
        break;

    case ListPopupMenu::mt_OverlappingSpecialisation:
        {
            UMLCategory* catObj = currItem->umlObject()->asUMLCategory();
            catObj->setType(UMLCategory::ct_Overlapping_Specialisation);
        }
        break;

    case ListPopupMenu::mt_Union:
        {
            UMLCategory* catObj = currItem->umlObject()->asUMLCategory();
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
    case ListPopupMenu::mt_InstanceAttribute:
        addNewItem(currItem, UMLListViewItem::lvt_InstanceAttribute);
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

    case ListPopupMenu::mt_Import_Class:
        UMLApp::app()->slotImportClass();
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
        {
            const Uml::ID::Type id = currItem->ID();
            UMLView *view = m_doc->findView(id);
            if (view) {
                if (view->umlScene())
                    view->umlScene()->getImageExporter()->exportView();
                else
                    uError() << "ListPopupMenu::mt_Export_Image: view " << Uml::ID::toString(id)
                             << " umlScene() is NULL";
            } else {
                uError() << "ListPopupMenu::mt_Export_Image: m_doc->findView("
                         << Uml::ID::toString(id) << " returns NULL";
            }
        }
        break;

    case ListPopupMenu::mt_Externalize_Folder:
        {
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            UMLFolder *modelFolder = current->umlObject()->asUMLFolder();
            if (modelFolder == 0) {
                uError() << "modelFolder is 0";
                return;
            }
            // configure & show the file dialog
#if QT_VERSION >= 0x050000
            const QString rootDir(m_doc->url().adjusted(QUrl::RemoveFilename).path());
            QPointer<QFileDialog> fileDialog = new QFileDialog(this, i18n("Externalize Folder"), rootDir, QLatin1String("*.xml"));
#else
            const QString rootDir(m_doc->url().directory());
            QPointer<KFileDialog> fileDialog = new KFileDialog(rootDir, QLatin1String("*.xml"), this);
            fileDialog->setCaption(i18n("Externalize Folder"));
            fileDialog->setOperationMode(KFileDialog::Other);
#endif
            // set a sensible default filename
            QString defaultFilename = current->text(0).toLower();
            defaultFilename.replace(QRegExp(QLatin1String("\\W+")), QLatin1String("_"));
            defaultFilename.append(QLatin1String(".xml"));  // default extension
#if QT_VERSION >= 0x050000
            fileDialog->selectFile(defaultFilename);
            QList<QUrl> selURL;
            if (fileDialog->exec() == QDialog::Accepted) {
                selURL = fileDialog->selectedUrls();
            }
#else
            fileDialog->setSelection(defaultFilename);
            KUrl selURL;
            if (fileDialog->exec() == QDialog::Accepted) {
                selURL = fileDialog->selectedUrl();
            }
#endif
            delete fileDialog;
            if (selURL.isEmpty())
                return;
#if QT_VERSION >= 0x050000
            QString path = selURL[0].toLocalFile();
#else
            QString path = selURL.toLocalFile();
#endif
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
            folderText.remove(QRegExp(QLatin1String("\\s*\\(.*$")));
            folderText.append(QLatin1String(" (") + fileName + QLatin1Char(')'));
            current->setText(folderText);
            break;
        }

    case ListPopupMenu::mt_Internalize_Folder:
        {
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            UMLFolder *modelFolder = current->umlObject()->asUMLFolder();
            if (modelFolder == 0) {
                uError() << "modelFolder is 0";
                return;
            }
            modelFolder->setFolderFile(QString());
            // Recompute text of the folder
            QString folderText = current->text(0);
            folderText.remove(QRegExp(QLatin1String("\\s*\\(.*$")));
            current->setText(folderText);
            break;
        }

    case ListPopupMenu::mt_Model:
        {
            QString name = m_doc->name();
            bool ok = Dialog_Utils::askName(i18n("Enter Model Name"),
                                            i18n("Enter the new name of the model:"),
                                            name);
            if (ok) {
                setTitle(0, name);
                m_doc->setName(name);
            }
            break;
        }

    case ListPopupMenu::mt_Open_File: {
        UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
        UMLArtifact *artifact = current->umlObject()->asUMLArtifact();
        if (artifact == nullptr) {
            uError() << "artifact is 0";
            return;
        }
        QUrl file = QUrl::fromLocalFile(artifact->fullPath());
        UMLApp::app()->slotOpenFileInEditor(file);
        break;
    }

    case ListPopupMenu::mt_Rename:
        edit(currentIndex());
        break;

    case ListPopupMenu::mt_Delete:
        deleteItem(currItem);
        break;

    case ListPopupMenu::mt_Show:
        // first check if we are on a diagram
        if (!Model_Utils::typeIsDiagram(lvt)) {
            UMLObject * object = currItem->umlObject();
            if (!object) {
                uError() << "UMLObjet of ... is null! Doing nothing.";
                return;
            }
            QList<UMLWidget*> findResults;

            if (Model_Utils::typeIsCanvasWidget(lvt)) {
                UMLViewList views = m_doc->viewIterator();
                foreach (UMLView *view, views) {
                    foreach(UMLWidget *widget, view->umlScene()->widgetList()) {
                        if (object == widget->umlObject()) {
                            findResults.append(widget);
                        }
                    }
                }
            }

            if (findResults.size() == 0)  {
                break;
            }

            UMLWidget *selectedResult = 0;

            if (findResults.size() > 1) {
                QMenu menu(this);
                int i = 0;
                foreach(UMLWidget *w, findResults) {
                    QAction *action = menu.addAction(w->umlScene()->name() + QLatin1Char(':') + w->name());
                    action->setData(i++);
                }
                QAction *action = menu.exec(position);
                if (action) {
                    selectedResult = findResults.at(action->data().toInt());
                }
            } else {
                selectedResult = findResults.first();
            }

            if (!selectedResult) {
                break;
            }

            UMLView *view = selectedResult->umlScene()->activeView();
            selectedResult->umlScene()->setIsOpen(true);
            view->setZoom(100);
            if (UMLApp::app()->currentView() != view) {
                UMLApp::app()->setCurrentView(view, false);
            }
            view->centerOn(selectedResult->scenePos());
            selectedResult->setSelected(true);
        }
        break;

    case ListPopupMenu::mt_Properties:
        if (Model_Utils::typeIsProperties(lvt)) {
            UMLApp::app()->slotPrefs(Model_Utils::convert_LVT_PT(lvt));
            return;
        }
        else if (Model_Utils::typeIsDiagram(lvt)) {
            UMLView * pView = m_doc->findView(currItem->ID());
            if (pView) {
                UMLApp::app()->docWindow()->updateDocumentation(false);
                pView->showPropertiesDialog();
                UMLApp::app()->docWindow()->showDocumentation(pView->umlScene(), true);
            }
            return;
        }

        { // ok, we are on another object, so find out on which one
            UMLObject * object = currItem->umlObject();
            if (!object) {
                uError() << "UMLObject of ... is null! Doing nothing.";
                return;
            }
            object->showPropertiesDialog();
        }
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

    case ListPopupMenu::mt_Clone:
        UMLApp::app()->slotEditCopy();
        UMLApp::app()->slotEditPaste();
        break;

    case ListPopupMenu::mt_ChangeToClass:
        {
            UMLClassifier* o = currItem->umlObject()->asUMLClassifier();
            o->setStereotypeCmd(QString());
            currItem->updateObject();
        }
        break;
    case ListPopupMenu::mt_ChangeToPackage:
        {
            UMLClassifier* o = currItem->umlObject()->asUMLClassifier();
            o->setStereotypeCmd(QString());
            o->setBaseType(UMLObject::ot_Package);
            currItem->updateObject();
        }
        break;

    case ListPopupMenu::mt_Undefined:
        // We got signalled for a menu action, but that menu action was not
        // defined in ListPopupMenu. This is the case for "create diagram"
        // actions which are defined and handled in UMLApp. This is fine,
        // ignore them without warning.
        break;

    default:
        uError() << "unknown type" << menuType;

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
UMLListViewItem *UMLListView::findFolderForDiagram(Uml::DiagramType::Enum dt)
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
void UMLListView::slotDiagramCreated(Uml::ID::Type id)
{
    if (findItem(id)) {
        uDebug() << "list view item " << Uml::ID::toString(id) << " already exists";
        return;
    }
    UMLView *v = m_doc->findView(id);
    if (v) {
        UMLScene *scene = v->umlScene();
        if (scene) {
            const Uml::DiagramType::Enum dt = scene->type();
            UMLListViewItem* p = findUMLObject(scene->folder());
            UMLListViewItem* item = new UMLListViewItem(p, scene->name(), Model_Utils::convert_DT_LVT(dt), id);
            item->setSelected(true);
            UMLApp::app()->docWindow()->showDocumentation(scene, false);
        }
    } else {
        uError() << "UmlDoc::findView(" << Uml::ID::toString(id) << ") returns NULL";
    }
}

/**
 * Determine the parent ListViewItem given a UMLObject.
 *
 * @param object   Pointer to the UMLObject for which to look up the parent.
 * @return    Pointer to the parent UMLListViewItem chosen.
 *            Returns NULL on error (no parent could be determined.)
 */
UMLListViewItem* UMLListView::determineParentItem(UMLObject* object) const
{
    UMLListViewItem* parentItem = 0;
    UMLPackage*      pkg = 0;
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
    case UMLObject::ot_InstanceAttribute:
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
    default:
        pkg = object->umlPackage();
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
            Uml::ModelType::Enum guess = Model_Utils::guessContainer(object);
            parentItem = m_lv[guess];
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
    // case UMLObject::ot_Instance:
    //   Must be handled separately because UMLInstanceAttribute is not a
    //   UMLClassifierListItem.
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
        // ClassifierListPage::slot{Up, Down}Clicked()
        return;
    }

    if (object->baseType() == UMLObject::ot_Association)
        return;

    UMLListViewItem* newItem = findUMLObject(object);

    if (newItem) {
        DEBUG(DBG_SRC) << object->name() << ", type=" << newItem->type()
                       << ", id=" << Uml::ID::toString(object->id())
                       << ": item already exists.";
        Icon_Utils::IconType icon = Model_Utils::convert_LVT_IT(newItem->type());
        newItem->setIcon(icon);
        return;
    }
    UMLListViewItem* parentItem = 0;
    UMLPackage *p = object->umlPackage();
    if (p) {
        parentItem = findUMLObject(p);
        if (parentItem == 0)
            parentItem = determineParentItem(object);
    } else {
        uWarning() << object->name() << " : umlPackage not set on object";
        parentItem = determineParentItem(object);
    }
    if (parentItem == 0)
        return;
    UMLObject::ObjectType type = object->baseType();
    if (type == UMLObject::ot_Datatype) {
        UMLDatatype *dt = object->asUMLDatatype();
        if (!dt->isActive()) {
            DEBUG(DBG_SRC) << object->name() << " is not active. Refusing to create UMLListViewItem";
            return;
        }
    }

    connectNewObjectsSlots(object);
    const UMLListViewItem::ListViewType lvt = Model_Utils::convert_OT_LVT(object);
    QString name = object->name();
    if (type == UMLObject::ot_Folder) {
        UMLFolder *f = object->asUMLFolder();
        QString folderFile = f->folderFile();
        if (!folderFile.isEmpty())
            name.append(QLatin1String(" (") + folderFile + QLatin1Char(')'));
    }
    newItem = new UMLListViewItem(parentItem, name, lvt, object);
    if (mayHaveChildItems(type)) {
        UMLClassifier *c = object->asUMLClassifier();
        UMLClassifierListItemList cListItems = c->getFilteredList(UMLObject::ot_UMLObject);
        foreach(UMLClassifierListItem *cli, cListItems)
            childObjectAdded(cli, c);
    }
    if (m_doc->loading())
        return;
    scrollToItem(newItem);
    newItem->setOpen(true);
    clearSelection();
    newItem->setSelected(true);
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
    case UMLObject::ot_Interface:
        {
            UMLClassifier *c = object->asUMLClassifier();
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
    case UMLObject::ot_Instance:
        {
            connect(object, SIGNAL(modified()), this, SLOT(slotObjectChanged()));
        }
        break;
    case UMLObject::ot_Enum:
        {
            UMLEnum *e = object->asUMLEnum();
            connect(e, SIGNAL(enumLiteralAdded(UMLClassifierListItem*)),
                    this, SLOT(childObjectAdded(UMLClassifierListItem*)));
            connect(e, SIGNAL(enumLiteralRemoved(UMLClassifierListItem*)),
                    this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
        }
        connect(object, SIGNAL(modified()), this, SLOT(slotObjectChanged()));
        break;
    case UMLObject::ot_Entity:
        {
            UMLEntity *ent = object->asUMLEntity();
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
    case UMLObject::ot_InstanceAttribute:
    case UMLObject::ot_UniqueConstraint:
    case UMLObject::ot_ForeignKeyConstraint:
    case UMLObject::ot_CheckConstraint:
    case UMLObject::ot_Package:
    case UMLObject::ot_Actor:
    case UMLObject::ot_UseCase:
    case UMLObject::ot_Component:
    case UMLObject::ot_Port:
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
            childItem->setSelected(true);
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
void UMLListView::slotDiagramRenamed(Uml::ID::Type id)
{
    UMLListViewItem* item;
    UMLView* v = m_doc->findView(id);
    if ((item = findView(v)) == 0) {
        uError() << "UMLDoc::findView(" << Uml::ID::toString(id) << ") returns 0";
        return;
    }
    item->setText(v->umlScene()->name());
}

/**
 * Sets the document this is associated with.  This is important as
 * this is required as to set up the callbacks.
 *
 * @param doc   The document to associate with this class.
 */
void UMLListView::setDocument(UMLDoc *doc)
{
    if (m_doc && m_doc != doc) {
        //disconnect signals from old doc and reset view
    }
    m_doc = doc;

    connect(m_doc, SIGNAL(sigDiagramCreated(Uml::ID::Type)), this, SLOT(slotDiagramCreated(Uml::ID::Type)));
    connect(m_doc, SIGNAL(sigDiagramRemoved(Uml::ID::Type)), this, SLOT(slotDiagramRemoved(Uml::ID::Type)));
    connect(m_doc, SIGNAL(sigDiagramRenamed(Uml::ID::Type)), this, SLOT(slotDiagramRenamed(Uml::ID::Type)));
    connect(m_doc, SIGNAL(sigObjectCreated(UMLObject*)),   this, SLOT(slotObjectCreated(UMLObject*)));
    connect(m_doc, SIGNAL(sigObjectRemoved(UMLObject*)),   this, SLOT(slotObjectRemoved(UMLObject*)));
}

/**
 * Disconnects signals and removes the list view item.
 * @param object the object about to be removed
 */
void UMLListView::slotObjectRemoved(UMLObject* object)
{
    if (m_doc->loading() && !m_doc->importing()) { //needed for class wizard but not when importing
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
void UMLListView::slotDiagramRemoved(Uml::ID::Type id)
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
 * This method looks for an object in a folder an its subfolders recursively.
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
        case UMLListViewItem::lvt_Port :
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
 * Find a UMLObject in the listview.
 *
 * @param p   Pointer to the object to find in the list view.
 * @return    Pointer to the UMLObject found or NULL if not found.
 */
UMLListViewItem * UMLListView::findUMLObject(const UMLObject *p) const
{
    UMLListViewItem *item = m_rv;
    UMLListViewItem *testItem = item->findUMLObject(p);
    if (testItem)
        return testItem;
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
    Uml::DiagramType::Enum dType = v->umlScene()->type();
    UMLListViewItem::ListViewType type = Model_Utils::convert_DT_LVT(dType);
    Uml::ID::Type id = v->umlScene()->ID();
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
    if (m_doc->loading()) {
        DEBUG(DBG_SRC) << "could not find " << v->umlScene()->name() << " in " << *item;
    } else {
        uWarning() << "could not find " << v->umlScene()->name() << " in " << *item;
    }
    return 0;
}

/**
 * Searches the tree for a diagram (view).
 * Warning: these method may return in some cases the wrong diagram
 * Used by findView().
 */
UMLListViewItem* UMLListView::recursiveSearchForView(UMLListViewItem* listViewItem,
        UMLListViewItem::ListViewType type, Uml::ID::Type id)
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
        if (listViewItem->type() == type && listViewItem->ID() == id)
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
UMLListViewItem* UMLListView::findItem(Uml::ID::Type id)
{
    UMLListViewItem *topLevel = m_rv;
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
            Uml::ModelType::Enum mt = Uml::ModelType::fromInt(i);
            UMLFolder *sysFolder = m_doc->rootFolder(mt);
            UMLListViewItem::ListViewType lvt = Model_Utils::convert_MT_LVT(mt);
            m_lv[i] = new UMLListViewItem(m_rv, sysFolder->localName(), lvt, sysFolder);
        }
    } else {
        clean();
    }
    UMLFolder *datatypeFolder = m_doc->datatypeFolder();
    if (!m_datatypeFolder) {
        m_datatypeFolder = new UMLListViewItem(m_lv[Uml::ModelType::Logical], datatypeFolder->localName(),
                                           UMLListViewItem::lvt_Datatype_Folder, datatypeFolder);
    }
    if (m_settingsFolder == 0) {
        m_settingsFolder =  new UMLListViewItem(this, i18n("Settings"), UMLListViewItem::lvt_Properties);
        Icon_Utils::IconType icon = Model_Utils::convert_LVT_IT(m_settingsFolder->type());
        m_settingsFolder->setIcon(icon);
        m_settingsFolder->setID("Settings");
        new UMLListViewItem(m_settingsFolder, i18n("Auto Layout"), UMLListViewItem::lvt_Properties_AutoLayout);
        new UMLListViewItem(m_settingsFolder, i18n("Class"), UMLListViewItem::lvt_Properties_Class);
        new UMLListViewItem(m_settingsFolder, i18n("Code Importer"), UMLListViewItem::lvt_Properties_CodeImport);
        new UMLListViewItem(m_settingsFolder, i18n("Code Generation"), UMLListViewItem::lvt_Properties_CodeGeneration);
        new UMLListViewItem(m_settingsFolder, i18n("Code Viewer"), UMLListViewItem::lvt_Properties_CodeViewer);
        new UMLListViewItem(m_settingsFolder, i18n("Font"), UMLListViewItem::lvt_Properties_Font);
        new UMLListViewItem(m_settingsFolder, i18n("General"), UMLListViewItem::lvt_Properties_General);
        new UMLListViewItem(m_settingsFolder, i18n("User Interface"), UMLListViewItem::lvt_Properties_UserInterface);
    }

    m_rv->setOpen(true);
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_lv[i]->setOpen(true);
    }
    m_datatypeFolder->setOpen(false);

    //setup misc.
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
        temp->setSelected(true);
}

/**
 * Event handler for mouse double click.
 */
void UMLListView::mouseDoubleClickEvent(QMouseEvent * me)
{
    UMLListViewItem * item = static_cast<UMLListViewItem *>(currentItem());
    if (!item || me->button() != Qt::LeftButton)
        return;

    UMLListViewItem::ListViewType lvType = item->type();
    if (Model_Utils::typeIsProperties(lvType)) {
        UMLApp::app()->docWindow()->updateDocumentation(false);
        UMLApp::app()->slotPrefs(Model_Utils::convert_LVT_PT(lvType));
        return;
    }
    else if (Model_Utils::typeIsDiagram(lvType)) {
        UMLView * pView = m_doc->findView(item->ID());
        if (pView) {
            UMLApp::app()->docWindow()->updateDocumentation(false);
            pView->showPropertiesDialog();
            UMLApp::app()->docWindow()->showDocumentation(pView->umlScene(), true);
        }
        return;
    }
    //else see if an object
    UMLObject * object = item->umlObject();
    //continue only if we are on a UMLObject
    if (!object) {
        return;
    }

    object->showPropertiesDialog(this);
}

/**
 * Event handler for accepting drag request.
 * @param event   the drop event
 * @return success state
 */
bool UMLListView::acceptDrag(QDropEvent* event) const
{
    UMLListViewItem* target = (UMLListViewItem*)itemAt(event->pos());
    if (!target) {
        DEBUG(DBG_SRC) << "itemAt(mouse position) returns 0";
        return false;
    }

    bool accept = false;
    UMLListViewItem::ListViewType srcType = UMLListViewItem::lvt_Unknown;
    UMLListViewItem::ListViewType dstType = UMLListViewItem::lvt_Unknown;

    // Handle different drop actions
    switch (event->proposedAction()) {
        case Qt::CopyAction: {
            // Instead of relying on the current item being the drag source,
            // we should use the mime data to obtain the dragged type (or types
            // if we implement multiple item selections in tree view)
            srcType = static_cast<UMLListViewItem*>(currentItem())->type();
            dstType = target->type();

            // Copy of diagrams is not supported
            if (Model_Utils::typeIsDiagram(srcType)) {
                accept = false;
            } else {
                accept = Model_Utils::typeIsAllowedInType(srcType, dstType);
            }
            break;
        }
        case Qt::MoveAction: {
            UMLDragData::LvTypeAndID_List list;
            if (!UMLDragData::getClip3TypeAndID(event->mimeData(), list)) {
                uError() << "UMLDragData::getClip3TypeAndID returns false";
                return false;
            }

            UMLDragData::LvTypeAndID_It it(list);
            UMLDragData::LvTypeAndID * data = 0;
            dstType = target->type();
            while (it.hasNext()) {
                data = it.next();
                srcType = data->type;
                accept = Model_Utils::typeIsAllowedInType(srcType, dstType);

                // disallow drop if any child element is not allowed
                if (!accept)
                    break;
            }
            break;
        }
        default: {
            uError() << "Unsupported drop-action in acceptDrag()";
            return false;
        }
    }

    if (!accept) {
        uDebug() << "Disallowing drop because source type" << UMLListViewItem::toString(srcType)
                 << "is not allowed in target type" << UMLListViewItem::toString(dstType);
    }

    return accept;
}

/**
 * Auxiliary method for moveObject(): Adds the model object at the proper
 * new container (package if nested, UMLDoc if at global level), and
 * updates the containment relationships in the model.
 */
void UMLListView::addAtContainer(UMLListViewItem *item, UMLListViewItem *parent)
{
    UMLCanvasObject *o = item->umlObject()->asUMLCanvasObject();
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
        UMLPackage *pkg = parent->umlObject()->asUMLPackage();
        o->setUMLPackage(pkg);
        pkg->addObject(o);
    } else {
        uError() << item->text(0) << ": parent type is " << parent->type();
    }
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView)
        currentView->umlScene()->updateContainment(o);
}

/**
 * Moves an object given its unique ID and listview type to an
 * other listview parent item.
 * Also takes care of the corresponding move in the model.
 */
UMLListViewItem * UMLListView::moveObject(Uml::ID::Type srcId, UMLListViewItem::ListViewType srcType,
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
    else if (Model_Utils::typeIsDiagram(srcType)) {
        UMLView *v = m_doc->findView(srcId);
        UMLFolder *newParentObj = newParent->umlObject()->asUMLFolder();
        if (v) {
            UMLFolder *srcPkg = v->umlScene()->folder();
            if (srcPkg) {
                if (srcPkg == newParentObj) {
                    uError() << v->umlScene()->name() << ": Object is already in target package";
                    return 0;
                }
                srcPkg->removeView(v);
                newParentObj->addView(v);
                v->umlScene()->setFolder(newParentObj);
                UMLApp::app()->document()->diagramsModel()->emitDataChanged(v);
            }
        }
    }

    UMLListViewItem::ListViewType newParentType = newParent->type();
    DEBUG(DBG_SRC) << "newParentType is " << UMLListViewItem::toString(newParentType);
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
    case UMLListViewItem::lvt_Port:
        if (newParentType == UMLListViewItem::lvt_Component) {
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
    case UMLListViewItem::lvt_Object_Diagram:
        if (newParentType == UMLListViewItem::lvt_Package ||
                newParentType == UMLListViewItem::lvt_Logical_Folder ||
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
    case UMLListViewItem::lvt_Instance:
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
            UMLCanvasObject *o = newItem->umlObject()->asUMLCanvasObject();
            if (o == 0) {
                DEBUG(DBG_SRC) << "moveObject: newItem's UMLObject is 0";
            } else if (newParentObj == 0) {
                uError() << o->name() << ": newParentObj is 0";
            } else {
                UMLPackage *pkg = newParentObj->asUMLPackage();
                o->setUMLPackage(pkg);
                pkg->addObject(o);
            }
            UMLView *currentView = UMLApp::app()->currentView();
            if (currentView)
                currentView->umlScene()->updateContainment(o);
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

			if (!srcObj) {
			    uError() << "srcObj is NULL";
			    break;
			}

            UMLClassifier *oldParentClassifier = srcObj->umlParent()->asUMLClassifier();
            UMLClassifier *newParentClassifier = newParentObj->asUMLClassifier();
            if (srcType == UMLListViewItem::lvt_Attribute) {
                UMLAttribute *att = srcObj->asUMLAttribute();
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
                    newParent->addChildItem(newAtt, newItem);

                    connectNewObjectsSlots(newAtt);
                    // Let's not forget to update the DocWindow::m_pObject
                    // because the old one is about to be physically deleted !
                    UMLApp::app()->docWindow()->showDocumentation(newAtt, true);
                    delete att;
                }
            } else {
                UMLOperation *op = srcObj->asUMLOperation();
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
                                Uml::ID::None,
                                parm->visibility(),
                                parm->getType(),
                                parm->getInitialValue());
                        newParm->setParmKind(parm->getParmKind());
                        newOp->addParm(newParm);
                    }
                    newItem->setUMLObject(newOp);
                    newParent->addChildItem(newOp, newItem);

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
void UMLListView::slotDropped(QDropEvent* de, UMLListViewItem* target)
{
    DEBUG(DBG_SRC) << "Dropping on target " << target->text(0);

    // Copy or move tree items
    if (de->dropAction() == Qt::CopyAction) {
        UMLClipboard clipboard;

        // Todo: refactor UMLClipboard to support pasting to a non-current item
        setCurrentItem(target);

        // Paste the data (not always clip3)
        if (!clipboard.paste(m_dragCopyData)) {
            uError() << "Unable to copy selected item into the target item";
        }
    } else {
        UMLDragData::LvTypeAndID_List srcList;
        if (! UMLDragData::getClip3TypeAndID(de->mimeData(), srcList)) {
            uError() << "Unexpected mime data in drop event";
            return;
        }
        UMLDragData::LvTypeAndID_It it(srcList);
        UMLDragData::LvTypeAndID * src = 0;
        while (it.hasNext()) {
            src = it.next();
            moveObject(src->id, src->type, target);
        }
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
    UMLListViewItem *item = new UMLListViewItem(parent, view->umlScene()->name(), lvt, view->umlScene()->ID());
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
    case UMLListViewItem::lvt_Port:
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

void UMLListView::contextMenuEvent(QContextMenuEvent *event)
{
    // Get the UMLListViewItem at the point where the mouse pointer was pressed
    UMLListViewItem * item = static_cast<UMLListViewItem*>(itemAt(event->pos()));
    if (item) {
        UMLListViewPopupMenu popup(this, item);
        QAction *triggered = popup.exec(event->globalPos());
        slotMenuSelection(triggered, event->globalPos());
        event->accept();
    }

    QTreeWidget::contextMenuEvent(event);
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
    UMLListViewItem * myItem = dynamic_cast<UMLListViewItem*>(item);
    if (!myItem)
        return;
    if (isExpandable(myItem->type())) {
        myItem->updateFolder();
    }
}

/**
 * Calls updateFolder() on the item to update the icon to closed.
 */
void UMLListView::slotCollapsed(QTreeWidgetItem * item)
{
    UMLListViewItem * myItem = dynamic_cast<UMLListViewItem*>(item);
    if (!myItem)
        return;
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
        UMLListViewItem* item = static_cast<UMLListViewItem*>(currentItem());
        deleteItem(item);

        m_bStartedCut = false;
    }
}

/**
 * Delete every selected item
 */
void UMLListView::slotDeleteSelectedItems()
{
    UMLListViewItemList itemsSelected = selectedItemsRoot();
    foreach(UMLListViewItem *item, itemsSelected) {
        deleteItem(item);
    }
}

/**
 * Adds a new item to the tree of the given type under the given parent.
 * Method will take care of signalling anyone needed on creation of new item.
 * e.g. UMLDoc if a UMLObject is created.
 */
void UMLListView::addNewItem(UMLListViewItem *parentItem, UMLListViewItem::ListViewType type)
{
    if (type == UMLListViewItem::lvt_Datatype) {
        parentItem = m_datatypeFolder;
    }

    parentItem->setOpen(true);

    // Determine the UMLObject belonging to the listview item we're using as parent
    UMLObject* parent = parentItem->umlObject();
    if (parent == 0) {
        uError() << "UMLListView::addNewItem - "
                 << UMLListViewItem::toString(type) << ": parentPkg is 0";
        return;
    }

    if (Model_Utils::typeIsDiagram(type)) {
        Uml::DiagramType::Enum diagramType = Model_Utils::convert_LVT_DT(type);
        QString diagramName = m_doc->createDiagramName(diagramType);
        if (diagramName.isEmpty()) {
            // creation was cancelled by the user
            return;
        }
        UMLFolder* parent = parentItem->umlObject()->asUMLFolder();
        UMLApp::app()->executeCommand(new Uml::CmdCreateDiagram(m_doc, diagramType, diagramName, parent));
        return;
    }

    // Determine the ObjectType of the new object
    UMLObject::ObjectType objectType = Model_Utils::convert_LVT_OT(type);
    if (objectType == UMLObject::ot_UMLObject) {
        uError() << "no UMLObject for type " << UMLListViewItem::toString(type);
        return;
    }

    if (Model_Utils::typeIsClassifierList(type)) {
        UMLClassifier* classifier = parent->asUMLClassifier();
        QString name = classifier->uniqChildName(objectType);
        UMLObject* object = Object_Factory::createChildObject(classifier, objectType, name);

        if (object == 0) {
            // creation was cancelled by the user
            return;
        }

        // Handle primary key constraints (mark the unique constraint as PK on
        // the parent entity)
        if (type == UMLListViewItem::lvt_PrimaryKeyConstraint) {
            UMLUniqueConstraint* uuc = object->asUMLUniqueConstraint();
            UMLEntity* ent = uuc ? uuc->umlParent()->asUMLEntity() : 0;
            if (ent) {
                ent->setAsPrimaryKey(uuc);
            }
        }
    } else {
        bool instanceOfClass = (type == UMLListViewItem::lvt_Instance && parent->isUMLClassifier());
        UMLPackage* package = (instanceOfClass ? parent->umlPackage() : parent->asUMLPackage());
        QString name = Model_Utils::uniqObjectName(objectType, package);
        UMLObject* object = Object_Factory::createUMLObject(objectType, name, package);

        if (object == 0) {
            // creation was cancelled by the user
            return;
        }

        if (type == UMLListViewItem::lvt_Subsystem) {
            object->setStereotypeCmd(QLatin1String("subsystem"));
        } else if (Model_Utils::typeIsFolder(type)) {
            object->setStereotypeCmd(QLatin1String("folder"));
        } else if (instanceOfClass) {
            qApp->processEvents();
            UMLInstance *inst = object->asUMLInstance();
            inst->setClassifierCmd(parent->asUMLClassifier());
            UMLListViewItem *instanceItem = findUMLObject(inst);
            if (instanceItem == 0) {
                uError() << "listviewitem for " << inst->name() << " not found";
                return;
            }
            scrollToItem(instanceItem);
            clearSelection();
            instanceItem->setSelected(true);
            UMLObjectList& values = inst->subordinates();
            foreach (UMLObject *child, values) {
                if (!child->isUMLInstanceAttribute())
                    continue;
                connectNewObjectsSlots(child);
                const QString text = child->asUMLInstanceAttribute()->toString();
                UMLListViewItem *childItem =
                    new UMLListViewItem(instanceItem, text, UMLListViewItem::lvt_InstanceAttribute , child);
                Q_UNUSED(childItem);
            }
        }
    }
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

    case UMLListViewItem::lvt_Object_Diagram:
        return !m_doc->findView(Uml::DiagramType::Object, name);
    break;

    case UMLListViewItem::lvt_Actor:
    case UMLListViewItem::lvt_UseCase:
    case UMLListViewItem::lvt_Node:
    case UMLListViewItem::lvt_Artifact:
    case UMLListViewItem::lvt_Category:
    case UMLListViewItem::lvt_Instance:
        return !m_doc->findUMLObject(name, Model_Utils::convert_LVT_OT(type));
        break;

    case UMLListViewItem::lvt_Class:
    case UMLListViewItem::lvt_Package:
    case UMLListViewItem::lvt_Interface:
    case UMLListViewItem::lvt_Datatype:
    case UMLListViewItem::lvt_Enum:
    case UMLListViewItem::lvt_Entity:
    case UMLListViewItem::lvt_Component:
    case UMLListViewItem::lvt_Port:
    case UMLListViewItem::lvt_Subsystem:
    case UMLListViewItem::lvt_Logical_Folder:
    case UMLListViewItem::lvt_UseCase_Folder:
    case UMLListViewItem::lvt_Component_Folder:
    case UMLListViewItem::lvt_Deployment_Folder:
    case UMLListViewItem::lvt_EntityRelationship_Folder: {
        UMLListViewItem::ListViewType lvt = parentItem->type();
        if (!Model_Utils::typeIsContainer(lvt))
            return (m_doc->findUMLObject(name) == 0);
        UMLPackage *pkg = parentItem->umlObject()->asUMLPackage();
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
    case UMLListViewItem::lvt_InstanceAttribute:
    case UMLListViewItem::lvt_Operation:
    case UMLListViewItem::lvt_EnumLiteral:
    case UMLListViewItem::lvt_UniqueConstraint:
    case UMLListViewItem::lvt_PrimaryKeyConstraint:
    case UMLListViewItem::lvt_ForeignKeyConstraint:
    case UMLListViewItem::lvt_CheckConstraint: {
        UMLClassifier *parent = parentItem->umlObject()->asUMLClassifier();
        if (parent == 0) {
            uError() << "internal error - "
                     << "parent listviewitem is package but has no UMLObject";
            return true;
        }
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
void UMLListView::saveToXMI1(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("listview"));
    m_rv->saveToXMI1(writer);
    writer.writeEndElement();
}

/**
 *
 */
bool UMLListView::loadFromXMI1(QDomElement & element)
{
    QDomNode node = element.firstChild();
    QDomElement domElement = node.toElement();
    m_doc->writeToStatusBar(i18n("Loading listview..."));
    while (!domElement.isNull()) {
        if (domElement.tagName() == QLatin1String("listitem")) {
            QString type = domElement.attribute(QLatin1String("type"), QLatin1String("-1"));
            if (type == QLatin1String("-1"))
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
        if (domElement.tagName() != QLatin1String("listitem")) {
            domElement = node.toElement();
            continue;
        }
        QString id = domElement.attribute(QLatin1String("id"), QLatin1String("-1"));
        QString type = domElement.attribute(QLatin1String("type"), QLatin1String("-1"));
        QString label = domElement.attribute(QLatin1String("label"));
        QString open = domElement.attribute(QLatin1String("open"), QLatin1String("1"));
        if (type == QLatin1String("-1"))
            return false;
        UMLListViewItem::ListViewType lvType = (UMLListViewItem::ListViewType)type.toInt();
        bool bOpen = (bool)open.toInt();
        Uml::ID::Type nID = Uml::ID::fromString(id);
        UMLListViewItem * item = 0;
        if (nID != Uml::ID::None) {
            // The following is an ad hoc hack for the copy/paste code.
            // The clip still contains the old children although new
            // UMLCLassifierListItems have already been created.
            // If the IDChangeLog finds new IDs this means we are in
            // copy/paste and need to adjust the child listitems to the
            // new UMLCLassifierListItems.
            IDChangeLog *idchanges = m_doc->changeLog();
            if (idchanges) {
                Uml::ID::Type newID = idchanges->findNewID(nID);
                if (newID != Uml::ID::None) {
                    DEBUG(DBG_SRC) << " using id " << Uml::ID::toString(newID)
                                   << " instead of " << Uml::ID::toString(nID);
                    nID = newID;
                }
            }
            /************ End of hack for copy/paste code ************/

            UMLObject *pObject = m_doc->findObjectById(nID);
            if (pObject) {
                if (label.isEmpty())
                    label = pObject->name();
            } else if (Model_Utils::typeIsFolder(lvType)) {
                // Synthesize the UMLFolder here
                UMLObject *umlParent = parent->umlObject();
                UMLPackage *parentPkg = umlParent->asUMLPackage();
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
            const Uml::ModelType::Enum mt = Model_Utils::convert_LVT_MT(lvType);
            nID = m_doc->rootFolder(mt)->id();
        } else if (Model_Utils::typeIsFolder(lvType)) {
            // Pre-1.2 format: Folders did not have their ID set.
            // Pull a new ID now.
            nID = m_doc->rootFolder(Uml::ModelType::Logical)->id();
        } else {
            uError() << "item of type " << type << " has no ID, skipping.";
            domElement = node.toElement();
            continue;
        }

        switch (lvType) {
        case UMLListViewItem::lvt_Actor:
        case UMLListViewItem::lvt_UseCase:
        case UMLListViewItem::lvt_Class:
        case UMLListViewItem::lvt_Instance:
        case UMLListViewItem::lvt_Interface:
        case UMLListViewItem::lvt_Datatype:
        case UMLListViewItem::lvt_Enum:
        case UMLListViewItem::lvt_Entity:
        case UMLListViewItem::lvt_Package:
        case UMLListViewItem::lvt_Subsystem:
        case UMLListViewItem::lvt_Component:
        case UMLListViewItem::lvt_Port:
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
                         << "findItem(id " << Uml::ID::toString(nID) << ") returns 0";
            } else if (parent != item->parent()) {
                // The existing item was created by the slot event triggered
                // by the loading of the corresponding model object from the
                // XMI file.
                UMLListViewItem *itmParent = dynamic_cast<UMLListViewItem*>(item->parent());
                DEBUG(DBG_SRC) << "Loaded <listview> entry does not match uml model"
                               << item->text(0) << " parent "
                               << parent->text(0) << " (" << parent << ") != "
                               << (itmParent ? itmParent->text(0) : QLatin1String(""))
                               << " (" << itmParent << ")";
            }
            break;
        case UMLListViewItem::lvt_Attribute:
        case UMLListViewItem::lvt_EntityAttribute:
        case UMLListViewItem::lvt_InstanceAttribute:
        case UMLListViewItem::lvt_Template:
        case UMLListViewItem::lvt_Operation:
        case UMLListViewItem::lvt_EnumLiteral:
        case UMLListViewItem::lvt_UniqueConstraint:
        case UMLListViewItem::lvt_PrimaryKeyConstraint:
        case UMLListViewItem::lvt_ForeignKeyConstraint:
        case UMLListViewItem::lvt_CheckConstraint:
            item = findItem(nID);
            if (item == 0) {
                DEBUG(DBG_SRC) << "item " << Uml::ID::toString(nID) << " (of type "
                               << UMLListViewItem::toString(lvType) << ") does not yet exist...";
                UMLObject* umlObject = parent->umlObject();
                if (!umlObject) {
                    DEBUG(DBG_SRC) << "And also the parent->umlObject() does not exist";
                    return false;
                }
                if (nID == Uml::ID::None) {
                    uWarning() << "lvtype " << UMLListViewItem::toString(lvType) << " has id -1";
                } else if (lvType == UMLListViewItem::lvt_InstanceAttribute) {
                    UMLInstance *instance = umlObject->asUMLInstance();
                    if (instance) {
                        umlObject = instance->findChildObjectById(nID);
                        if (umlObject) {
                            UMLInstanceAttribute *instAttr = umlObject->asUMLInstanceAttribute();
                            connectNewObjectsSlots(instAttr);
                            label = instAttr->toString();
                            item = new UMLListViewItem(parent, label, lvType, instAttr);
                        } else {
                            DEBUG(DBG_SRC) << umlObject->name() << " lvt_InstanceAttribute child "
                                           << " object " << Uml::ID::toString(nID) << " not found";
                        }
                    } else {
                        DEBUG(DBG_SRC) << "cast to instance object failed";
                    }
                } else {
                    UMLClassifier *classifier = umlObject->asUMLClassifier();
                    if (classifier) {
                        umlObject = classifier->findChildObjectById(nID);
                        if (umlObject) {
                            connectNewObjectsSlots(umlObject);
                            label = umlObject->name();
                            item = new UMLListViewItem(parent, label, lvType, umlObject);
                        } else {
                            DEBUG(DBG_SRC) << "lvtype " << UMLListViewItem::toString(lvType)
                                           << " child object " << Uml::ID::toString(nID) << " not found";
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
                    << UMLListViewItem::toString(lvType) << " (ID " << Uml::ID::toString(nID) << ")";
            }
            break;
        }//end switch

        if (item)  {
            item->setOpen((bool)bOpen);
            if (!loadChildrenFromXMI(item, domElement)) {
                return false;
            }
        } else {
            uWarning() << "unused list view item " << Uml::ID::toString(nID)
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
    if (!item) item = m_rv;
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
    if (!item) item = m_rv;
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
        m_doc->removeDiagram(temp->ID());
    } else if (temp == m_datatypeFolder) {
        // we can't delete the datatypeFolder because umbrello will crash without a special handling
        return false;
    } else if (Model_Utils::typeIsCanvasWidget(lvt) || Model_Utils::typeIsClassifierList(lvt)) {
        UMLPackage *nmSpc = object->asUMLPackage();
        if (nmSpc) {
            UMLObjectList contained = nmSpc->containedObjects();
            if (contained.count()) {
                if (nmSpc->baseType() == UMLObject::ot_Class) {
                    KMessageBox::error(
                        nullptr,
                        i18n("The class must be emptied before it can be deleted."),
                        i18n("Class Not Empty"));
                } else if (nmSpc->baseType() == UMLObject::ot_Package) {
                    KMessageBox::error(
                        nullptr,
                        i18n("The package must be emptied before it can be deleted."),
                        i18n("Package Not Empty"));
                } else if (nmSpc->baseType() == UMLObject::ot_Folder) {
                    KMessageBox::error(
                        nullptr,
                        i18n("The folder must be emptied before it can be deleted."),
                        i18n("Folder Not Empty"));
                }
                return false;
            }
        }
        UMLCanvasObject *canvasObj = object->asUMLCanvasObject();
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
            UMLApp::app()->executeCommand(new Uml::CmdRemoveUMLObject(object));

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
 * Always allow starting a drag
 */
void UMLListView::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
}

/**
 * Check drag destination and update move/copy action
 */
void UMLListView::dragMoveEvent(QDragMoveEvent* event)
{
    // Check if drag destination is compatible with source
    if (acceptDrag(event)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
        return;
    }
}

/**
 *
 */
void UMLListView::dropEvent(QDropEvent* event)
{
    if (!acceptDrag(event)) {
        event->ignore();
    }
    else {
        UMLListViewItem* target = static_cast<UMLListViewItem*>(itemAt(event->pos()));
        if (!target) {
            DEBUG(DBG_SRC) << "itemAt(mousePoint) returns 0";
            event->ignore();
            return;
        }
        slotDropped(event, target);
    }
}

void UMLListView::commitData(QWidget *editor)
{
    if (!editor)
        return;

    QModelIndex index = currentIndex();
    if (!index.isValid())
        return;

    QAbstractItemDelegate *delegate = itemDelegate(index);
    editor->removeEventFilter(delegate);
    QByteArray n = editor->metaObject()->userProperty().name();
    if (n.isEmpty()) {
        DEBUG(DBG_SRC) << "no name property found in list view item editor";
        return;
    }

    QString newText = editor->property(n).toString();

    UMLListViewItem *item = dynamic_cast<UMLListViewItem *>(currentItem());
    if (!item) {
        DEBUG(DBG_SRC) << "no item found after editing model index" << index;
        return;
    }
    item->slotEditFinished(newText);
    editor->installEventFilter(delegate);
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

