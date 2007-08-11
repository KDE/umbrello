/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2007                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "umllistview.h"

// qt/kde includes
#include <qregexp.h>
#include <qpoint.h>
#include <qrect.h>
#include <qevent.h>
#include <qheader.h>
#include <qtooltip.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kinputdialog.h>

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
#include "docwindow.h"
#include "listpopupmenu.h"
#include "template.h"
#include "operation.h"
#include "attribute.h"
#include "entityattribute.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistviewitemlist.h"
#include "umllistviewitem.h"
#include "umlview.h"
#include "umlviewimageexporter.h"
#include "usecase.h"
#include "model_utils.h"
#include "uniqueid.h"
#include "clipboard/idchangelog.h"
#include "clipboard/umldrag.h"
#include "dialogs/classpropdlg.h"
#include "dialogs/umlattributedialog.h"
#include "dialogs/umlentityattributedialog.h"
#include "dialogs/umloperationdialog.h"
#include "dialogs/umltemplatedialog.h"

#ifdef WANT_LVTOOLTIP
class LVToolTip : public QToolTip
{
public:
    LVToolTip (QWidget* parent) : QToolTip (parent) {}
    virtual ~LVToolTip () {}
protected:
    /**
     * Reimplemented from QToolTip for internal reasons.
     * At classifiers, only the method names are shown in the list view -
     * we use a tooltip for the full signature display.
     * Once KListView's tooltip overriding mechanism works, we can kick
     * this class out.
     */
    virtual void maybeTip (const QPoint& pos) {
        UMLListView *lv = UMLApp::app()->getListView();
        UMLListViewItem * item = (UMLListViewItem*)lv->itemAt(pos);
        if (item == NULL)
            return;
        UMLObject *obj = item->getUMLObject();
        if (obj == NULL || obj->getBaseType() != Uml::ot_Operation)
            return;
        UMLOperation *op = static_cast<UMLOperation*>(obj);
        QString text = op->toString(Uml::st_ShowSig);
        QRect rect = lv->itemRect(item);
        tip(rect, text);
    }
};
#endif


UMLListView::UMLListView(QWidget *parent, const char *name)
        : KListView(parent,name), m_pMenu(0), m_doc(UMLApp::app()->getDocument())
{
    loadPixmaps();

    //setup list view
    setBackgroundColor(Qt::white);
    setAcceptDrops(true);
    setDropVisualizer(false);
    setItemsMovable(true);
    setItemsRenameable( true );
    setSelectionModeExt(FileManager);
    setFocusPolicy(QWidget::StrongFocus);
    setDragEnabled(true);
    setColumnWidthMode( 0, Manual );
    setDefaultRenameAction( Accept );
    setResizeMode( LastColumn );
    header()->setClickEnabled(true);
    //add columns and initial items
    addColumn(m_doc->getName());

#ifdef WANT_LVTOOLTIP
    /* In KDE-3.3, we cannot use KListView's builtin mechanism for
       overriding the tooltips. Instead, see the above class LVToolTip.
    setShowToolTips( true );
    setTooltipColumn( 0 );
     */
    (void) new LVToolTip(viewport());
#endif
    m_pMenu = NULL;
    m_bStartedCut = m_bStartedCopy = false;
    m_bIgnoreCancelRename = true;
    m_bCreatingChildObject = false;
    m_rv = NULL;
    for (int i = 0; i < Uml::N_MODELTYPES; i++)
        m_lv[i] = NULL;
    m_datatypeFolder = NULL;
    //setup slots/signals
    connect(this, SIGNAL(dropped(QDropEvent *, QListViewItem *, QListViewItem *)),
            this, SLOT(slotDropped(QDropEvent *, QListViewItem *, QListViewItem *)));
    connect( this, SIGNAL( collapsed( QListViewItem * ) ),
             this, SLOT( slotCollapsed( QListViewItem * ) ) );
    connect( this, SIGNAL( expanded( QListViewItem * ) ), this, SLOT( slotExpanded( QListViewItem * ) ) );
    connect( UMLApp::app(), SIGNAL( sigCutSuccessful() ), this, SLOT( slotCutSuccessful() ) );
}

UMLListView::~UMLListView() {}

bool UMLListView::eventFilter(QObject *o, QEvent *e) {
    if (e->type() != QEvent::MouseButtonPress || !o->isA("QHeader"))
        return QListView::eventFilter(o, e);
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::RightButton) {
        if (m_pMenu) {
            m_pMenu->hide();
            disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(popupMenuSel(int)));
            delete m_pMenu;
        }
        m_pMenu = new ListPopupMenu(this, Uml::lvt_Model);
        m_pMenu->popup(me->globalPos());
        connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(popupMenuSel(int)));
        return true;
    }
    return QListView::eventFilter(o, e);
}

void UMLListView::contentsMousePressEvent(QMouseEvent *me) {
    UMLView *currentView = UMLApp::app()->getCurrentView();
    if (currentView)
        currentView->clearSelected();
    if( me -> state() != Qt::ShiftButton )
        clearSelection();
    QPoint pt = this->QScrollView::contentsToViewport( me->pos() );
    UMLListViewItem * item = (UMLListViewItem*)itemAt(pt);
    const Qt::ButtonState button = me->button();

    if (!item || (button != Qt::RightButton && button != Qt::LeftButton)) {
        UMLApp::app()->getDocWindow()->updateDocumentation(true);
        return;
    }

    if (button == Qt::LeftButton) {
        UMLObject *o = item->getUMLObject();
        if (o)
            UMLApp::app()->getDocWindow()->showDocumentation(o, false);
        else
            UMLApp::app()->getDocWindow()->updateDocumentation(true);
    }
    if (button == Qt::RightButton) {
        if(m_pMenu != 0) {
            m_pMenu->hide();
            disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(popupMenuSel(int)));
            delete m_pMenu;
            m_pMenu = 0;
        }
        const Uml::ListView_Type type = item->getType();
        m_pMenu = new ListPopupMenu(this, type);
        m_pMenu->popup(me->globalPos());
        connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(popupMenuSel(int)));
    }//end if right button

    this->KListView::contentsMousePressEvent(me);
}

void UMLListView::contentsMouseReleaseEvent(QMouseEvent *me) {
    if (me->button() != Qt::LeftButton) {
        this->KListView::contentsMouseReleaseEvent(me);
        return;
    }
    const QPoint pt = this->QScrollView::contentsToViewport( me->pos() );
    UMLListViewItem *item = dynamic_cast<UMLListViewItem*>(itemAt(pt));
    if (item == NULL || !Model_Utils::typeIsDiagram(item->getType())) {
        this->KListView::contentsMouseReleaseEvent(me);
        return;
    }
    // Switch to diagram on mouse release - not on mouse press
    // because the user might intend a drag-to-note.
    m_doc->changeCurrentView( item->getID() );
    UMLApp::app()->getDocWindow()->showDocumentation(m_doc->findView(item->getID()), false);
    this->KListView::contentsMouseReleaseEvent(me);
}

void UMLListView::keyPressEvent(QKeyEvent *ke) {
    UMLView *view = UMLApp::app()->getCurrentView();
    if (view && view->getSelectCount()) {
        // Widgets have been selected in the diagram area,
        // assume they handle the keypress.
        ke->accept();                 // munge and do nothing
    } else {
        const int k = ke->key();
        if (k == Qt::Key_Delete || k == Qt::Key_Backspace) {
            // delete every selected item
            UMLListViewItemList selecteditems;
            getSelectedItemsRoot(selecteditems);
            UMLListViewItemListIt it(selecteditems);
            for (UMLListViewItem *item = 0; (item = it.current()); ++it) {
                deleteItem(dynamic_cast<UMLListViewItem*>(item));
            }
        } else {
            QListView::keyPressEvent(ke); // let parent handle it
        }
    }
}

void UMLListView::popupMenuSel(int sel) {
    UMLListViewItem * temp = (UMLListViewItem*)currentItem();
    if ( !temp ) {
        kDebug() << "popupMenuSel invoked without currently selectedItem" << endl;
        return;
    }
    UMLObject * object = temp -> getUMLObject();
    Uml::ListView_Type lvt = temp -> getType();
    Uml::Object_Type umlType = Uml::ot_UMLObject;
    ListPopupMenu::Menu_Type menuType = (ListPopupMenu::Menu_Type)sel;
    QString name;

    switch (menuType) {
    case ListPopupMenu::mt_Class:
        addNewItem( temp, Uml::lvt_Class );
        break;

    case ListPopupMenu::mt_Package:
        addNewItem(temp, Uml::lvt_Package);
        break;

    case ListPopupMenu::mt_Subsystem:
        addNewItem(temp, Uml::lvt_Subsystem);
        break;

    case ListPopupMenu::mt_Component:
        addNewItem(temp, Uml::lvt_Component);
        break;

    case ListPopupMenu::mt_Node:
        addNewItem(temp, Uml::lvt_Node);
        break;

    case ListPopupMenu::mt_Artifact:
        addNewItem(temp, Uml::lvt_Artifact);
        break;

    case ListPopupMenu::mt_Interface:
        addNewItem(temp, Uml::lvt_Interface);
        break;

    case ListPopupMenu::mt_Enum:
        addNewItem(temp, Uml::lvt_Enum);
        break;

    case ListPopupMenu::mt_EnumLiteral:
        addNewItem(temp, Uml::lvt_EnumLiteral);
        break;

    case ListPopupMenu::mt_Template:
        addNewItem(temp, Uml::lvt_Template);
        break;

    case ListPopupMenu::mt_Entity:
        addNewItem(temp, Uml::lvt_Entity);
        break;

    case ListPopupMenu::mt_Datatype:
        addNewItem(temp, Uml::lvt_Datatype);
        break;

    case ListPopupMenu::mt_Actor:
        addNewItem( temp, Uml::lvt_Actor );
        break;

    case ListPopupMenu::mt_UseCase:
        addNewItem( temp, Uml::lvt_UseCase );
        break;

    case ListPopupMenu::mt_Attribute:
        addNewItem( temp, Uml::lvt_Attribute );
        break;

    case ListPopupMenu::mt_EntityAttribute:
        addNewItem( temp, Uml::lvt_EntityAttribute );
        break;

    case ListPopupMenu::mt_Operation:
        addNewItem( temp, Uml::lvt_Operation );
        break;

    case ListPopupMenu::mt_Import_Classes:
        UMLApp::app()->slotImportClasses();
        break;

    case ListPopupMenu::mt_Expand_All:
        expandAll(temp);
        break;

    case ListPopupMenu::mt_Collapse_All:
        collapseAll(temp);
        break;

    case ListPopupMenu::mt_Export_Image:
        m_doc->findView(temp->getID())->getImageExporter()->exportView();
        break;

    case ListPopupMenu::mt_Externalize_Folder:
        {
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            UMLFolder *modelFolder = dynamic_cast<UMLFolder*>(current->getUMLObject());
            if (modelFolder == NULL) {
                kError() << "UMLListView::popupMenuSel: modelFolder is NULL" << endl;
                return;
            }
            // configure & show the file dialog
            const QString rootDir(m_doc->URL().directory());
            KFileDialog fileDialog(rootDir, "*.xml", this, ":externalize-folder", true);
            fileDialog.setCaption(i18n("Externalize Folder"));
            fileDialog.setOperationMode(KFileDialog::Other);
            // set a sensible default filename
            QString defaultFilename = current->getText().lower();
            defaultFilename.replace(QRegExp("\\W+"), "_");
            defaultFilename.append(".xml");  // default extension
            fileDialog.setSelection(defaultFilename);
            fileDialog.exec();
            KURL selURL = fileDialog.selectedURL();
            if (selURL.isEmpty())
                return;
            QString path = selURL.path();
            QString fileName = path;
            if (fileName.startsWith(rootDir)) {
                fileName.remove(rootDir);
            } else {
                // This should be done using a KMessageBox but we currently
                // cannot add new i18n strings.
                kError() << "Folder " << path
                    << " must be relative to the main model directory, "
                    << rootDir << endl;
                return;
            }
            QFile file(path);
            // Warn if file exists.
            if (file.exists()) {
                // This should be done using a KMessageBox but we currently
                // cannot add new i18n strings.
                kWarning() << "file " << fileName << " already exists!" << endl;
                kWarning() << "The existing file will be overwritten." << endl;
            }
            // Test if file is writable.
            if (file.open(IO_WriteOnly)) {
                file.close();
            } else {
                KMessageBox::error(0,
                                   i18n("There was a problem saving file: %1").arg(fileName),
                                   i18n("Save Error"));
                return;
            }
            modelFolder->setFolderFile(fileName);
            // Recompute text of the folder
            QString folderText = current->getText();
            folderText.remove( QRegExp("\\s*\\(.*$") );
            folderText.append( " (" + fileName + ')' );
            current->setText(folderText);
            break;
        }

    case ListPopupMenu::mt_Internalize_Folder:
        {
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            UMLFolder *modelFolder = dynamic_cast<UMLFolder*>(current->getUMLObject());
            if (modelFolder == NULL) {
                kError() << "UMLListView::popupMenuSel: modelFolder is NULL" << endl;
                return;
            }
            modelFolder->setFolderFile(QString::null);
            // Recompute text of the folder
            QString folderText = current->getText();
            folderText.remove( QRegExp("\\s*\\(.*$") );
            current->setText(folderText);
            break;
        }

    case ListPopupMenu::mt_Model:
        {
            bool ok = false;
            QString name = KInputDialog::getText( i18n("Enter Model Name"),
                                                  i18n("Enter the new name of the model:"),
                                                  m_doc->getName(), &ok, UMLApp::app() );
            if (ok) {
                setColumnText(0, name);
                m_doc->setName(name);
            }
            break;
        }

    case ListPopupMenu::mt_Rename:
        temp-> startRename(0);
        break;

    case ListPopupMenu::mt_Delete:
        deleteItem(temp);

        return;
        break;

    case ListPopupMenu::mt_Properties:
        /* first check if we are on a diagram */
        if( Model_Utils::typeIsDiagram(lvt) ) {
            UMLView * pView = m_doc->findView( temp->getID() );
            if( !pView ) {
                return;
            }
            UMLApp::app()->getDocWindow()->updateDocumentation(false);
            pView->showPropDialog();
            UMLApp::app()->getDocWindow()->showDocumentation(pView, true);
            temp->cancelRename(0);
            return;
        }

        /* ok, we are on another object, so find out on which one */
        umlType = object->getBaseType();

        if ( Model_Utils::typeIsCanvasWidget(lvt) ) {
            object->showProperties(ClassPropDlg::page_gen);
        } else if(umlType == Uml::ot_Attribute) {
            // show the attribute dialog
            UMLAttribute* selectedAttribute = static_cast<UMLAttribute*>(object);
            UMLAttributeDialog dialog( this, selectedAttribute );
            dialog.exec();
        } else if(umlType == Uml::ot_EntityAttribute) {
            // show the attribute dialog
            UMLEntityAttribute* selectedAttribute = static_cast<UMLEntityAttribute*>(object);
            UMLEntityAttributeDialog dialog( this, selectedAttribute );
            dialog.exec();
        } else if(umlType == Uml::ot_Operation) {
            // show the operation dialog
            UMLOperation* selectedOperation = static_cast<UMLOperation*>(object);
            UMLOperationDialog dialog( this, selectedOperation );
            dialog.exec();
        } else if(umlType == Uml::ot_Template) {
            // show the template dialog
            UMLTemplate* selectedTemplate = static_cast<UMLTemplate*>(object);
            UMLTemplateDialog dialog( this, selectedTemplate );
            dialog.exec();
        } else {
            kWarning() << "calling properties on unknown type" << endl;
        }
        temp -> cancelRename( 0 );
        break;

    case ListPopupMenu::mt_Logical_Folder:
        addNewItem( temp, Uml::lvt_Logical_Folder );
        break;

    case ListPopupMenu::mt_UseCase_Folder:
        addNewItem( temp, Uml::lvt_UseCase_Folder );
        break;

    case ListPopupMenu::mt_Component_Folder:
        addNewItem(temp, Uml::lvt_Component_Folder);
        break;

    case ListPopupMenu::mt_Deployment_Folder:
        addNewItem(temp, Uml::lvt_Deployment_Folder);
        break;

    case ListPopupMenu::mt_EntityRelationship_Folder:
        addNewItem(temp, Uml::lvt_EntityRelationship_Folder);
        break;

    case ListPopupMenu::mt_Cut:
        m_bStartedCut = true;
        m_bStartedCopy = false;
        UMLApp::app() -> slotEditCut();
        break;

    case ListPopupMenu::mt_Copy:
        m_bStartedCut = false;
        m_bStartedCopy = true;
        UMLApp::app() -> slotEditCopy();
        break;

    case ListPopupMenu::mt_Paste:
        UMLApp::app() -> slotEditPaste();
        break;

    default:
        {
            Uml::Diagram_Type dt = ListPopupMenu::convert_MT_DT(menuType);
            if (dt == Uml::dt_Undefined) {
                kWarning() << "UMLListView::popupMenuSel: unknown type"
                    << sel << endl;
            } else {
                UMLFolder *f = dynamic_cast<UMLFolder*>(object);
                if (f == NULL)
                    kError() << "UMLListView::popupMenuSel(" << menuType
                        << "): current item's UMLObject is not a UMLFolder" << endl;
                else
                    m_doc->createDiagram(f, dt);
            }
        }
        break;
    }//end switch
}

UMLListViewItem *UMLListView::findFolderForDiagram(Uml::Diagram_Type dt) {
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

void UMLListView::slotDiagramCreated( Uml::IDType id ) {
    if( m_doc->loading() )
        return;
    UMLView *v = m_doc -> findView( id );
    if (!v)
        return;
    const Uml::Diagram_Type dt = v->getType();
    UMLListViewItem * temp = 0, *p = findFolderForDiagram(dt);
    temp = new UMLListViewItem(p, v->getName(), Model_Utils::convert_DT_LVT(dt), id);
    setSelected( temp, true );
    UMLApp::app() -> getDocWindow() -> showDocumentation( v , false );
}

UMLListViewItem* UMLListView::determineParentItem(UMLObject* object) const {
    UMLListViewItem* parentItem = NULL;
    UMLListViewItem* current = (UMLListViewItem*) currentItem();
    Uml::ListView_Type lvt = Uml::lvt_Unknown;
    if (current)
        lvt = current->getType();
    Uml::Object_Type t = object->getBaseType();

    switch (t) {
    case Uml::ot_Attribute:
    case Uml::ot_Operation:
    case Uml::ot_Template:
    case Uml::ot_EnumLiteral:
    case Uml::ot_EntityAttribute:
        //this will be handled by childObjectAdded
        return NULL;
        break;
    case Uml::ot_Association:
    case Uml::ot_Role:
    case Uml::ot_Stereotype:
        return NULL;  // currently no representation in list view
        break;
    default:
        {
            UMLPackage *pkg = object->getUMLPackage();
            if (pkg) {
                UMLListViewItem* pkgItem = findUMLObject(pkg);
                if (pkgItem == NULL)
                    kError() << "UMLListView::determineParentItem: could not find "
                        << "parent package " << pkg->getName() << endl;
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

bool UMLListView::mayHaveChildItems(Uml::Object_Type type) {
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

void UMLListView::slotObjectCreated(UMLObject* object) {
    if (m_bCreatingChildObject) {
        // @todo eliminate futile signal traffic
        // e.g. we get here thru various indirections from
        // ClassifierListPage::slot{Up,Down}Clicked()
        return;
    }
    UMLListViewItem* newItem = findUMLObject(object);
    if (newItem) {
        kDebug() << "UMLListView::slotObjectCreated(" << object->getName()
            << ", id= " << ID2STR(object->getID())
            << "): item already exists." << endl;
        Uml::Icon_Type icon = Model_Utils::convert_LVT_IT(newItem->getType());
        newItem->setIcon(icon);
        return;
    }
    UMLListViewItem* parentItem = determineParentItem(object);
    if (parentItem == NULL)
        return;
    Uml::Object_Type type = object->getBaseType();

    connectNewObjectsSlots(object);
    const Uml::ListView_Type lvt = Model_Utils::convert_OT_LVT(object);
    QString name = object->getName();
    if (type == Uml::ot_Folder) {
        UMLFolder *f = static_cast<UMLFolder*>(object);
        QString folderFile = f->getFolderFile();
        if (!folderFile.isEmpty())
            name.append(" (" + folderFile + ')');
    }
    newItem = new UMLListViewItem(parentItem, name, lvt, object);
    if (mayHaveChildItems(type)) {
        UMLClassifier *c = static_cast<UMLClassifier*>(object);
        UMLClassifierListItemList cListItems = c->getFilteredList(Uml::ot_UMLObject);
        UMLClassifierListItem *cli;
        for (UMLClassifierListItemListIt it(cListItems); (cli = it.current()) != NULL; ++it)
            childObjectAdded(cli, c);
    }
    if (m_doc->loading())
        return;
    ensureItemVisible(newItem);
    newItem->setOpen(true);
    clearSelection();
    setSelected(newItem, true);
    UMLApp::app()->getDocWindow()->showDocumentation(object, false);
}

void UMLListView::connectNewObjectsSlots(UMLObject* object) {
    Uml::Object_Type type = object->getBaseType();
    switch( type )
    {
    case Uml::ot_Class:
    case Uml::ot_Interface:
        {
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
            connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
        }
        break;
    case Uml::ot_Enum:
        {
            UMLEnum *e = static_cast<UMLEnum*>(object);
            connect(e, SIGNAL(enumLiteralAdded(UMLClassifierListItem*)),
                    this, SLOT(childObjectAdded(UMLClassifierListItem*)));
            connect(e, SIGNAL(enumLiteralRemoved(UMLClassifierListItem*)),
                    this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
        }
        connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
        break;
    case Uml::ot_Entity:
        {
            UMLEntity *ent = static_cast<UMLEntity*>(object);
            connect(ent, SIGNAL(entityAttributeAdded(UMLClassifierListItem*)),
                    this, SLOT(childObjectAdded(UMLClassifierListItem*)));
            connect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                    this, SLOT(childObjectRemoved(UMLClassifierListItem*)));
        }
        connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
        break;
    case Uml::ot_Datatype:
    case Uml::ot_Attribute:
    case Uml::ot_Operation:
    case Uml::ot_Template:
    case Uml::ot_EnumLiteral:
    case Uml::ot_EntityAttribute:
    case Uml::ot_Package:
    case Uml::ot_Actor:
    case Uml::ot_UseCase:
    case Uml::ot_Component:
    case Uml::ot_Artifact:
    case Uml::ot_Node:
    case Uml::ot_Folder:
        connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
        break;
    case Uml::ot_UMLObject:
    case Uml::ot_Association:
    case Uml::ot_Stereotype:
        break;
    default:
        kWarning() << "unknown type in connectNewObjectsSlots" << endl;
        break;
    }
}

void UMLListView::slotObjectChanged() {
    if (m_doc->loading()) { //needed for class wizard
        return;
    }
    UMLObject* obj = const_cast<UMLObject*>( dynamic_cast<const UMLObject*>(sender()) );
    UMLListViewItem* item = findUMLObject(obj);
    if(item) {
        item->updateObject();
    }
}

void UMLListView::childObjectAdded(UMLClassifierListItem* obj) {
    UMLClassifier *parent = const_cast<UMLClassifier*>(dynamic_cast<const UMLClassifier*>(sender()));
    childObjectAdded(obj, parent);
}

void UMLListView::childObjectAdded(UMLClassifierListItem* child, UMLClassifier* parent) {
    if (m_bCreatingChildObject)
        return;
    const QString text = child->toString(Uml::st_SigNoVis);
    UMLListViewItem *childItem = NULL;
    UMLListViewItem *parentItem = findUMLObject(parent);
    if (parentItem == NULL) {
        kDebug() << "UMLListView::childObjectAdded(" << child->getName()
            << "): parent " << parent->getName()
            << " does not yet exist, creating it now." << endl;
        const Uml::ListView_Type lvt = Model_Utils::convert_OT_LVT(parent);
        parentItem = new UMLListViewItem(m_lv[Uml::mt_Logical], parent->getName(), lvt, parent);
    } else {
        childItem = parentItem->findChildObject(child);
    }
    if (childItem) {
        childItem->setText(text);
    } else {
        const Uml::ListView_Type lvt = Model_Utils::convert_OT_LVT(child);
        childItem = new UMLListViewItem(parentItem, text, lvt, child);
        if (! m_doc->loading()) {
            ensureItemVisible(childItem);
            clearSelection();
            setSelected(childItem, true);
        }
        connectNewObjectsSlots(child);
    }
}

void UMLListView::childObjectRemoved(UMLClassifierListItem* obj) {
    UMLClassifier *parent = const_cast<UMLClassifier*>(dynamic_cast<const UMLClassifier*>(sender()));
    UMLListViewItem *parentItem = findUMLObject(parent);
    if (parentItem == NULL) {
        kError() << "UMLListView::childObjectRemoved(" << obj->getName()
            << "): cannot find parent UMLListViewItem" << endl;
        return;
    }
    parentItem->deleteChildItem(obj);
}

void UMLListView::slotDiagramRenamed(Uml::IDType id) {
    UMLListViewItem* temp;
    UMLView* v = m_doc->findView(id);
    if ((temp = findView(v)) == NULL) {
        kError() << "UMLListView::slotDiagramRenamed: UMLDoc::findView("
            << ID2STR(id) << ") returns NULL" << endl;
        return;
    }
    temp->setText( v->getName() );
}

void UMLListView::setDocument(UMLDoc *d) {
    if( m_doc && m_doc != d)
    {
        //disconnect signals from old doc and reset view
    }
    m_doc = d;

    connect(m_doc, SIGNAL(sigDiagramCreated(Uml::IDType)), this, SLOT(slotDiagramCreated(Uml::IDType)));
    connect(m_doc, SIGNAL(sigDiagramRemoved(Uml::IDType)), this, SLOT(slotDiagramRemoved(Uml::IDType)));
    connect(m_doc, SIGNAL(sigDiagramRenamed(Uml::IDType)), this, SLOT(slotDiagramRenamed(Uml::IDType)));
    connect(m_doc, SIGNAL(sigObjectCreated(UMLObject *)), this, SLOT(slotObjectCreated(UMLObject *)));
    connect(m_doc, SIGNAL(sigObjectRemoved(UMLObject *)), this, SLOT(slotObjectRemoved(UMLObject *)));
}

void UMLListView::slotObjectRemoved(UMLObject* object) {
    if (m_doc->loading()) { //needed for class wizard
        return;
    }
    disconnect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
    UMLListViewItem* item = findItem(object->getID());
    delete item;
    UMLApp::app()->getDocWindow()->updateDocumentation(true);
}

void UMLListView::slotDiagramRemoved(Uml::IDType id) {
    UMLListViewItem* item = findItem(id);
    delete item;
    UMLApp::app()->getDocWindow()->updateDocumentation(true);
}

QDragObject* UMLListView::dragObject() {
    UMLListViewItemList selecteditems;
    getSelectedItems(selecteditems);
    selecteditems.setAutoDelete( false );
    UMLListViewItemListIt it(selecteditems);
    UMLListViewItem * item = 0;
    UMLListViewItemList  list;
    list.setAutoDelete( false );
    while((item=it.current()) != 0) {
        ++it;
        Uml::ListView_Type type = item->getType();
        if (!Model_Utils::typeIsCanvasWidget(type) && !Model_Utils::typeIsDiagram(type)
                && !Model_Utils::typeIsClassifierList(type)) {
            return 0;
        }
        list.append(item);
    }
    UMLDrag *t = new UMLDrag(list, this);

    return t;
}

void UMLListView::startDrag() {
    QDragObject *o = dragObject();
    if (o)
        o->dragCopy();
}

UMLListViewItem * UMLListView::findUMLObjectInFolder(UMLListViewItem* folder, UMLObject* obj) {
    UMLListViewItem *item = static_cast<UMLListViewItem *>(folder->firstChild());
    while(item)
    {
        switch(item->getType())
        {
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
            if(item->getUMLObject() == obj)
                return item;
            break;
        case Uml::lvt_Logical_Folder :
        case Uml::lvt_UseCase_Folder :
        case Uml::lvt_Component_Folder :
        case Uml::lvt_Deployment_Folder :
        case Uml::lvt_EntityRelationship_Folder :
        case Uml::lvt_Datatype_Folder :
            {
                UMLListViewItem *temp = findUMLObjectInFolder(item, obj);
                if (temp)
                    return temp;
            }
        default:
            break;
        }
        item = static_cast<UMLListViewItem *>(item->nextSibling());
    }
    return 0;
}

UMLListViewItem * UMLListView::findUMLObject(const UMLObject *p) const {
    UMLListViewItem *item = static_cast<UMLListViewItem*>(firstChild());
    while (item) {
        UMLListViewItem *testItem = item->findUMLObject(p);
        if (testItem)
            return testItem;
        item = static_cast<UMLListViewItem*>(item->nextSibling());
    }
    return item;
}

void UMLListView::changeIconOf(UMLObject *o, Uml::Icon_Type to) {
    UMLListViewItem *item = findUMLObject(o);
    if (item == NULL)
        return;
    item->setIcon(to);
}

UMLListViewItem* UMLListView::findView(UMLView* v) {
    if (!v) {
        kWarning() << "returning 0 from UMLListView::findView()" << endl;
        return 0;
    }
    UMLListViewItem* item;
    Uml::Diagram_Type dType = v->getType();
    Uml::ListView_Type type = Model_Utils::convert_DT_LVT( dType );
    Uml::IDType id = v->getID();
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

    UMLListViewItem* searchStartItem = (UMLListViewItem *)item->firstChild();

    UMLListViewItem* foundItem = recursiveSearchForView(searchStartItem, type, id);

    if (!foundItem) {
        kWarning() << "returning 0 at UMLListView::findView" << endl;
    }
    return foundItem;
}

UMLListViewItem* UMLListView::recursiveSearchForView(UMLListViewItem* listViewItem,
        Uml::ListView_Type type, Uml::IDType id) {
    while (listViewItem) {
        if ( Model_Utils::typeIsFolder(listViewItem->getType()) ) {
            UMLListViewItem* child = (UMLListViewItem *)listViewItem->firstChild();
            UMLListViewItem* resultListViewItem = recursiveSearchForView(child, type, id);
            if (resultListViewItem) {
                return resultListViewItem;
            }
        } else {
            if(listViewItem->getType() == type && listViewItem->getID() == id) {
                return listViewItem;
            }
        }
        listViewItem = (UMLListViewItem*)listViewItem->nextSibling();
    }
    return 0;
}

UMLListViewItem* UMLListView::findItem(Uml::IDType id) {
    UMLListViewItem *temp;
    QListViewItemIterator it(this);
    for( ; (temp = (UMLListViewItem*)it.current()); ++it ) {
        UMLListViewItem * item = temp->findItem(id);
        if (item)
            return item;
    }
    return 0;
}


//
// This method is called more than once during an instance's lifetime (by UMLDoc)!
// So we must not allocate any memory before freeing the previously allocated one
// or do connect()s.
//
void UMLListView::init() {
    if (m_rv == NULL) {
        m_rv =  new UMLListViewItem(this, i18n("Views"), Uml::lvt_View);
        for (int i = 0; i < Uml::N_MODELTYPES; i++) {
            Uml::Model_Type mt = (Uml::Model_Type)i;
            UMLFolder *sysFolder = m_doc->getRootFolder(mt);
            Uml::ListView_Type lvt = Model_Utils::convert_MT_LVT(mt);
            m_lv[i] = new UMLListViewItem(m_rv, sysFolder->getLocalName(), lvt, sysFolder);
        }
    } else {
        for (int i = 0; i < Uml::N_MODELTYPES; i++)
            deleteChildrenOf(m_lv[i]);
    }
    UMLFolder *datatypeFolder = m_doc->getDatatypeFolder();
    m_datatypeFolder = new UMLListViewItem(m_lv[Uml::mt_Logical], datatypeFolder->getLocalName(),
                                           Uml::lvt_Datatype_Folder, datatypeFolder);
    m_rv->setOpen(true);
    for (int i = 0; i < Uml::N_MODELTYPES; i++)
        m_lv[i]->setOpen(true);
    m_datatypeFolder->setOpen(false);

    //setup misc.
    delete m_pMenu;
    m_pMenu = 0;
    m_bStartedCut = m_bStartedCopy = false;
    m_bIgnoreCancelRename = true;
    m_bCreatingChildObject = false;
}

void UMLListView::setView(UMLView * v) {
    if(!v)
        return;
    UMLListViewItem * temp = findView(v);
    if(temp)
        setSelected(temp, true);
}

void UMLListView::contentsMouseDoubleClickEvent(QMouseEvent * me) {
    UMLListViewItem * item = static_cast<UMLListViewItem *>( currentItem() );
    if( !item || me -> button() != Qt::LeftButton )
        return;
    //see if on view
    Uml::ListView_Type lvType = item -> getType();
    if( Model_Utils::typeIsDiagram(lvType) ) {
        UMLView * pView = m_doc -> findView( item -> getID() );
        if( !pView )
            return;
        UMLApp::app() -> getDocWindow() -> updateDocumentation( false );
        pView -> showPropDialog();
        UMLApp::app() -> getDocWindow() -> showDocumentation( pView, true );
        item -> cancelRename( 0 );
        return;
    }
    //else see if an object
    UMLObject * object = item -> getUMLObject();
    //continue only if we are on a UMLObject
    if(!object)
        return;


    Uml::Object_Type type = object -> getBaseType();
    int page = ClassPropDlg::page_gen;
    if(type == Uml::ot_Attribute || type == Uml::ot_Operation)
        object = (UMLObject *)object -> parent();
    //set what page to show
    if(type == Uml::ot_Attribute)
        page = ClassPropDlg::page_att;
    else if(type == Uml::ot_Operation)
        page = ClassPropDlg::page_op;
    //FIXME for entityattributes

    if(object)
        object->showProperties(page);
    item -> cancelRename( 0 );//double click can cause it to go into rename mode.
}


bool UMLListView::acceptDrag(QDropEvent* event) const {
    QPoint mousePoint = ((UMLListView*)this)->contentsToViewport( event->pos() );

    UMLListViewItem* item = (UMLListViewItem*)itemAt(mousePoint);
    if(!item) {
        kDebug() << "UMLListView::acceptDrag: itemAt(mousePoint) returns NULL"
            << endl;
        return false;
    }
    ((QListView*)this)->setCurrentItem( (QListViewItem*)item );

    UMLDrag::LvTypeAndID_List list;
    if (! UMLDrag::getClip3TypeAndID(event, list)) {
        kDebug() << "UMLListView::acceptDrag: UMLDrag::getClip3TypeAndID returns false"
            << endl;
        return false;
    }

    UMLDrag::LvTypeAndID_It it(list);
    UMLDrag::LvTypeAndID * data = 0;
    Uml::ListView_Type dstType = item->getType();
    bool accept = true;
    while(accept && ((data = it.current()) != 0)) {
        ++it;
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
            accept = (dstType == Uml::lvt_EntityRelationship_Folder);
            break;
        default:
            accept = false;
            break;
        }
    }

    //kDebug() << "UMLListView::acceptDrag: dstType = " << dstType
    //    << ", accept=" << accept << endl;
    return accept;
}

void UMLListView::addAtContainer(UMLListViewItem *item, UMLListViewItem *parent) {
    UMLCanvasObject *o = static_cast<UMLCanvasObject*>(item->getUMLObject());
    if (o == NULL) {
        kDebug() << "UMLListView::addAtContainer(" << item->getText()
            << "): item's UMLObject is NULL" << endl;
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
        kError() << "UMLListView::addAtContainer(" << item->getText()
            << "): parent type is " << parent->getType() << endl;
    }
    UMLView *currentView = UMLApp::app()->getCurrentView();
    if (currentView)
        currentView->updateContainment(o);
}

UMLListViewItem * UMLListView::moveObject(Uml::IDType srcId, Uml::ListView_Type srcType,
                                          UMLListViewItem *newParent) {
    if (newParent == NULL)
        return NULL;
    UMLListViewItem * move = findItem( srcId );
    if (move == NULL)
        return NULL;

    UMLObject *newParentObj = NULL;
    // Remove the source object at the old parent package.
    UMLObject *srcObj = m_doc->findObjectById(srcId);
    if (srcObj) {
        newParentObj = newParent->getUMLObject();
        if (srcObj == newParentObj) {
            kError() << "UMLListView::moveObject(" << srcObj->getName()
                << "): Cannot move onto self" << endl;
            return NULL;
        }
        UMLPackage *srcPkg = srcObj->getUMLPackage();
        if (srcPkg) {
            if (srcPkg == newParentObj) {
                kError() << "UMLListView::moveObject(" << srcObj->getName()
                    << "): Object is already in target package" << endl;
                return NULL;
            }
            srcPkg->removeObject(srcObj);
        }
    }

    Uml::ListView_Type newParentType = newParent->getType();
    kDebug() << "UMLListView::moveObject: newParentType is " << newParentType << endl;
    UMLListViewItem *newItem = NULL;

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
            if (o == NULL) {
                kDebug() << "moveObject: newItem's UMLObject is NULL" << endl;
            } else if (newParentObj == NULL) {
                kError() << "UMLListView::moveObject(" << o->getName()
                    << "): newParentObj is NULL" << endl;
            } else {
                UMLPackage *pkg = static_cast<UMLPackage*>(newParentObj);
                o->setUMLPackage( pkg );
                pkg->addObject( o );
            }
            UMLView *currentView = UMLApp::app()->getCurrentView();
            if (currentView)
                currentView->updateContainment(o);
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
                if (att == NULL) {
                    kError() << "moveObject internal error: srcObj "
                        << srcObj->getName() << " is not a UMLAttribute" << endl;
                } else if (oldParentClassifier->takeItem(att) == -1) {
                    kError() << "moveObject: oldParentClassifier->takeItem(att "
                        << att->getName() << ") returns NULL" << endl;
                } else {
                    const QString& nm = att->getName();
                    UMLAttribute *newAtt = newParentClassifier->createAttribute(nm,
                                                                                att->getType(),
                                                                                att->getVisibility(),
                                                                                att->getInitialValue());
                    newItem->setUMLObject(newAtt);
                    newParent->addClassifierListItem( newAtt, newItem );

                    connectNewObjectsSlots( newAtt );
                    // Let's not forget to update the DocWindow::m_pObject
                    // because the old one is about to be physically deleted !
                    UMLApp::app()->getDocWindow()->showDocumentation(newAtt, true);
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
                                              op->getName(), &isExistingOp, &ntDummyList);
                    newOp->setType(op->getType());
                    newOp->setVisibility(op->getVisibility());
                    UMLAttributeList parmList = op->getParmList();
                    for (UMLAttributeListIt plit(parmList); plit.current(); ++plit) {
                        UMLAttribute *parm = plit.current();
                        UMLAttribute *newParm = new UMLAttribute(newParentClassifier,
                                                                 parm->getName(),
                                                                 Uml::id_None,
                                                                 parm->getVisibility(),
                                                                 parm->getType(),
                                                                 parm->getInitialValue());
                        newParm->setParmKind(parm->getParmKind());
                        newOp->addParm(newParm);
                    }
                    newItem->setUMLObject(newOp);
                    newParent->addClassifierListItem( newOp, newItem );

                    connectNewObjectsSlots( newOp );

                    // Let's not forget to update the DocWindow::m_pObject
                    // because the old one is about to be physically deleted !
                    UMLApp::app()->getDocWindow()->showDocumentation(newOp, true);
                    delete op;
                } else {
                    kError() << "moveObject: oldParentClassifier->takeItem(op) returns NULL"
                        << endl;
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

void UMLListView::slotDropped(QDropEvent* de, QListViewItem* /* parent */, QListViewItem* item) {
    item = (UMLListViewItem *)currentItem();
    if(!item) {
        kDebug() << "UMLListView::slotDropped: item is NULL - doing nothing" << endl;
        return;
    }
    UMLDrag::LvTypeAndID_List srcList;
    if (! UMLDrag::getClip3TypeAndID(de, srcList)) {
        return;
    }
    UMLListViewItem *newParent = (UMLListViewItem*)item;
    kDebug() << "slotDropped: newParent->getText() is " << newParent->getText() << endl;
    UMLDrag::LvTypeAndID_It it(srcList);
    UMLDrag::LvTypeAndID * src = 0;
    while((src = it.current()) != 0) {
        ++it;
        moveObject(src->id, src->type, newParent);
    }
}

int UMLListView::getSelectedItems(UMLListViewItemList &ItemList) {
    ItemList.setAutoDelete( false );
    QListViewItemIterator it(this);
    // iterate through all items of the list view
    for ( ; it.current(); ++it ) {
        if ( it.current()->isSelected() ) {
            UMLListViewItem *item = (UMLListViewItem*)it.current();
            ItemList.append(item);
        }
    }
    kDebug() << "UMLListView::getSelectedItems: selItems = " << ItemList.count() << endl;

    return (int)ItemList.count();
}

int UMLListView::getSelectedItemsRoot(UMLListViewItemList &ItemList) {
    ItemList.setAutoDelete( false );
    QListViewItemIterator it(this);

    // iterate through all items of the list view
    for ( ; it.current(); ++it ) {
        if ( it.current()->isSelected() ) {
            UMLListViewItem *item = (UMLListViewItem*)it.current();
            // this is the trick, we select only the item with a parent unselected
            // since we can't select a child and its grandfather without its parent
            // we would be able to delete each item individually, without an invalid iterator
            if (item && item->parent() && item->parent()->isSelected()==false) {
              ItemList.append(item);
            }
        }
    }
    kDebug() << "UMLListView::getSelectedItemsRoot: selItems = " << ItemList.count() << endl;

    return (int)ItemList.count();
}

UMLListViewItem* UMLListView::createDiagramItem(UMLView *v) {
    Uml::ListView_Type lvt = Model_Utils::convert_DT_LVT(v->getType());
    UMLListViewItem *parent = NULL;
    UMLFolder *f = v->getFolder();
    if (f) {
        parent = findUMLObject(f);
        if (parent == NULL)
            kError() << "UMLListView::createDiagramItem(" << v->getName()
                << "): findUMLObject(" << f->getName() << ") returns NULL"
                << endl;
    } else {
        kDebug() << "UMLListView::createDiagramItem(" << v->getName()
            << "): no parent folder set, using predefined folder" << endl;
    }
    if (parent == NULL) {
        parent = determineParentItem(lvt);
        lvt = Model_Utils::convert_DT_LVT(v->getType());
    }
    UMLListViewItem *item = new UMLListViewItem(parent, v->getName(), lvt, v->getID());
    return item;
}

/** Creates a new UMLListViewItem from a UMLListViewItem,
    if parent is null the ListView Decides who is going to be
    the parent */
UMLListViewItem* UMLListView::createItem(UMLListViewItem& Data, IDChangeLog& IDChanges,
        UMLListViewItem* parent /*= 0*/) {
    UMLObject* pObject = 0;
    UMLListViewItem* item = 0;
    Uml::ListView_Type lvt = Data.getType();
    if(!parent) {
        parent = determineParentItem(lvt);
        if (!parent)
            return 0;
    }

    switch(lvt) {
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
        pObject = m_doc->findObjectById( Data.getID() );
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
        {
            UMLClassifier *pClass =  static_cast<UMLClassifier*>(parent->getUMLObject());
            Uml::IDType newID = IDChanges.findNewID( Data.getID() );
            pObject = pClass->findChildObjectById(newID);
            if (pObject) {
                item = new UMLListViewItem( parent, Data.getText(), lvt, pObject );
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
    case Uml::lvt_EntityRelationship_Diagram:
        {
            Uml::IDType newID = IDChanges.findNewID(Data.getID());
            UMLView* v = m_doc->findView(newID);
            if (v == NULL) {
                return NULL;
            }
            const Uml::ListView_Type lvt = Model_Utils::convert_DT_LVT(v->getType());
            item = new UMLListViewItem(parent, v->getName(), lvt, newID);
        }
        break;
    default:
        kWarning() << "createItem() called on unknown type" << endl;
        break;
    }
    return item;
}

UMLListViewItem* UMLListView::determineParentItem(Uml::ListView_Type lvt) const {
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
        parent = m_lv[Uml::mt_EntityRelationship];
        break;
    default:
        if (Model_Utils::typeIsDiagram(lvt) || !Model_Utils::typeIsClassifierList(lvt))
            parent = m_lv[Uml::mt_Logical];
        break;
    }
    return parent;
}

int UMLListView::getSelectedCount() {
    QListViewItemIterator it(this);
    int count = 0;
    // iterate through all items of the list view
    for ( ; it.current(); ++it ) {
        if ( it.current()->isSelected() ) {
            count++;
        }
    }

    return count;
}

void UMLListView::focusOutEvent ( QFocusEvent * fe) {
    QFocusEvent::Reason reason = fe->reason();
    if (reason != QFocusEvent::Popup) {
        clearSelection();
        triggerUpdate();
    }
    //repaint();

    QListView::focusOutEvent(fe);
}

Uml::ListView_Type UMLListView::rootViewType(UMLListViewItem *item) {
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

QPixmap & UMLListView::getPixmap(Uml::Icon_Type type) {
    if (type < Uml::it_Home || type >= Uml::N_ICONTYPES) {
        kWarning() << "getPixmap() called on unknown icon " << type << endl;
        // you'll know you have a problem if this shows up in the list
        type = Uml::it_Home;
    }
    return m_Pixmaps[type];
}

void UMLListView::loadPixmaps() {
    KStandardDirs * dirs = KGlobal::dirs();
    QString dataDir = dirs -> findResourceDir("data", "umbrello/pics/object.png");
    dataDir += "/umbrello/pics/";

#define makeBarIcon(iconType, barIcon)   m_Pixmaps[iconType] = BarIcon(barIcon)
    makeBarIcon(Uml::it_Home,               "folder_home");
    makeBarIcon(Uml::it_Folder_Cyan,        "folder");
    makeBarIcon(Uml::it_Folder_Cyan_Open,   "folder_open");
    makeBarIcon(Uml::it_Folder_Green,       "folder_green");
    makeBarIcon(Uml::it_Folder_Green_Open,  "folder_green_open");
    makeBarIcon(Uml::it_Folder_Orange,      "folder_orange");
    makeBarIcon(Uml::it_Folder_Orange_Open, "folder_orange_open");
    makeBarIcon(Uml::it_Folder_Grey,        "folder_grey");
    makeBarIcon(Uml::it_Folder_Grey_Open,   "folder_grey_open");
    makeBarIcon(Uml::it_Folder_Red,         "folder_red");
    makeBarIcon(Uml::it_Folder_Red_Open,    "folder_red_open");
    makeBarIcon(Uml::it_Folder_Violet,      "folder_violet");
    makeBarIcon(Uml::it_Folder_Violet_Open, "folder_violet_open");

    makeBarIcon(Uml::it_Diagram_Activity,           "umbrello_diagram_activity");
    makeBarIcon(Uml::it_Diagram_Class,              "umbrello_diagram_class");
    makeBarIcon(Uml::it_Diagram_Component,          "umbrello_diagram_component");
    makeBarIcon(Uml::it_Diagram_State,              "umbrello_diagram_state");
    makeBarIcon(Uml::it_Diagram_Sequence,           "umbrello_diagram_sequence");
    makeBarIcon(Uml::it_Diagram_Deployment,         "umbrello_diagram_deployment");
    makeBarIcon(Uml::it_Diagram_EntityRelationship, "umbrello_diagram_deployment");
    makeBarIcon(Uml::it_Diagram_Usecase,            "umbrello_diagram_usecase");
    makeBarIcon(Uml::it_Diagram_Collaboration,      "umbrello_diagram_collaboration");
#undef makeBarIcon

#define loadPixmap(iconType, pngName)   m_Pixmaps[iconType].load(dataDir + pngName)
    loadPixmap(Uml::it_Diagram,    "CVnamespace.png");
    loadPixmap(Uml::it_Class,      "class.png");
    loadPixmap(Uml::it_Template,   "template.png");
    loadPixmap(Uml::it_Package,    "package.png");
    loadPixmap(Uml::it_Subsystem,  "subsystem.png");
    loadPixmap(Uml::it_Component,  "component.png");
    loadPixmap(Uml::it_Node,       "node.png");
    loadPixmap(Uml::it_Artifact,   "artifact.png");
    loadPixmap(Uml::it_Interface,  "interface.png");
    loadPixmap(Uml::it_Datatype,   "datatype.png");
    loadPixmap(Uml::it_Enum,       "enum.png");
    loadPixmap(Uml::it_Entity,     "entity.png");
    loadPixmap(Uml::it_Actor,      "actor.png");
    loadPixmap(Uml::it_UseCase,    "usecase.png");
    loadPixmap(Uml::it_Public_Method,  "CVpublic_meth.png");
    loadPixmap(Uml::it_Private_Method,  "CVprivate_meth.png");
    loadPixmap(Uml::it_Protected_Method, "CVprotected_meth.png");
    loadPixmap(Uml::it_Public_Attribute,  "CVpublic_var.png");
    loadPixmap(Uml::it_Private_Attribute,  "CVprivate_var.png");
    loadPixmap(Uml::it_Protected_Attribute, "CVprotected_var.png");
#undef loadPixmap
}

bool UMLListView::isExpandable(Uml::ListView_Type lvt) {
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

void UMLListView::slotExpanded( QListViewItem * item ) {
    UMLListViewItem * myItem= static_cast<UMLListViewItem*>(item);
    if (isExpandable(myItem->getType()))
        myItem->updateFolder();
}

void UMLListView::slotCollapsed( QListViewItem * item ) {
    UMLListViewItem * myItem = static_cast<UMLListViewItem*>(item);
    if (isExpandable(myItem->getType()))
        myItem->updateFolder();
}

void UMLListView::slotCutSuccessful() {
    if( m_bStartedCut ) {
        popupMenuSel( ListPopupMenu::mt_Delete );
        //deletion code here
        m_bStartedCut = false;
    }
}

void UMLListView::addNewItem(UMLListViewItem *parentItem, Uml::ListView_Type type) {
     if (type == Uml::lvt_Datatype) {
         parentItem = m_datatypeFolder;
     }

    UMLListViewItem * newItem = NULL;
    parentItem->setOpen( true );

    Uml::Icon_Type icon = Model_Utils::convert_LVT_IT(type);

    QString name;
    if (Model_Utils::typeIsDiagram(type)) {
        Uml::Diagram_Type dt = Model_Utils::convert_LVT_DT(type);
        name = getUniqueDiagramName(dt);
        newItem = new UMLListViewItem(parentItem, name, type, Uml::id_None);
    } else {
        Uml::Object_Type ot = Model_Utils::convert_LVT_OT(type);
        if (ot == Uml::ot_UMLObject) {
            kDebug() << "UMLListView::addNewItem: no UMLObject for listview type "
                << type << endl;
            return;
        }
        UMLPackage *parentPkg =
            dynamic_cast<UMLPackage*>(parentItem->getUMLObject());
        if (parentPkg == NULL) {
            kError() << "UMLListView::addNewItem(type " << type
                << "): parentPkg is NULL" << endl;
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
    newItem->setIcon( icon );
    newItem->setOpen( true );
    newItem->setCreating( true );
    newItem->startRename( 0 );  // calls QListView::ensureItemVisible()
    // When the user accepts the rename operation, UMLListViewItem::okRename()
    // is called (automatically by QListViewItem.)
}

bool UMLListView::itemRenamed( QListViewItem * item , int /*col*/ ) {
    //if true the item was cancel before this message
    if( m_bIgnoreCancelRename ) {
        return true;
    }
    m_bIgnoreCancelRename = true;
    UMLListViewItem * renamedItem = static_cast< UMLListViewItem *>( item ) ;
    Uml::ListView_Type type = renamedItem -> getType();
    QString newText = renamedItem -> text( 0 );
    renamedItem -> setCreating( false );

    // If the type is empty then delete it.
    if (newText.isEmpty() || newText.contains(QRegExp("^\\s+$"))) {
        KMessageBox::error(
            kapp -> mainWidget(),
            i18n( "The name you entered was invalid.\nCreation process has been canceled." ),
            i18n( "Name Not Valid" ) );
        return false;
    }

    if( !isUnique( renamedItem, newText ) ) {
        //if operation ask if ok not to be unique i.e overloading
        if( type == Uml::lvt_Operation ) {
            if( KMessageBox::warningYesNo(
                        kapp -> mainWidget(),
                        i18n( "The name you entered was not unique.\nIs this what you wanted?" ),
                        i18n( "Name Not Unique" ), i18n("Use Name"), i18n("Enter New Name") ) == KMessageBox::No ) {
                return false;
            }
        } else {
            KMessageBox::error(
                kapp -> mainWidget(),
                i18n( "The name you entered was not unique!\nCreation process has been canceled." ),
                i18n( "Name Not Unique" ) );
            return false;
        }
    }

    switch( type ) {
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
        {
            Uml::Object_Type ot = Model_Utils::convert_LVT_OT(type);
            if (! ot) {
                kError() << "UMLListView::itemRenamed() internal" << endl;
                return false;
            }
            UMLObject *o = createUMLObject( renamedItem, ot );
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
        return createChildUMLObject( renamedItem, Model_Utils::convert_LVT_OT(type) );
        break;

    case Uml::lvt_Class_Diagram:
        createDiagram( renamedItem, Uml::dt_Class );
        break;

    case Uml::lvt_UseCase_Diagram:
        createDiagram( renamedItem, Uml::dt_UseCase );
        break;

    case Uml::lvt_Sequence_Diagram:
        createDiagram( renamedItem, Uml::dt_Sequence );
        break;

    case Uml::lvt_Collaboration_Diagram:
        createDiagram( renamedItem, Uml::dt_Collaboration );
        break;

    case Uml::lvt_State_Diagram:
        createDiagram( renamedItem, Uml::dt_State );
        break;

    case Uml::lvt_Activity_Diagram:
        createDiagram( renamedItem, Uml::dt_Activity );
        break;

    case Uml::lvt_Component_Diagram:
        createDiagram( renamedItem, Uml::dt_Component );
        break;

    case Uml::lvt_Deployment_Diagram:
        createDiagram( renamedItem, Uml::dt_Deployment );
        break;

    case Uml::lvt_EntityRelationship_Diagram:
        createDiagram( renamedItem, Uml::dt_EntityRelationship );
        break;

    default:
        break;
    }
    return true;
}

UMLObject *UMLListView::createUMLObject( UMLListViewItem * item, Uml::Object_Type type ) {
    QString name = item -> text( 0 );
    UMLObject * object = NULL;
    switch( type ) {
    case Uml::ot_UseCase:
        object = new UMLUseCase( name );
        break;

    case Uml::ot_Actor:
        object = new UMLActor( name );
        break;

    case Uml::ot_Class:
        object = new UMLClassifier( name );
        break;

    case Uml::ot_Package:
        object = new UMLPackage( name );
        break;

    case Uml::ot_Folder:
        object = new UMLFolder( name );
        break;

    case Uml::ot_Component:
        object = new UMLComponent( name );
        break;

    case Uml::ot_Node:
        object = new UMLNode( name );
        break;

    case Uml::ot_Artifact:
        object = new UMLArtifact( name );
        break;

    case Uml::ot_Interface:
        {
            UMLClassifier *c = new UMLClassifier(name);
            c->setBaseType(Uml::ot_Interface);
            object = c;
        }
        break;

    case Uml::ot_Datatype:
        {
            UMLClassifier *c = new UMLClassifier(name);
            c->setBaseType(Uml::ot_Datatype);
            object = c;
        }
        break;

    case Uml::ot_Enum:
        object = new UMLEnum( name );
        break;

    case Uml::ot_Entity:
        object = new UMLEntity( name );
        break;

    default:
        kWarning() << "creating UML Object of unknown type" << endl;
        return NULL;
    }

    UMLListViewItem * parentItem = static_cast<UMLListViewItem *>(item->parent());
    const Uml::ListView_Type lvt = parentItem->getType();
    if (! Model_Utils::typeIsContainer(lvt)) {
        kError() << "UMLListView::createUMLObject(" << object->getName()
            << "): parentItem (" << lvt << " is not a container" << endl;
        delete object;
        return NULL;
    }
    UMLPackage *pkg = static_cast<UMLPackage*>(parentItem->getUMLObject());
    object->setUMLPackage(pkg);
    pkg->addObject(object);
    connectNewObjectsSlots(object);
    item -> setUMLObject( object );
    item -> setText( name );
    return object;
}

bool UMLListView::createChildUMLObject( UMLListViewItem * item, Uml::Object_Type type ) {
    m_bCreatingChildObject = true;
    QString text = item->text( 0 );
    UMLObject* parent = static_cast<UMLListViewItem *>( item->parent() )->getUMLObject();
    if( !parent ) {
        kError() << "UMLListView::createChildUMLObject: parent UMLObject is NULL" << endl;
        m_bCreatingChildObject = false;
        return false;
    }

    //kDebug() << "UMLListView::createChildUMLObject (" << text << ")" << endl;
    UMLObject* newObject = NULL;
    if ( type == Uml::ot_EnumLiteral ) {
        UMLEnum *owningEnum = static_cast<UMLEnum*>(parent);
        newObject = owningEnum->createEnumLiteral(text);

        UMLEnumLiteral* enumLiteral = static_cast<UMLEnumLiteral*>(newObject);
        text = enumLiteral->toString(Uml::st_SigNoVis);
    } else if ( type == Uml::ot_Template )  {
        UMLClassifier *owningClassifier = static_cast<UMLClassifier*>(parent);
        Model_Utils::NameAndType nt;
        Model_Utils::Parse_Status st = Model_Utils::parseTemplate(text, nt, owningClassifier);
        if (st) {
            KMessageBox::error( kapp->mainWidget(),
                                Model_Utils::psText(st),
                                i18n("Creation canceled") );
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
            KMessageBox::error( kapp->mainWidget(),
                                Model_Utils::psText(st),
                                i18n("Creation canceled") );
            m_bCreatingChildObject = false;
            return false;
        }
        newObject = owningClass->createAttribute(nt.m_name, nt.m_type, vis, nt.m_initialValue);
        UMLAttribute *att = static_cast<UMLAttribute*>(newObject);
        att->setParmKind(nt.m_direction);
        text = att->toString(Uml::st_SigNoVis);
    } else if ( type == Uml::ot_Operation ) {
        UMLClassifier *owningClassifier = static_cast<UMLClassifier*>(parent);
        Model_Utils::OpDescriptor od;
        Model_Utils::Parse_Status st = Model_Utils::parseOperation(text, od, owningClassifier);
        if (st) {
            KMessageBox::error( kapp->mainWidget(),
                                Model_Utils::psText(st),
                                i18n("Creation canceled") );
            m_bCreatingChildObject = false;
            return false;
        }
        bool isExistingOp = false;
        newObject = owningClassifier->createOperation(od.m_name, &isExistingOp, &od.m_args);
        if (newObject == NULL || isExistingOp) {
            if (isExistingOp)
                KMessageBox::error(
                    kapp -> mainWidget(),
                    i18n( "The name you entered was not unique!\nCreation process has been canceled." ),
                    i18n( "Name Not Unique" ) );
            m_bCreatingChildObject = false;
            return false;
        }
        UMLOperation *op = static_cast<UMLOperation*>(newObject);
        if (od.m_pReturnType) {
            op->setType(od.m_pReturnType);
        }
        text = op->toString(Uml::st_SigNoVis);
    } else {
        kError() << "UMLListView::createChildUMLObject called for type "
            << type << " (ignored)" << endl;
        m_bCreatingChildObject = false;
        return false;
    }

    // make changes to the object visible to this umllistviewitem
    connectNewObjectsSlots( newObject );
    item->setUMLObject( newObject );
    item->setText( text );
    ensureItemVisible(item);

    // as it's a ClassifierListItem add it to the childObjectMap of the parent
    UMLClassifierListItem* classifierListItem = static_cast<UMLClassifierListItem*>( newObject );
    static_cast<UMLListViewItem*>( item->parent() )->addClassifierListItem(classifierListItem, item );

    m_bCreatingChildObject = false;

    if (! m_doc->loading())
        m_doc->setModified();
    return true;
}

void UMLListView::createDiagram( UMLListViewItem * item, Uml::Diagram_Type type ) {
    QString name = item -> text( 0 );
    UMLView * view = m_doc -> findView( type, name );
    if( view ) {
        delete item;
        return;
    }
    UMLListViewItem *parentItem = static_cast<UMLListViewItem*>(item->parent());
    UMLFolder *parentFolder = dynamic_cast<UMLFolder*>(parentItem->getUMLObject());
    if (parentFolder == NULL) {
        kError() << "UMLListView::createDiagram(" << name
            << "): parent UMLObject is not a UMLFolder" << endl;
        delete item;
        return;
    }
    view = new UMLView(parentFolder);
    view->setName( name );
    view->setType( type );
    view->setID( UniqueID::gen() );
    m_doc -> addView( view );
    view -> setOptionState( Settings::getOptionState() );
    item -> setID( view -> getID() );
    item -> setText( name );
    view->activate();
    m_doc -> changeCurrentView( view -> getID() );
}

QString UMLListView::getUniqueDiagramName(Uml::Diagram_Type type) {
    return m_doc->uniqViewName(type);
}

bool UMLListView::isUnique( UMLListViewItem * item, const QString &name ) {
    UMLListViewItem * parentItem = static_cast<UMLListViewItem *>( item -> parent() );
    Uml::ListView_Type type = item -> getType();
    switch( type ) {
    case Uml::lvt_Class_Diagram:
        return !m_doc -> findView( Uml::dt_Class, name );
        break;

    case Uml::lvt_Sequence_Diagram:
        return !m_doc -> findView( Uml::dt_Sequence, name );
        break;

    case Uml::lvt_UseCase_Diagram:
        return !m_doc -> findView( Uml::dt_UseCase, name );
        break;

    case Uml::lvt_Collaboration_Diagram:
        return !m_doc -> findView( Uml::dt_Collaboration, name );
        break;

    case Uml::lvt_State_Diagram:
        return !m_doc -> findView( Uml::dt_State, name );
        break;

    case Uml::lvt_Activity_Diagram:
        return !m_doc -> findView( Uml::dt_Activity, name );
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
        return !m_doc->findUMLObject( name, Model_Utils::convert_LVT_OT(type) );
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
    case Uml::lvt_EntityRelationship_Folder:
        {
            Uml::ListView_Type lvt = parentItem->getType();
            if (!Model_Utils::typeIsContainer(lvt))
                return (m_doc->findUMLObject(name) == NULL);
            UMLPackage *pkg = static_cast<UMLPackage*>(parentItem->getUMLObject());
            if (pkg == NULL) {
                kError() << "UMLListView::isUnique: internal error - "
                    << "parent listviewitem is package but has no UMLObject" << endl;
                return true;
            }
            return (pkg->findObject(name) == NULL);
            break;
        }

    case Uml::lvt_Template:
    case Uml::lvt_Attribute:
    case Uml::lvt_EntityAttribute:
    case Uml::lvt_Operation:
    case Uml::lvt_EnumLiteral:
        {
            UMLClassifier *parent = static_cast<UMLClassifier*>(parentItem->getUMLObject());
            return (parent->findChildObject(name) == NULL);
            break;
        }

    default:
        break;
    }
    return false;
}

void UMLListView::cancelRename( QListViewItem * item ) {
    if( !m_bIgnoreCancelRename ) {
        delete item;
        m_bIgnoreCancelRename = true;
    }
}

void UMLListView::saveToXMI( QDomDocument & qDoc, QDomElement & qElement) {
    QDomElement listElement = qDoc.createElement( "listview" );
    m_rv->saveToXMI(qDoc, listElement);
    qElement.appendChild( listElement );
}

bool UMLListView::loadFromXMI( QDomElement & element ) {
    /*
        deleteChildrenOf( m_ucv );
        deleteChildrenOf( m_lv );
        deleteChildrenOf( m_cmpv );
        deleteChildrenOf( m_dplv );
     */
    QDomNode node = element.firstChild();
    QDomElement domElement = node.toElement();
    m_doc->writeToStatusBar( i18n("Loading listview...") );
    while( !domElement.isNull() ) {
        if( domElement.tagName() == "listitem" ) {
            QString type = domElement.attribute( "type", "-1" );
            if( type == "-1" )
                return false;
            Uml::ListView_Type lvType = (Uml::ListView_Type)type.toInt();
            if( lvType == Uml::lvt_View ) {
                if( !loadChildrenFromXMI( m_rv, domElement ) )
                    return false;
            } else
                return false;
        }
        node = node.nextSibling();
        domElement = node.toElement();

    }//end while
    return true;
}

bool UMLListView::loadChildrenFromXMI( UMLListViewItem * parent, QDomElement & element ) {
    QDomNode node = element.firstChild();
    QDomElement domElement = node.toElement();
    const QString pfx("UMLListView::loadChildrenFromXMI: ");
    while( !domElement.isNull() ) {
        node = domElement.nextSibling();
        if( domElement.tagName() != "listitem" ) {
            domElement = node.toElement();
            continue;
        }
        QString id = domElement.attribute( "id", "-1" );
        QString type = domElement.attribute( "type", "-1" );
        QString label = domElement.attribute( "label", "" );
        QString open = domElement.attribute( "open", "1" );
        if( type == "-1" )
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
            IDChangeLog *idchanges = m_doc->getChangeLog();
            if (idchanges != NULL) {
                Uml::IDType newID = idchanges->findNewID(nID);
                if (newID != Uml::id_None) {
                    kDebug() << pfx << " using id " << ID2STR(newID)
                        << " instead of " << ID2STR(nID) << endl;
                    nID = newID;
                }
            }
            /************ End of hack for copy/paste code ************/

            pObject = m_doc->findObjectById(nID);
            if (pObject) {
                if (label.isEmpty())
                    label = pObject->getName();
            } else if (Model_Utils::typeIsFolder(lvType)) {
                // Synthesize the UMLFolder here
                UMLObject *umlParent = parent->getUMLObject();
                UMLPackage *parentPkg = dynamic_cast<UMLPackage*>(umlParent);
                if (parentPkg == NULL) {
                    kError() << pfx << "umlParent(" << umlParent << ") is not a UMLPackage"
                        << endl;
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
            nID = m_doc->getRootFolder(mt)->getID();
        } else if (Model_Utils::typeIsFolder(lvType)) {
            // Pre-1.2 format: Folders did not have their ID set.
            // Pull a new ID now.
            nID = UniqueID::get();
        } else {
            kError() << pfx << "item of type " << type << " has no ID, skipping." << endl;
            domElement = node.toElement();
            continue;
        }

        switch( lvType ) {
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
            item = findItem(nID);
            if (item == NULL) {
                kError() << pfx << "INTERNAL ERROR: "
                    << "findItem(id " << ID2STR(nID) << ") returns NULL" << endl;
                /*
                if (pObject && pObject->getUMLPackage() &&
                        parent->getType() != Uml::lvt_Package) {
                    // Pre-1.2 file format:
                    // Objects were not nested in their packages.
                    // Synthesize the nesting here.
                    UMLPackage *umlpkg = pObject->getUMLPackage();
                    UMLListViewItem *pkgItem = findUMLObject(umlpkg);
                    if (pkgItem == NULL) {
                        kDebug() << pfx << "synthesizing ListViewItem for package "
                            << ID2STR(umlpkg->getID()) << endl;
                        pkgItem = new UMLListViewItem(parent, umlpkg->getName(),
                                                      Uml::lvt_Package, umlpkg);
                        pkgItem->setOpen(true);
                    }
                    item = new UMLListViewItem(pkgItem, label, lvType, pObject);
                } else {
                    item = new UMLListViewItem(parent, label, lvType, pObject);
                }
                 */
            }
            else if (parent != item->parent()) {
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
                kDebug() << pfx << item->getText() << " parent "
                    << parent->getText() << " (" << parent << ") != "
                    << itmParent->getText() << " (" << itmParent << ")" << endl;
                if (item == m_datatypeFolder && itmParent == m_lv[Uml::mt_Logical]) {
                    kDebug() << pfx << "Reparenting the Datatypes folder is prohibited" << endl;
                } else {
                    UMLListViewItem *newItem = moveObject(nID, lvType, parent);
                    item = newItem;
                    if (item) {
                        kDebug() << pfx << "Attempted reparenting of " << item->getText()
                            << "(current parent: " << (itmParent ? itmParent->getText() : "NULL")
                            << ", new parent: " << parent->getText() << ")" << endl;
                    }
                }
            }
            break;
        case Uml::lvt_Attribute:
        case Uml::lvt_EntityAttribute:
        case Uml::lvt_Template:
        case Uml::lvt_Operation:
        case Uml::lvt_EnumLiteral:
            item = findItem(nID);
            if (item == NULL) {
                kDebug() << pfx << "item " << ID2STR(nID) << " (of type "
                    << lvType << ") does not yet exist..." << endl;
                UMLObject* umlObject = parent->getUMLObject();
                if (!umlObject) {
                    kDebug() << "And also the parent->getUMLObject() does not exist" << endl;
                    return false;
                }
                if (nID == Uml::id_None) {
                    kWarning() << pfx << "lvtype " << lvType << " has id -1" << endl;
                } else {
                    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(umlObject);
                    if (classifier) {
                        umlObject = classifier->findChildObjectById(nID);
                        if (umlObject) {
                            connectNewObjectsSlots(umlObject);
                            label = umlObject->getName();
                            item = new UMLListViewItem( parent, label, lvType, umlObject);
                        } else {
                            kDebug() << pfx << "lvtype " << lvType << " child object "
                                << ID2STR(nID) << " not found" << endl;
                        }
                    } else {
                        kDebug() << pfx << "cast to classifier object failed" << endl;
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
                item = new UMLListViewItem( parent, label, lvType, nID );
            } else {
                kError() << pfx << "INTERNAL ERROR: unexpected listview type "
                    << lvType << " (ID " << ID2STR(nID) << ")" << endl;
            }
            break;
        }//end switch

        if (item)  {
            item->setOpen( (bool)bOpen );
            if ( !loadChildrenFromXMI(item, domElement) ) {
                return false;
            }
        } else {
            kWarning() << "unused list view item " << ID2STR(nID)
                << " of lvtype " << lvType << endl;
        }
        domElement = node.toElement();
    }//end while
    return true;
}

/** Open all items in the list view*/
void UMLListView::expandAll(QListViewItem *item) {
    if(!item) item = firstChild();
    for (item = item->firstChild(); item; item = item->nextSibling())  {
        item->setOpen(true);
    }
}
/** Close all items in the list view*/
void UMLListView::collapseAll(QListViewItem *item) {
    if(!item) item = firstChild();
    for( item = item->firstChild(); item; item = item->nextSibling())
        item->setOpen(false);
}

void UMLListView::setStartedCut(bool startedCut) {
    m_bStartedCut = startedCut;
}

void UMLListView::setStartedCopy(bool startedCopy) {
    m_bStartedCopy = startedCopy;
}

bool UMLListView::startedCopy() const {
    return m_bStartedCopy;
}

UMLListViewItem *UMLListView::rootView(Uml::ListView_Type type) {
    UMLListViewItem *theView = NULL;
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

void UMLListView::deleteChildrenOf(QListViewItem* parent) {
    if ( !parent ) {
        return;
    }
    if (parent == m_lv[Uml::mt_Logical])
        m_datatypeFolder = NULL;
    while ( parent->firstChild() ) {
        delete parent->firstChild();
    }
}

void UMLListView::closeDatatypesFolder()  {
    m_datatypeFolder->setOpen(false);
}


bool UMLListView::deleteItem(UMLListViewItem *temp) {
    if (!temp)
        return false;
    UMLObject *object = temp->getUMLObject();
    Uml::ListView_Type lvt = temp->getType();
    if ( Model_Utils::typeIsDiagram(lvt) ) {
        m_doc->removeDiagram( temp->getID() );
    } else if (temp == m_datatypeFolder) {
        // we can't delete the datatypeFolder because umbrello will crash without a special handling
        return false;
    } else if (Model_Utils::typeIsCanvasWidget(lvt) || Model_Utils::typeIsClassifierList(lvt)) {
        UMLPackage *nmSpc = dynamic_cast<UMLPackage*>(object);
        if (nmSpc) {
            UMLObjectList contained = nmSpc->containedObjects();
            if (contained.count()) {
                KMessageBox::error(
                    kapp->mainWidget(),
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
        kWarning() << "umllistview::listpopupmenu::mt_Delete called with unknown type"
            << endl;
    }
    return true;
}


#include "umllistview.moc"
