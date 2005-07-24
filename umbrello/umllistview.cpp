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

// app includes
#include "inputdialog.h"
#include "actor.h"
#include "classimport.h"
#include "classifier.h"
#include "package.h"
#include "component.h"
#include "node.h"
#include "artifact.h"
#include "datatype.h"
#include "enum.h"
#include "entity.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "template.h"
#include "operation.h"
#include "attribute.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistviewitemlist.h"
#include "umllistviewitem.h"
#include "umlview.h"
#include "usecase.h"
#include "model_utils.h"
#include "clipboard/idchangelog.h"
#include "clipboard/umldrag.h"
#include "dialogs/classpropdlg.h"
#include "dialogs/umlattributedialog.h"
#include "dialogs/umloperationdialog.h"
#include "dialogs/umltemplatedialog.h"

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


UMLListView::UMLListView(QWidget *parent, const char *name)
        : KListView(parent,name), m_pMenu(0), m_doc(UMLApp::app()->getDocument())
{
    loadPixmaps();

    //setup list view
    setBackgroundColor(white);
    setAcceptDrops(true);
    setDropVisualizer(false);
    setItemsMovable(true);
    setItemsRenameable( true );
    setSelectionModeExt(FileManager);
    setFocusPolicy(QWidget::StrongFocus);
    setDragEnabled(TRUE);
    setColumnWidthMode( 0, Manual );
    setDefaultRenameAction( Accept );
    setResizeMode( LastColumn );
    header()->setClickEnabled(true);
    //add columns and initial items
    addColumn(m_doc->getName());

    m_rv =  new UMLListViewItem(this, i18n("Views"), Uml::lvt_View);
    m_ucv = new UMLListViewItem(m_rv, i18n("Use Case View"), Uml::lvt_UseCase_View);
    m_lv = new UMLListViewItem(m_rv, i18n("Logical View"), Uml::lvt_Logical_View);
    componentView = new UMLListViewItem(m_rv, i18n("Component View"), Uml::lvt_Component_View);
    deploymentView = new UMLListViewItem(m_rv, i18n("Deployment View"), Uml::lvt_Deployment_View);
    entityRelationshipModel = new UMLListViewItem(m_rv, i18n("Entity Relationship Model"), Uml::lvt_EntityRelationship_Model);
    datatypeFolder = new UMLListViewItem(m_lv, i18n("Datatypes"), Uml::lvt_Datatype_Folder);

#ifdef WANT_LVTOOLTIP
    /* In KDE-3.3, we cannot use KListView's builtin mechanism for
       overriding the tooltips. Instead, see the above class LVToolTip.
    setShowToolTips( true );
    setTooltipColumn( 0 );
     */
    (void) new LVToolTip(viewport());
#endif
    init();
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
    if (me->button() == RightButton) {
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
    if( m_doc -> getCurrentView() )
        m_doc -> getCurrentView() -> clearSelected();
    if( me -> state() != ShiftButton )
        clearSelection();
    QPoint pt = this->QScrollView::contentsToViewport( me->pos() );
    UMLListViewItem * item = (UMLListViewItem*)itemAt(pt);

    if(  me -> button() != RightButton && me -> button() != LeftButton  ) {
        UMLApp::app() -> getDocWindow() -> updateDocumentation( true );
        return;
    }

    Uml::ListView_Type type;
    type = item ? item->getType() : Uml::lvt_Unknown;
    if (me->button() == LeftButton) {
        switch( type ) {

        case Uml::lvt_UseCase:
        case Uml::lvt_Class:
        case Uml::lvt_Package:
        case Uml::lvt_Component:
        case Uml::lvt_Node:
        case Uml::lvt_Interface:
        case Uml::lvt_Datatype:
        case Uml::lvt_Enum:
        case Uml::lvt_Entity:
        case Uml::lvt_Actor:
        case Uml::lvt_Attribute:
        case Uml::lvt_Operation:
        case Uml::lvt_Template:
            UMLApp::app() -> getDocWindow() -> showDocumentation( item -> getUMLObject(), false );
            break;

        default:
            UMLApp::app() -> getDocWindow() -> updateDocumentation( true );
            break;
        }
    }
    if(me->button() == RightButton) {
        if(m_pMenu != 0) {
            m_pMenu->hide();
            disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(popupMenuSel(int)));
            delete m_pMenu;
            m_pMenu = 0;
        }

        m_pMenu = new ListPopupMenu(this, type);
        m_pMenu->popup(me->globalPos());
        connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(popupMenuSel(int)));
    }//end if right button

    this->KListView::contentsMousePressEvent(me);
}

void UMLListView::contentsMouseReleaseEvent(QMouseEvent *me) {
    if (me->button() != LeftButton) {
        this->KListView::contentsMouseReleaseEvent(me);
        return;
    }
    const QPoint pt = this->QScrollView::contentsToViewport( me->pos() );
    UMLListViewItem *item = dynamic_cast<UMLListViewItem*>(itemAt(pt));
    if (item == NULL || !typeIsDiagram(item->getType())) {
        this->KListView::contentsMouseReleaseEvent(me);
        return;
    }
    // Switch to diagram on mouse release - not on mouse press
    // because the user might intend a drag-to-note.
    m_doc->changeCurrentView( item->getID() );
    emit diagramSelected( item->getID() );
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
        QListView::keyPressEvent(ke); // let parent handle it
    }
}

void UMLListView::popupMenuSel(int sel) {
    UMLListViewItem * temp = (UMLListViewItem*)currentItem();
    if ( !temp ) {
        kdDebug() << "popupMenuSel invoked without currently selectedItem" << endl;
        return;
    }
    UMLObject * object = temp -> getUMLObject();
    Uml::ListView_Type lvt = temp -> getType();
    Uml::Object_Type umlType = Uml::ot_UMLObject;
    QString name;

    switch(sel) {
    case ListPopupMenu::mt_Activity_Diagram:
        m_doc->createDiagram(Uml::dt_Activity);
        break;

    case ListPopupMenu::mt_UseCase_Diagram:
        m_doc->createDiagram(Uml::dt_UseCase);
        break;

    case ListPopupMenu::mt_Sequence_Diagram:
        m_doc->createDiagram(Uml::dt_Sequence);
        break;

    case ListPopupMenu::mt_Collaboration_Diagram:
        m_doc->createDiagram(Uml::dt_Collaboration);
        break;

    case ListPopupMenu::mt_Class_Diagram:
        m_doc->createDiagram(Uml::dt_Class);
        break;

    case ListPopupMenu::mt_State_Diagram:
        m_doc->createDiagram(Uml::dt_State);
        break;

    case ListPopupMenu::mt_Component_Diagram:
        m_doc->createDiagram(Uml::dt_Component);
        break;

    case ListPopupMenu::mt_Deployment_Diagram:
        m_doc->createDiagram(Uml::dt_Deployment);
        break;

    case ListPopupMenu::mt_EntityRelationship_Diagram:
        m_doc->createDiagram(Uml::dt_EntityRelationship);
        break;

    case ListPopupMenu::mt_Class:
        addNewItem( temp, Uml::lvt_Class );
        break;

    case ListPopupMenu::mt_Package:
        addNewItem(temp, Uml::lvt_Package);
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
        m_doc->getCurrentView()->exportImage();
        break;

    case ListPopupMenu::mt_Externalize_Folder:
        {
            // configure & show the file dialog
            KFileDialog fileDialog(m_doc->URL().directory(), "*.xm1", this,
                                   ":externalize-folder", true);
            fileDialog.setCaption(i18n("Externalize Folder"));
            fileDialog.setOperationMode(KFileDialog::Other);
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            // set a sensible default filename
            QString defaultFilename = current->getFolderFile();
            if (defaultFilename.isEmpty()) {
                defaultFilename = current->getText().lower();
                defaultFilename.replace( QRegExp("\\W+"), "_" );
                defaultFilename.append(".xm1");  // default extension
            }
            fileDialog.setSelection(defaultFilename.lower());
            fileDialog.exec();
            KURL selURL = fileDialog.selectedURL();
            if (selURL.isEmpty())
                return;
            QString fileName = selURL.fileName();
            current->setFolderFile( fileName );
            // Recompute text of the folder
            QString folderText = current->getText();
            folderText.remove( QRegExp("\\s*\\(.*$") );
            folderText.append( " (" + fileName + ")" );
            current->setText(folderText);
            break;
        }

    case ListPopupMenu::mt_Internalize_Folder:
        {
            UMLListViewItem *current = static_cast<UMLListViewItem*>(currentItem());
            current->setFolderFile( QString::null );
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
        if ( typeIsDiagram(lvt) ) {
            m_doc->removeDiagram( temp->getID() );
        } else if( typeIsFolder(lvt) ) {
            if ( temp->firstChild() ) {
                KMessageBox::error(
                    kapp->mainWidget(),
                    i18n("The folder must be emptied before it can be deleted."),
                    i18n("Folder Not Empty"));
            } else {
                delete temp;
            }
        } else if ( typeIsCanvasWidget(lvt) || typeIsClassifierList(lvt) ) {
            if (lvt == Uml::lvt_Package || lvt == Uml::lvt_Class) {
                UMLPackage *nmSpc = dynamic_cast<UMLPackage*>(object);
                if (nmSpc == NULL) {
                    kdError() << "internal problem: object is not a package" << endl;
                    return;
                }
                UMLObjectList contained = nmSpc->containedObjects();
                for (UMLObjectListIt it(contained); it.current(); ++it) {
                    UMLObject *obj = it.current();
                    moveObject(obj->getID(), convert_OT_LVT(obj->getBaseType()), m_lv);
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
                canvasObj->removeAllAssociations();
            }
            m_doc->removeUMLObject(object);
        } else {
            kdWarning() << "umllistview::listpopupmenu::mt_Delete called with unknown type"
            << endl;
        }
        return;
        break;

    case ListPopupMenu::mt_Properties:
        /* first check if we are on a diagram */
        if( typeIsDiagram(lvt) ) {
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

        if ( typeIsCanvasWidget(lvt) ) {
            m_doc->showProperties(object, ClassPropDlg::page_gen);
        } else if(umlType == Uml::ot_Attribute) {
            // show the attribute dialogue
            UMLAttribute* selectedAttribute = static_cast<UMLAttribute*>(object);
            UMLAttributeDialog dialogue( this, selectedAttribute );
            dialogue.exec();
        } else if(umlType == Uml::ot_Operation) {
            // show the operation dialogue
            UMLOperation* selectedOperation = static_cast<UMLOperation*>(object);
            UMLOperationDialog dialogue( this, selectedOperation );
            dialogue.exec();
        } else if(umlType == Uml::ot_Template) {
            // show the template dialogue
            UMLTemplate* selectedTemplate = static_cast<UMLTemplate*>(object);
            UMLTemplateDialog dialogue( this, selectedTemplate );
            dialogue.exec();
        } else {
            kdWarning() << "calling properties on unknown type" << endl;
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
        kdWarning() << "UMLListView::popupMenuSel called with unknown type" << endl;
        //must be something we don't want to do
        break;
    }//end switch
}

void UMLListView::slotDiagramCreated( Uml::IDType id ) {
    if( m_doc->loading() )
        return;
    UMLListViewItem * temp = 0, *p = 0;
    UMLView *v = m_doc -> findView( id );
    if( !v )
        return;
    //See if we wanted to create diagram in folder
    UMLListViewItem * current = (UMLListViewItem *) currentItem();
    if ( current && typeIsFolder(current->getType()) ) {
        p = current;
    } else if (v->getType() == Uml::dt_UseCase) {
        p = m_ucv;
    } else if (v->getType() == Uml::dt_Component) {
        p = componentView;
    } else if (v->getType() == Uml::dt_Deployment) {
        p = deploymentView;
    } else if (v->getType() == Uml::dt_EntityRelationship) {
        p = entityRelationshipModel;
    } else {
        p = m_lv;
    }
    temp = new UMLListViewItem( p, v->getName(), convert_DT_LVT( v->getType() ),  id );
    setSelected( temp, true );
    UMLApp::app() -> getDocWindow() -> showDocumentation( v , false );
}

void UMLListView::slotObjectCreated(UMLObject* object) {
    /* kdDebug() << "UMLListView::slotObjectCreated(" << object->getName()
    	  << "): ID is " << object->getID() << endl;
     */
    UMLListViewItem* newItem = 0;
    UMLListViewItem* parentItem = 0;
    UMLListViewItem* current = (UMLListViewItem*) currentItem();
    Uml::Object_Type type = object->getBaseType();
    Uml::ListView_Type lvt = Uml::lvt_Unknown;
    if (current)
        lvt = current->getType();

    switch( type )
    {
    case Uml::ot_Attribute:
    case Uml::ot_Operation:
    case Uml::ot_Template:
    case Uml::ot_EnumLiteral:
    case Uml::ot_EntityAttribute:
        //this will be handled by childObjectAdded
        return;
        break;
    case Uml::ot_Association:
    case Uml::ot_Role:
    case Uml::ot_Stereotype:
        return;  // currently no representation in list view
        break;
    case Uml::ot_Class:
    case Uml::ot_Interface:
    case Uml::ot_Enum:
    case Uml::ot_Package:
    case Uml::ot_Datatype:
        {
            UMLPackage *pkg = object->getUMLPackage();
            if (pkg) {
                UMLListViewItem* pkgItem = findUMLObject(pkg);
                if (pkgItem == NULL)
                    kdDebug() << "UMLListView::slotObjectCreated: could not find "
                    << "parent package " << pkg->getName() << endl;
                else
                    parentItem = pkgItem;
            }
            if (parentItem) {
                //CHECK: Are these assignments necessary here?
                // They should already have been done at object creation.
                UMLPackage *pkg = (UMLPackage*)parentItem->getUMLObject();
                object->setUMLPackage(pkg);
                if (pkg)
                    pkg->addObject(object);
            } else if (lvt == Uml::lvt_Logical_Folder)
                parentItem = current;
            else if (type == Uml::ot_Datatype)
                parentItem = datatypeFolder;
            else
                parentItem = m_lv;
        }
        break;
    case Uml::ot_Actor:
    case Uml::ot_UseCase:
        if ( lvt == Uml::lvt_UseCase_Folder )
            parentItem = current;
        else
            parentItem = m_ucv;
        break;
    case Uml::ot_Component:
    case Uml::ot_Artifact:
        if( lvt == Uml::lvt_Component_Folder )
            parentItem = current;
        else
            parentItem = componentView;
        break;
    case Uml::ot_Node:
        if (lvt == Uml::lvt_Deployment_Folder) {
            parentItem = current;
        } else {
            parentItem = deploymentView;
        }
        break;
    case Uml::ot_Entity:
        if (lvt == Uml::lvt_EntityRelationship_Folder) {
            parentItem = current;
        } else {
            parentItem = entityRelationshipModel;
        }
        break;
    default:
        kdWarning() << "UMLListView::slotObjectCreated("<< object->getName()
        << ") : no appropriate parent found for type " << type
        << endl;
        return;
    }

    connectNewObjectsSlots(object);
    newItem = new UMLListViewItem(parentItem, object->getName(), convert_OT_LVT(type), object);
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
        connect(object,SIGNAL(attributeAdded(UMLObject*)),
                this,SLOT(childObjectAdded(UMLObject*)));
        connect(object,SIGNAL(attributeRemoved(UMLObject*)),
                this,SLOT(childObjectRemoved(UMLObject*)));
        connect(object,SIGNAL(childObjectAdded(UMLObject*)), // for operations
                this,SLOT(childObjectAdded(UMLObject*)));
        connect(object,SIGNAL(childObjectRemoved(UMLObject*)),
                this,SLOT(childObjectRemoved(UMLObject*)));
        connect(object,SIGNAL(templateAdded(UMLObject*)),
                this,SLOT(childObjectAdded(UMLObject*)));
        connect(object,SIGNAL(templateRemoved(UMLObject*)),
                this,SLOT(childObjectRemoved(UMLObject*)));
        connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
        break;
    case Uml::ot_Interface:
        connect(object,SIGNAL(childObjectAdded(UMLObject*)),
                this,SLOT(childObjectAdded(UMLObject*)));
        connect(object,SIGNAL(childObjectRemoved(UMLObject*)), // for operations
                this,SLOT(childObjectRemoved(UMLObject*)));
        connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
        break;
    case Uml::ot_Datatype:
    case Uml::ot_Enum:
    case Uml::ot_Entity:
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
        connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
        break;
    case Uml::ot_UMLObject:
    case Uml::ot_Association:
    case Uml::ot_Stereotype:
        break;
    default:
        kdWarning() << "unknown type in connectNewObjectsSlots" << endl;
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

void UMLListView::childObjectAdded(UMLObject* obj) {
    UMLObject *parent = const_cast<UMLObject*>(dynamic_cast<const UMLObject*>(sender()));
    childObjectAdded(obj, parent);
}

void UMLListView::childObjectAdded(UMLObject* obj, UMLObject* parent) {
    Uml::Object_Type ot = obj->getBaseType();
    /* kdDebug() << "UMLListView::childObjectAdded(" << obj->getName()
    		  << ", type " << ot << "): ID is " << obj->getID() << endl;
     */
    if (ot == Uml::ot_Stereotype || m_bIgnoreChildCreationSignal) {
        return;
    }
    if (!m_bCreatingChildObject) {
        UMLListViewItem *parentItem = findUMLObject(parent);
        UMLClassifierListItem *child = static_cast<UMLClassifierListItem*>(obj);
        QString text = child->toString(Uml::st_SigNoScope);
        UMLListViewItem *newItem = new UMLListViewItem(parentItem, text,
                                   convert_OT_LVT(obj->getBaseType()), obj);
        if (! m_doc->loading()) {
            ensureItemVisible(newItem);
            clearSelection();
            setSelected(newItem, true);
        }
    }
    connectNewObjectsSlots(obj);
}

void UMLListView::childObjectRemoved(UMLObject* obj) {
    UMLObject *parent = const_cast<UMLObject*>(dynamic_cast<const UMLObject*>(sender()));
    UMLListViewItem *item(0);
    UMLListViewItem *parentItem = findUMLObject(parent);
    for( item = static_cast<UMLListViewItem*>(parentItem->firstChild());
            item;
            item = static_cast<UMLListViewItem*>(item->nextSibling()) )
    {
        if(item->getUMLObject() == obj)
        {
            delete item;
            return;
        }
    }
}

void UMLListView::slotDiagramRenamed(Uml::IDType id) {
    UMLListViewItem* temp;
    UMLView* v = m_doc->findView(id);
    temp = findView(v);
    temp->setText( v->getName() );
}

void UMLListView::setDocument(UMLDoc *d) {
    if( m_doc && m_doc != d)
    {
        //disconnect signals from old doc and reset view
    }
    m_doc = d;

    Settings::OptionState optionState = UMLApp::app()->getOptionState();
    if (! optionState.generalState.tabdiagrams) {
        connect(m_doc, SIGNAL(sigDiagramCreated(Uml::IDType)), this, SLOT(slotDiagramCreated(Uml::IDType)));
        connect(m_doc, SIGNAL(sigDiagramRemoved(Uml::IDType)), this, SLOT(slotDiagramRemoved(Uml::IDType)));
        connect(m_doc, SIGNAL(sigDiagramRenamed(Uml::IDType)), this, SLOT(slotDiagramRenamed(Uml::IDType)));
    }
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
    selecteditems.setAutoDelete( FALSE );
    UMLListViewItemListIt it(selecteditems);
    UMLListViewItem * item = 0;
    UMLListViewItemList  list;
    list.setAutoDelete( FALSE );
    while((item=it.current()) != 0) {
        ++it;
        Uml::ListView_Type type = item->getType();
        if ( !typeIsCanvasWidget(type) && !typeIsDiagram(type) && !typeIsFolder(type)
                && type != Uml::lvt_Attribute && type != Uml::lvt_Operation) {
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

UMLListViewItem * UMLListView::findUMLObject(UMLObject *p) const {
    UMLListViewItem *item = static_cast<UMLListViewItem*>(firstChild());
    while (item) {
        UMLListViewItem *testItem = item->findUMLObject(p);
        if (testItem)
            return testItem;
        item = static_cast<UMLListViewItem*>(item->nextSibling());
    }
    return item;
}

void UMLListView::changeIconOf(UMLObject *o, Icon_Type to) {
    UMLListViewItem *item = findUMLObject(o);
    if (item == NULL)
        return;
    item->setPixmap(0, getPixmap(to));
}

UMLListViewItem* UMLListView::findView(UMLView* v) {
    if (!v) {
        kdWarning() << "returning 0 from UMLListView::findView()" << endl;
        return 0;
    }
    UMLListViewItem* item;
    Uml::Diagram_Type dType = v->getType();
    Uml::ListView_Type type = convert_DT_LVT( dType );
    Uml::IDType id = v->getID();
    if (dType == Uml::dt_UseCase) {
        item = m_ucv;
    } else if (dType == Uml::dt_Component) {
        item = componentView;
    } else if (dType == Uml::dt_Deployment) {
        item = deploymentView;
    } else if (dType == Uml::dt_EntityRelationship) {
        item = entityRelationshipModel;
    } else {
        item = m_lv;
    }

    UMLListViewItem* searchStartItem = (UMLListViewItem *)item->firstChild();

    UMLListViewItem* foundItem = recursiveSearchForView(searchStartItem, type, id);

    if (!foundItem) {
        kdWarning() << "returning 0 at UMLListView::findView" << endl;
    }
    return foundItem;
}

UMLListViewItem* UMLListView::recursiveSearchForView(UMLListViewItem* listViewItem,
        Uml::ListView_Type type, Uml::IDType id) {
    while (listViewItem) {
        if ( typeIsFolder(listViewItem->getType()) ) {
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
    deleteChildrenOf( m_ucv );
    deleteChildrenOf( m_lv );
    deleteChildrenOf( componentView );
    deleteChildrenOf( deploymentView );
    deleteChildrenOf( entityRelationshipModel );
    //Uncomment for using Luis diagram display code
    //	deleteChildrenOf( diagramFolder );

    m_rv->setOpen(true);
    m_ucv->setOpen(true);
    m_lv->setOpen(true);
    datatypeFolder->setOpen(false);
    componentView->setOpen(true);
    deploymentView->setOpen(true);
    entityRelationshipModel->setOpen(true);

    //setup misc.
    delete m_pMenu;
    m_pMenu = 0;
    m_bStartedCut = m_bStartedCopy = false;
    m_bIgnoreCancelRename = true;
    m_bCreatingChildObject = false;
    m_bIgnoreChildCreationSignal = false;
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
    if( !item || me -> button() != LeftButton )
        return;
    //see if on view
    Uml::ListView_Type lvType = item -> getType();
    if( typeIsDiagram(lvType) ) {
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
        m_doc -> showProperties(object, page);
    item -> cancelRename( 0 );//double click can cause it to go into rename mode.
}


bool UMLListView::acceptDrag(QDropEvent* event) const {
    QPoint mousePoint = ((UMLListView*)this)->contentsToViewport( event->pos() );

    UMLListViewItem* item = (UMLListViewItem*)itemAt(mousePoint);
    if(!item) {
        kdDebug() << "UMLListView::acceptDrag: itemAt(mousePoint) returns NULL"
        << endl;
        return false;
    }
    ((QListView*)this)->setCurrentItem( (QListViewItem*)item );

    UMLDrag::LvTypeAndID_List list;
    if (! UMLDrag::getClip3TypeAndID(event, list)) {
        kdDebug() << "UMLListView::acceptDrag: UMLDrag::getClip3TypeAndID returns false"
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
        case Uml::lvt_Actor:
        case Uml::lvt_UseCase:
        case Uml::lvt_UseCase_Diagram:
            accept = (dstType == Uml::lvt_UseCase_Folder ||
                      dstType == Uml::lvt_UseCase_View);
            break;
        case Uml::lvt_Component:
        case Uml::lvt_Artifact:
        case Uml::lvt_Component_Diagram:
            accept = (dstType == Uml::lvt_Component_Folder);
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

    //kdDebug() << "UMLListView::acceptDrag: dstType = " << dstType
    //	  << ", accept=" << accept << endl;
    return accept;
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
            kdError() << "UMLListView::moveObject(" << srcObj->getName()
            << "): Cannot move onto self" << endl;
            return NULL;
        }
        UMLPackage *srcPkg = srcObj->getUMLPackage();
        if (srcPkg) {
            if (srcPkg == newParentObj) {
                kdError() << "UMLListView::moveObject(" << srcObj->getName()
                << "): Object is already in target package" << endl;
                return NULL;
            }
            srcPkg->removeObject(srcObj);
        }
    }

    Uml::ListView_Type newParentType = newParent->getType();
    kdDebug() << "UMLListView::moveObject: newParentType is " << newParentType << endl;
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
            delete move;
        }
        break;
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Component:
    case Uml::lvt_Artifact:
    case Uml::lvt_Component_Diagram:
        if (newParentType == Uml::lvt_Component_Folder ||
                newParentType == Uml::lvt_Component_View) {
            newItem = move->deepCopy(newParent);
            delete move;
        }
        break;
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_Node:
    case Uml::lvt_Deployment_Diagram:
        if (newParentType == Uml::lvt_Deployment_Folder ||
                newParentType == Uml::lvt_Deployment_View) {
            newItem = move->deepCopy(newParent);
            delete move;
        }
        break;
    case Uml::lvt_EntityRelationship_Folder:
    case Uml::lvt_Entity:
    case Uml::lvt_EntityRelationship_Diagram:
        if (newParentType == Uml::lvt_EntityRelationship_Folder ||
                newParentType == Uml::lvt_EntityRelationship_Model) {
            newItem = move->deepCopy(newParent);
            delete move;
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
            delete move;
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
            delete move;
            UMLCanvasObject *o = static_cast<UMLCanvasObject*>(newItem->getUMLObject());
            if (o == NULL) {
                kdDebug() << "moveObject: newItem's UMLObject is NULL"
                << endl;
            } else if (newParentType == Uml::lvt_Package ||
                       newParentType == Uml::lvt_Interface ||
                       newParentType == Uml::lvt_Class) {
                UMLPackage *pkg = static_cast<UMLPackage*>(newParentObj);
                o->setUMLPackage( pkg );
                pkg->addObject( o );
            } else {
                o->setUMLPackage( NULL );
                m_doc->addUMLObject( o );
            }
            m_doc->getCurrentView()->updateContainment(o);
        }
        break;
    case Uml::lvt_Attribute:
    case Uml::lvt_Operation:
        if (newParentType == Uml::lvt_Class ||
                newParentType == Uml::lvt_Interface) {
            // update list view
            newItem = move->deepCopy(newParent);
            delete move;
            // update model objects
            m_bIgnoreChildCreationSignal = true;
            UMLClassifier *oldParentClassifier = dynamic_cast<UMLClassifier*>(srcObj->parent());
            UMLClassifier *newParentClassifier = dynamic_cast<UMLClassifier*>(newParentObj);
            if (srcType == Uml::lvt_Attribute) {
                UMLAttribute *att = dynamic_cast<UMLAttribute*>(srcObj);
                att = oldParentClassifier->takeAttribute(att);
                // We can't use the existing 'att' directly
                // because its parent is fixed to the old classifier
                // and we have no way of changing that:
                // QObject does not permit changing the parent().
                if (att) {
                    UMLAttribute *newAtt = static_cast<UMLAttribute*>(
                                               newParentClassifier->createAttribute(
                                                   att->getName()));
                    newAtt->setType(att->getType());
                    newAtt->setScope(att->getScope());
                    newAtt->setInitialValue(att->getInitialValue());
                    newItem->setUMLObject(newAtt);
                    // Let's not forget to update the DocWindow::m_pObject
                    // because the old one is about to be physically deleted !
                    UMLApp::app()->getDocWindow()->showDocumentation(newAtt, true);
                    delete att;

                } else {
                    kdError() << "moveObject: oldParentClassifier->takeAttribute returns NULL"
                    << endl;
                }
            } else {
                UMLOperation *op = dynamic_cast<UMLOperation*>(srcObj);
                op = oldParentClassifier->takeOperation(op);
                // We can't use the existing 'op' directly
                // because its parent is fixed to the old classifier
                // and we have no way of changing that:
                // QObject does not permit changing the parent().
                if (op) {
                    bool isExistingOp;
                    Umbrello::NameAndType_List ntDummyList;
                    // We need to provide a dummy NameAndType_List
                    // else UMLClassifier::createOperation will
                    // bring up an operation dialog.
                    UMLOperation *newOp = newParentClassifier->createOperation(
                                              op->getName(), &isExistingOp, &ntDummyList);
                    newOp->setType(op->getType());
                    newOp->setScope(op->getScope());
                    UMLAttributeList *parmList = op->getParmList();
                    for (UMLAttributeListIt plit(*parmList); plit.current(); ++plit) {
                        UMLAttribute *parm = plit.current();
                        UMLAttribute *newParm = new UMLAttribute(newParentClassifier, parm->getName());
                        newParm->setScope(parm->getScope());
                        newParm->setType(parm->getType());
                        newParm->setInitialValue(parm->getInitialValue());
                        newOp->addParm(newParm);
                    }
                    newItem->setUMLObject(newOp);
                    // Let's not forget to update the DocWindow::m_pObject
                    // because the old one is about to be physically deleted !
                    UMLApp::app()->getDocWindow()->showDocumentation(newOp, true);
                    delete op;
                } else {
                    kdError() << "moveObject: oldParentClassifier->takeOperation returns NULL"
                    << endl;
                }
            }
            m_bIgnoreChildCreationSignal = false;
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
        kdDebug() << "UMLListView::slotDropped: item is NULL - doing nothing"
        << endl;
        return;
    }
    UMLDrag::LvTypeAndID_List srcList;
    if (! UMLDrag::getClip3TypeAndID(de, srcList)) {
        return;
    }
    UMLListViewItem *newParent = (UMLListViewItem*)item;
    kdDebug() << "slotDropped: newParent->getText() is " << newParent->getText()
    << endl;
    UMLDrag::LvTypeAndID_It it(srcList);
    UMLDrag::LvTypeAndID * src = 0;
    while((src = it.current()) != 0) {
        ++it;
        moveObject(src->id, src->type, newParent);
    }
}

int UMLListView::getSelectedItems(UMLListViewItemList &ItemList) {
    ItemList.setAutoDelete( FALSE );
    QListViewItemIterator it(this);
    // iterate through all items of the list view
    for ( ; it.current(); ++it ) {
        if ( it.current()->isSelected() ) {
            UMLListViewItem *item = (UMLListViewItem*)it.current();
            ItemList.append(item);
        }
    }
    kdDebug() << "UMLListView::getSelectedItems: selItems = "
    << ItemList.count() << endl;

    return (int)ItemList.count();
}

UMLListViewItem* UMLListView::createDiagramItem(UMLView *v) {
    Uml::ListView_Type lvt = convert_DT_LVT(v->getType());
    UMLListViewItem *parent = determineParentItem(lvt);
    UMLListViewItem *item = new UMLListViewItem(parent, v->getName(), lvt, v->getID());
    return item;
}

/** Creates a new UMLListViewItem from a UMLListViewItem,
    if parent is null the ListView Decides who is going to be
    the parent */
UMLListViewItem* UMLListView::createItem(UMLListViewItem& Data, IDChangeLog& IDChanges,
        UMLListViewItem* parent /*= 0*/) {
    UMLObject* pObject = 0;
    UMLView* v = 0;
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
    case Uml::lvt_Component:
    case Uml::lvt_Node:
    case Uml::lvt_Artifact:
    case Uml::lvt_Interface:
    case Uml::lvt_Datatype:
    case Uml::lvt_Enum:
    case Uml::lvt_Entity:
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
    case Uml::lvt_Logical_Folder:
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_EntityRelationship_Folder:
    case Uml::lvt_Datatype_Folder:
        item = new UMLListViewItem(parent, Data.getText(), lvt);
        break;
    case Uml::lvt_Attribute:
        {
            UMLClassifier *pClass =  static_cast<UMLClassifier*>(parent->getUMLObject());
            Uml::IDType newID = IDChanges.findNewID( Data.getID() );
            pObject = pClass -> findChildObject( newID );
            if (pObject) {
                item = new UMLListViewItem( parent, Data.getText(), lvt, pObject );
            } else {
                item = 0;
            }
            break;
        }
    case Uml::lvt_Operation:
    case Uml::lvt_Template:
        {
            UMLClassifier * pConcept =  (UMLClassifier *)parent -> getUMLObject();
            Uml::IDType newID = IDChanges.findNewID( Data.getID() );
            pObject = pConcept->findChildObject( newID );
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
        v = m_doc->findView(IDChanges.findNewID(Data.getID()));
        if(!v) {
            return 0;
        }
        item = new UMLListViewItem(parent, v->getName(), convert_DT_LVT( v->getType() ),  IDChanges.findNewID(Data.getID()));
        break;
    default:
        kdWarning() << "createItem() called on unknown type" << endl;
        break;
    }
    return item;
}

UMLListViewItem* UMLListView::determineParentItem(Uml::ListView_Type lvt) const {
    UMLListViewItem* parent = 0;
    switch (lvt) {
    case Uml::lvt_Actor:
    case Uml::lvt_UseCase:
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_UseCase_Diagram:
        parent = m_ucv;
        break;
    case Uml::lvt_Component_Diagram:
    case Uml::lvt_Component:
    case Uml::lvt_Artifact:
        parent = componentView;
        break;
    case Uml::lvt_Deployment_Diagram:
    case Uml::lvt_Node:
        parent = deploymentView;
        break;
    case Uml::lvt_EntityRelationship_Diagram:
    case Uml::lvt_Entity:
        parent = entityRelationshipModel;
        break;
    default:
        if (typeIsDiagram(lvt) || !typeIsClassifierList(lvt))
            parent = m_lv;
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

Uml::ListView_Type UMLListView::convert_DT_LVT(Uml::Diagram_Type dt) {
    Uml::ListView_Type type =  Uml::lvt_Unknown;
    switch(dt) {
    case Uml::dt_UseCase:
        type = Uml::lvt_UseCase_Diagram;
        break;

    case Uml::dt_Class:
        type = Uml::lvt_Class_Diagram;
        break;

    case Uml::dt_Sequence:
        type = Uml::lvt_Sequence_Diagram;
        break;

    case Uml::dt_Collaboration:
        type = Uml::lvt_Collaboration_Diagram;
        break;

    case Uml::dt_State:
        type = Uml::lvt_State_Diagram;
        break;

    case Uml::dt_Activity:
        type = Uml::lvt_Activity_Diagram;
        break;

    case Uml::dt_Component:
        type = Uml::lvt_Component_Diagram;
        break;

    case Uml::dt_Deployment:
        type = Uml::lvt_Deployment_Diagram;
        break;

    case Uml::dt_EntityRelationship:
        type = Uml::lvt_EntityRelationship_Diagram;
        break;

    default:
        kdWarning() << "convert_DT_LVT() called on unknown diagram type" << endl;
    }
    return type;
}

Uml::ListView_Type UMLListView::convert_OT_LVT(Uml::Object_Type ot) {
    Uml::ListView_Type type =  Uml::lvt_Unknown;
    switch(ot) {
    case Uml::ot_UseCase:
        type = Uml::lvt_UseCase;
        break;

    case Uml::ot_Actor:
        type = Uml::lvt_Actor;
        break;

    case Uml::ot_Class:
        type = Uml::lvt_Class;
        break;

    case Uml::ot_Package:
        type = Uml::lvt_Package;
        break;

    case Uml::ot_Component:
        type = Uml::lvt_Component;
        break;

    case Uml::ot_Node:
        type = Uml::lvt_Node;
        break;

    case Uml::ot_Artifact:
        type = Uml::lvt_Artifact;
        break;

    case Uml::ot_Interface:
        type = Uml::lvt_Interface;
        break;

    case Uml::ot_Datatype:
        type = Uml::lvt_Datatype;
        break;

    case Uml::ot_Enum:
        type = Uml::lvt_Enum;
        break;

    case Uml::ot_Entity:
        type = Uml::lvt_Entity;
        break;

    case Uml::ot_Attribute:
        type = Uml::lvt_Attribute;
        break;

    case Uml::ot_Operation:
        type = Uml::lvt_Operation;
        break;

    case Uml::ot_Template:
        type = Uml::lvt_Template;
        break;
    default:
        break;
    }
    return type;
}

Uml::Object_Type UMLListView::convert_LVT_OT(Uml::ListView_Type lvt) {
    Uml::Object_Type ot = (Uml::Object_Type)0;
    switch (lvt) {
    case Uml::lvt_UseCase:
        ot = Uml::ot_UseCase;
        break;

    case Uml::lvt_Actor:
        ot = Uml::ot_Actor;
        break;

    case Uml::lvt_Class:
        ot = Uml::ot_Class;
        break;

    case Uml::lvt_Package:
        ot = Uml::ot_Package;
        break;

    case Uml::lvt_Component:
        ot = Uml::ot_Component;
        break;

    case Uml::lvt_Node:
        ot = Uml::ot_Node;
        break;

    case Uml::lvt_Artifact:
        ot = Uml::ot_Artifact;
        break;

    case Uml::lvt_Interface:
        ot = Uml::ot_Interface;
        break;

    case Uml::lvt_Datatype:
        ot = Uml::ot_Datatype;
        break;

    case Uml::lvt_Enum:
        ot = Uml::ot_Enum;
        break;

    case Uml::lvt_Entity:
        ot = Uml::ot_Entity;
        break;

    case Uml::lvt_EntityAttribute:
        ot = Uml::ot_EntityAttribute;
        break;

    case Uml::lvt_Attribute:
        ot = Uml::ot_Attribute;
        break;

    case Uml::lvt_Operation:
        ot = Uml::ot_Operation;
        break;

    case Uml::lvt_Template:
        ot = Uml::ot_Template;
        break;

    default:
        break;
    }
    return ot;
}

QPixmap & UMLListView::getPixmap( Icon_Type type ) {
    switch( type ) {
    case it_Home:
        return m_Pixmaps.Home;
        break;

    case it_Folder_Cyan:
        return m_Pixmaps.Folder_Cyan;
        break;

    case it_Folder_Cyan_Open:
        return m_Pixmaps.Folder_Cyan_Open;
        break;

    case it_Folder_Green:
        return m_Pixmaps.Folder_Green;
        break;

    case it_Folder_Green_Open:
        return m_Pixmaps.Folder_Green_Open;
        break;

    case it_Folder_Grey:
        return m_Pixmaps.Folder_Grey;
        break;

    case it_Folder_Grey_Open:
        return m_Pixmaps.Folder_Grey_Open;
        break;

    case it_Folder_Orange:
        return m_Pixmaps.Folder_Orange;
        break;

    case it_Folder_Orange_Open:
        return m_Pixmaps.Folder_Orange_Open;
        break;

    case it_Folder_Red:
        return m_Pixmaps.Folder_Red;
        break;

    case it_Folder_Red_Open:
        return m_Pixmaps.Folder_Red_Open;
        break;

    case it_Folder_Violet:
        return m_Pixmaps.Folder_Violet;
        break;

    case it_Folder_Violet_Open:
        return m_Pixmaps.Folder_Violet_Open;
        break;

    case it_Diagram:
        return m_Pixmaps.Diagram;
        break;

    case it_Diagram_Activity:
        return m_Pixmaps.Diagram_Activity;
        break;

    case it_Diagram_Class:
        return m_Pixmaps.Diagram_Class;
        break;

    case it_Diagram_Collaboration:
        return m_Pixmaps.Diagram_Collaboration;
        break;

    case it_Diagram_Component:
        return m_Pixmaps.Diagram_Component;
        break;

    case it_Diagram_Deployment:
        return m_Pixmaps.Diagram_Deployment;
        break;

    case it_Diagram_EntityRelationship:
        return m_Pixmaps.Diagram_EntityRelationship;
        break;

    case it_Diagram_Sequence:
        return m_Pixmaps.Diagram_Sequence;
        break;

    case it_Diagram_State:
        return m_Pixmaps.Diagram_State;
        break;

    case it_Diagram_Usecase:
        return m_Pixmaps.Diagram_Usecase;
        break;

    case it_Class:
        return m_Pixmaps.Class;
        break;

    case it_Template:
        return m_Pixmaps.Template;
        break;

    case it_Package:
        return m_Pixmaps.Package;
        break;

    case it_Component:
        return m_Pixmaps.Component;
        break;

    case it_Node:
        return m_Pixmaps.Node;
        break;

    case it_Artifact:
        return m_Pixmaps.Artifact;
        break;

    case it_Interface:
        return m_Pixmaps.Interface;
        break;

    case it_Datatype:
        return m_Pixmaps.Datatype;
        break;

    case it_Enum:
        return m_Pixmaps.Enum;
        break;

    case it_Entity:
        return m_Pixmaps.Entity;
        break;

    case it_Actor:
        return m_Pixmaps.Actor;
        break;

    case it_UseCase:
        return m_Pixmaps.UseCase;
        break;

    case it_Public_Method:
        return m_Pixmaps.Public_Method;
        break;

    case it_Private_Method:
        return m_Pixmaps.Private_Method;
        break;

    case it_Protected_Method:
        return m_Pixmaps.Protected_Method;
        break;

    case it_Public_Attribute:
        return m_Pixmaps.Public_Attribute;
        break;

    case it_Private_Attribute:
        return m_Pixmaps.Private_Attribute;
        break;

    case it_Protected_Attribute:
        return m_Pixmaps.Protected_Attribute;
        break;

    default:
        kdWarning() << "getPixmap() called on unknown icon" << endl;
    }//end switch
    return m_Pixmaps.Home;//you'll know you have a problem if this shows up in the list!!
}

void UMLListView::loadPixmaps() {
    KStandardDirs * dirs = KGlobal::dirs();
    QString dataDir = dirs -> findResourceDir("data", "umbrello/pics/object.png");
    dataDir += "/umbrello/pics/";

    m_Pixmaps.Home = BarIcon("folder_home");
    m_Pixmaps.Folder_Cyan = BarIcon("folder");
    m_Pixmaps.Folder_Cyan_Open = BarIcon("folder_open");
    m_Pixmaps.Folder_Green = BarIcon("folder_green");
    m_Pixmaps.Folder_Green_Open = BarIcon("folder_green_open");
    m_Pixmaps.Folder_Orange = BarIcon("folder_orange");
    m_Pixmaps.Folder_Orange_Open = BarIcon("folder_orange_open");
    m_Pixmaps.Folder_Grey = BarIcon("folder_grey");
    m_Pixmaps.Folder_Grey_Open = BarIcon("folder_grey_open");
    m_Pixmaps.Folder_Red = BarIcon("folder_red");
    m_Pixmaps.Folder_Red_Open = BarIcon("folder_red_open");
    m_Pixmaps.Folder_Violet = BarIcon("folder_violet");
    m_Pixmaps.Folder_Violet_Open = BarIcon("folder_violet_open");

    m_Pixmaps.Diagram_Activity = BarIcon("umbrello_diagram_activity");
    m_Pixmaps.Diagram_Class = BarIcon("umbrello_diagram_class");
    m_Pixmaps.Diagram_Component = BarIcon("umbrello_diagram_component");
    m_Pixmaps.Diagram_State = BarIcon("umbrello_diagram_state");
    m_Pixmaps.Diagram_Sequence = BarIcon("umbrello_diagram_sequence");
    m_Pixmaps.Diagram_Deployment = BarIcon("umbrello_diagram_deployment");
    m_Pixmaps.Diagram_EntityRelationship = BarIcon("umbrello_diagram_deployment");
    m_Pixmaps.Diagram_Usecase = BarIcon("umbrello_diagram_usecase");
    m_Pixmaps.Diagram_Collaboration = BarIcon("umbrello_diagram_collaboration");

    m_Pixmaps.Diagram.load( dataDir + "CVnamespace.png" );
    m_Pixmaps.Class.load( dataDir + "class.png" );
    m_Pixmaps.Template.load( dataDir + "template.png" );
    m_Pixmaps.Package.load( dataDir + "package.png" );
    m_Pixmaps.Component.load( dataDir + "component.png" );
    m_Pixmaps.Node.load( dataDir + "node.png" );
    m_Pixmaps.Artifact.load( dataDir + "artifact.png" );
    m_Pixmaps.Interface.load( dataDir + "interface.png" );
    m_Pixmaps.Datatype.load( dataDir + "datatype.png" );
    m_Pixmaps.Enum.load( dataDir + "enum.png" );
    m_Pixmaps.Entity.load( dataDir + "entity.png" );
    m_Pixmaps.Actor.load( dataDir + "actor.png" );
    m_Pixmaps.UseCase.load( dataDir + "usecase.png" );
    m_Pixmaps.Public_Method.load( dataDir + "CVpublic_meth.png" );
    m_Pixmaps.Private_Method.load( dataDir + "CVprivate_meth.png" );
    m_Pixmaps.Protected_Method.load( dataDir + "CVprotected_meth.png" );
    m_Pixmaps.Public_Attribute.load( dataDir + "CVpublic_var.png" );
    m_Pixmaps.Private_Attribute.load( dataDir + "CVprivate_var.png" );
    m_Pixmaps.Protected_Attribute.load( dataDir + "CVprotected_var.png" );
}

void UMLListView::slotExpanded( QListViewItem * item ) {
    UMLListViewItem * myItem= (UMLListViewItem *)item;
    switch( myItem -> getType() ) {
    case Uml::lvt_Logical_View:
    case Uml::lvt_UseCase_View:
    case Uml::lvt_Component_View:
    case Uml::lvt_Deployment_View:
    case Uml::lvt_EntityRelationship_Model:
    case Uml::lvt_Logical_Folder:
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_EntityRelationship_Folder:
    case Uml::lvt_Datatype_Folder:
    case Uml::lvt_Package:
        myItem->updateFolder();
        break;
    default:
        break;
    }//end switch
}

void UMLListView::slotCollapsed( QListViewItem * item ) {
    UMLListViewItem * myItem = (UMLListViewItem *)item;
    switch( myItem -> getType() ) {
    case Uml::lvt_Logical_View:
    case Uml::lvt_UseCase_View:
    case Uml::lvt_Component_View:
    case Uml::lvt_Deployment_View:
    case Uml::lvt_EntityRelationship_Model:
    case Uml::lvt_Logical_Folder:
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_EntityRelationship_Folder:
    case Uml::lvt_Datatype_Folder:
    case Uml::lvt_Package:
        myItem->updateFolder();
        break;
    default:
        break;
    }//end switch
}

void UMLListView::slotCutSuccessful() {
    if( m_bStartedCut ) {
        popupMenuSel( ListPopupMenu::mt_Delete );
        //deletion code here
        m_bStartedCut = false;
    }
}

void UMLListView::addNewItem( QListViewItem * parent, Uml::ListView_Type type ) {
    QString name = i18n("folder");
    if (type == Uml::lvt_Datatype)  {
        parent = datatypeFolder;
    }
    UMLListViewItem * parentItem = static_cast<UMLListViewItem *>( parent );
    UMLListViewItem * newItem = NULL;
    parent -> setOpen( true );
    switch( type ) {
    case Uml::lvt_UseCase_Folder:
    case Uml::lvt_Logical_Folder:
    case Uml::lvt_Component_Folder:
    case Uml::lvt_Deployment_Folder:
    case Uml::lvt_EntityRelationship_Folder:
    case Uml::lvt_Datatype_Folder:
        newItem = new UMLListViewItem( parentItem,
                                       name, type, m_doc->getUniqueID() );
        break;

    case Uml::lvt_Actor:
        name = m_doc->uniqObjectName( Uml::ot_Actor );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject *)0 );
        newItem -> setPixmap( 0, getPixmap( it_Actor ) );
        break;

    case Uml::lvt_Class:
        name = m_doc->uniqObjectName( Uml::ot_Class);
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject *)0 );
        newItem -> setPixmap( 0, getPixmap( it_Class ) );
        break;

    case Uml::lvt_Package:
        name = m_doc->uniqObjectName( Uml::ot_Package );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject *)0 );
        newItem->setPixmap( 0, getPixmap( it_Package ) );
        break;

    case Uml::lvt_Component:
        name = m_doc->uniqObjectName( Uml::ot_Component );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject *)0 );
        newItem->setPixmap( 0, getPixmap( it_Component ) );
        break;

    case Uml::lvt_Node:
        name = m_doc->uniqObjectName( Uml::ot_Node );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject *)0 );
        newItem->setPixmap( 0, getPixmap( it_Node ) );
        break;

    case Uml::lvt_Artifact:
        name = m_doc->uniqObjectName( Uml::ot_Artifact );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject *)0 );
        newItem->setPixmap( 0, getPixmap( it_Artifact ) );
        break;

    case Uml::lvt_Interface:
        name = m_doc->uniqObjectName( Uml::ot_Interface );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject*)0 );
        newItem->setPixmap( 0, getPixmap( it_Interface ) );
        break;

    case Uml::lvt_Datatype:
        name = m_doc->uniqObjectName( Uml::ot_Datatype );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject*)0 );
        newItem->setPixmap( 0, getPixmap( it_Datatype ) );
        break;

    case Uml::lvt_Enum:
        name = m_doc->uniqObjectName( Uml::ot_Enum );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject*)0 );
        newItem->setPixmap( 0, getPixmap( it_Enum ) );
        break;

    case Uml::lvt_Entity:
        name = m_doc->uniqObjectName( Uml::ot_Entity );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject*)0 );
        newItem->setPixmap( 0, getPixmap( it_Entity ) );
        break;

    case Uml::lvt_Template:
        {
            UMLClassifier *parent = static_cast<UMLClassifier*>(parentItem->getUMLObject());
            name = parent->uniqChildName(Uml::ot_Template);
            newItem = new UMLListViewItem( parentItem, name, type, (UMLObject*)0 );
            newItem->setPixmap( 0, getPixmap( it_Template ) );
            break;
        }
    case Uml::lvt_Attribute:
        {
            UMLClassifier *umlParent = static_cast<UMLClassifier*>(parentItem->getUMLObject());
            name = umlParent->uniqChildName( Uml::ot_Attribute );
            newItem = new UMLListViewItem( parentItem, name, type, (UMLObject *)0 );
            newItem -> setPixmap( 0, getPixmap( it_Private_Attribute ) );
            break;
        }
    case Uml::lvt_Operation:
        {
            UMLClassifier * childParent = static_cast<UMLClassifier *>( parentItem->getUMLObject() );
            name = childParent->uniqChildName( Uml::ot_Operation );
            newItem = new UMLListViewItem( parentItem, name, type, (UMLObject *)0 );
            newItem -> setPixmap( 0, getPixmap( it_Public_Method ) );
            break;
        }
    case Uml::lvt_UseCase:
        name = m_doc->uniqObjectName( Uml::ot_UseCase );
        newItem = new UMLListViewItem( parentItem, name, type, (UMLObject *)0 );
        newItem -> setPixmap( 0, getPixmap( it_UseCase ) );
        break;

    case Uml::lvt_Class_Diagram:
        name = getUniqueDiagramName( Uml::dt_Class );
        newItem = new UMLListViewItem( parentItem, name, type, Uml::id_None );
        break;

    case Uml::lvt_UseCase_Diagram:
        name = getUniqueDiagramName( Uml::dt_UseCase );
        newItem = new UMLListViewItem( parentItem, name, type, Uml::id_None );
        break;

    case Uml::lvt_Sequence_Diagram:
        name = getUniqueDiagramName( Uml::dt_Sequence );
        newItem = new UMLListViewItem( parentItem, name, type, Uml::id_None );
        break;

    case Uml::lvt_Collaboration_Diagram:
        name = getUniqueDiagramName( Uml::dt_Collaboration );
        newItem = new UMLListViewItem( parentItem, name, type, Uml::id_None );
        break;

    case Uml::lvt_State_Diagram:
        name = getUniqueDiagramName( Uml::dt_State );
        newItem = new UMLListViewItem( parentItem, name, type, Uml::id_None );
        break;

    case Uml::lvt_Activity_Diagram:
        name = getUniqueDiagramName( Uml::dt_Activity );
        newItem = new UMLListViewItem( parentItem, name, type, Uml::id_None );
        break;

    case Uml::lvt_Component_Diagram:
        name = getUniqueDiagramName(Uml::dt_Component);
        newItem = new UMLListViewItem(parentItem, name, type, Uml::id_None);
        break;

    case Uml::lvt_Deployment_Diagram:
        name = getUniqueDiagramName(Uml::dt_Deployment);
        newItem = new UMLListViewItem(parentItem, name, type, Uml::id_None);
        break;

    case Uml::lvt_EntityRelationship_Diagram:
        name = getUniqueDiagramName(Uml::dt_EntityRelationship);
        newItem = new UMLListViewItem(parentItem, name, type, Uml::id_None);
        break;
    default:
        break;
    }
    if (newItem == NULL) {
        kdDebug() << "UMLListView::addNewItem: nothing added" << endl;
        return;
    }
    m_bIgnoreCancelRename = false;
    newItem->setOpen( true );
    newItem->setCreating( true );
    newItem->startRename( 0 );  // calls QListView::ensureItemVisible()
    // When the user accepts the rename operation, UMLListViewItem::okRename()
    // is called (automatically by QListViewItem.)
}

bool UMLListView::slotItemRenamed( QListViewItem * item , int /*col*/ ) {
    //if true the item was cancel before this message
    if( m_bIgnoreCancelRename ) {
        return true;
    }
    m_bIgnoreCancelRename = true;
    UMLListViewItem * renamedItem = static_cast< UMLListViewItem *>( item ) ;
    Uml::ListView_Type type = renamedItem -> getType();
    QString newText = renamedItem -> text( 0 );
    renamedItem -> setCreating( false );

    //if the length of any type then delete it.
    if( newText.length() == 0 ) {
        KMessageBox::error(
            kapp -> mainWidget(),
            i18n( "The name you entered was invalid.\nCreation process has been canceled." ),
            i18n( "Name Not Valid" ) );
        delete item;
        return false;
    }
    // No need to do anything for folders if the string length was > 0
    if( typeIsFolder(type) ) {
        return true;
    }

    if( !isUnique( renamedItem, newText ) ) {
        //if operation ask if ok not to be unique i.e overloading
        if( type == Uml::lvt_Operation ) {
            if( KMessageBox::warningYesNo(
                        kapp -> mainWidget(),
                        i18n( "The name you entered was not unique.\nIs this what you wanted?" ),
                        i18n( "Name Not Unique" ), i18n("Use Name"), i18n("Enter New Name") ) == KMessageBox::No ) {
                delete item;
                return false;
            }
        } else {
            KMessageBox::error(
                kapp -> mainWidget(),
                i18n( "The name you entered was not unique!\nCreation process has been canceled." ),
                i18n( "Name Not Unique" ) );
            delete item;
            return false;
        }
    }

    switch( type ) {
    case Uml::lvt_Actor:
    case Uml::lvt_Class:
    case Uml::lvt_Package:
    case Uml::lvt_Component:
    case Uml::lvt_Node:
    case Uml::lvt_Artifact:
    case Uml::lvt_Interface:
    case Uml::lvt_Datatype:
    case Uml::lvt_Enum:
    case Uml::lvt_Entity:
    case Uml::lvt_UseCase:
        {
            Uml::Object_Type ot = convert_LVT_OT(type);
            if (! ot) {
                kdError() << "UMLListView::slotItemRenamed() internal" << endl;
                return false;
            }
            createUMLObject( renamedItem, ot );
        }
        break;

    case Uml::lvt_Template:
        return createChildUMLObject( renamedItem, Uml::ot_Template );
        break;

    case Uml::lvt_Attribute:
        return createChildUMLObject( renamedItem, Uml::ot_Attribute );
        break;

    case Uml::lvt_Operation:
        return createChildUMLObject( renamedItem, Uml::ot_Operation );
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

void UMLListView::createUMLObject( UMLListViewItem * item, Uml::Object_Type type ) {
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
            c->setInterface();
            object = c;
        }
        break;

    case Uml::ot_Datatype:
        object = new UMLDatatype( name );
        break;

    case Uml::ot_Enum:
        object = new UMLEnum( name );
        break;

    case Uml::ot_Entity:
        object = new UMLEntity( name );
        break;

    default:
        kdWarning() << "creating UML Object of unknown type" << endl;
        return;
    }

    UMLListViewItem * parentItem = static_cast<UMLListViewItem *>(item->parent());
    if (parentItem->getType() == Uml::lvt_Package) {
        UMLPackage *pkg = static_cast<UMLPackage*>(parentItem->getUMLObject());
        object->setUMLPackage(pkg);
        pkg->addObject(object);
    }
    connectNewObjectsSlots(object);
    m_doc -> addUMLObject( object );
    item -> setUMLObject( object );
    item -> setText( name );
}

bool UMLListView::createChildUMLObject( UMLListViewItem * item, Uml::Object_Type type ) {
    m_bCreatingChildObject = true;
    QString text = item->text( 0 );
    UMLObject* parent = static_cast<UMLListViewItem *>( item->parent() )->getUMLObject();
    if( !parent ) {
        kdError() << "UMLListView::createChildUMLObject: parent UMLObject is NULL"
        << endl;
        delete item;
        m_bCreatingChildObject = false;
        return false;
    }

    //kdDebug() << "UMLListView::createChildUMLObject (" << text << ")" << endl;
    UMLObject* newObject = NULL;
    if ( type == Uml::ot_Template )  {
        UMLClassifier *owningClassifier = static_cast<UMLClassifier*>(parent);
        Umbrello::NameAndType nt;
        Umbrello::Parse_Status st = Umbrello::parseTemplate(text, nt, owningClassifier);
        if (st) {
            KMessageBox::error( kapp->mainWidget(),
                                Umbrello::psText(st),
                                i18n("Creation canceled") );
            delete item;
            m_bCreatingChildObject = false;
            return false;
        }
        newObject = owningClassifier->createTemplate(nt.m_name);
        UMLTemplate *tmplParm = static_cast<UMLTemplate*>(newObject);
        tmplParm->setType(nt.m_type);
        text = tmplParm->toString(Uml::st_SigNoScope);
    } else if ( type == Uml::ot_Attribute )  {
        UMLClassifier *owningClass = static_cast<UMLClassifier*>(parent);
        Umbrello::NameAndType nt;
        Umbrello::Parse_Status st = Umbrello::parseAttribute(text, nt, owningClass);
        if (st) {
            KMessageBox::error( kapp->mainWidget(),
                                Umbrello::psText(st),
                                i18n("Creation canceled") );
            delete item;
            m_bCreatingChildObject = false;
            return false;
        }
        newObject = owningClass->createAttribute(nt.m_name);
        UMLAttribute *att = static_cast<UMLAttribute*>(newObject);
        att->setType(nt.m_type);
        att->setInitialValue(nt.m_initialValue);
        text = att->toString(Uml::st_SigNoScope);
    } else if ( type == Uml::ot_Operation ) {
        UMLClassifier *owningClassifier = static_cast<UMLClassifier*>(parent);
        Umbrello::OpDescriptor od;
        Umbrello::Parse_Status st = Umbrello::parseOperation(text, od, owningClassifier);
        if (st) {
            KMessageBox::error( kapp->mainWidget(),
                                Umbrello::psText(st),
                                i18n("Creation canceled") );
            delete item;
            m_bCreatingChildObject = false;
            return false;
        }
        bool isExistingOp = false;
        newObject = owningClassifier->createOperation(od.m_name, &isExistingOp, &od.m_args);
        if (isExistingOp) {
            KMessageBox::error(
                kapp -> mainWidget(),
                i18n( "The name you entered was not unique!\nCreation process has been canceled." ),
                i18n( "Name Not Unique" ) );
            delete item;
            m_bCreatingChildObject = false;
            return false;
        }
        UMLOperation *op = static_cast<UMLOperation*>(newObject);
        if (od.m_pReturnType) {
            op->setType(od.m_pReturnType);
        }
        text = op->toString(Uml::st_SigNoScope);
    } else {
        kdError() << "UMLListView::createChildUMLObject called for type "
        << type << " (ignored)" << endl;
        m_bCreatingChildObject = false;
        return false;
    }

    item->setUMLObject( newObject );
    item->setText( text );
    m_bCreatingChildObject = false;

    //m_doc->setModified();
    return true;
}

void UMLListView::createDiagram( UMLListViewItem * item, Uml::Diagram_Type type ) {
    QString name = item -> text( 0 );
    UMLView * view = m_doc -> findView( type, name );
    if( view ) {
        delete item;
        return;
    }
    view = new UMLView();
    view->setName( name );
    view->setType( type );
    view->setID( m_doc -> getUniqueID() );
    m_doc -> addView( view );
    view -> setOptionState( UMLApp::app()->getOptionState() );
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
    case Uml::lvt_Component:
        return !m_doc->findUMLObject( name, convert_LVT_OT(type) );
        break;

    case Uml::lvt_Class:
    case Uml::lvt_Package:
    case Uml::lvt_Interface:
    case Uml::lvt_Datatype:
    case Uml::lvt_Enum:
    case Uml::lvt_Entity:
        {
            if (parentItem->getType() != Uml::lvt_Package)
                return (m_doc->findUMLObject(name) == NULL);
            UMLPackage *pkg = static_cast<UMLPackage*>(parentItem->getUMLObject());
            if (pkg == NULL) {
                kdError() << "UMLListView::isUnique: internal error - "
                << "parent listviewitem is package but has no UMLObject"
                << endl;
                return true;
            }
            return (pkg->findObject(name) == NULL);
            break;
        }

    case Uml::lvt_Template:
    case Uml::lvt_Attribute:
    case Uml::lvt_Operation:
        {
            UMLClassifier *parent = static_cast<UMLClassifier*>(parentItem->getUMLObject());
            Uml::Object_Type ot = convert_LVT_OT(type);
            return (parent->findChildObject(ot, name).count() == 0);
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

void UMLListView::saveToXMI( QDomDocument & qDoc, QDomElement & qElement,
                             bool saveSubmodelFiles /* = false */) {
    QDomElement listElement = qDoc.createElement( "listview" );
    m_rv -> saveToXMI( qDoc, listElement, saveSubmodelFiles);
    qElement.appendChild( listElement );
}

bool UMLListView::loadFromXMI( QDomElement & element ) {
    /*
    	deleteChildrenOf( m_ucv );
    	deleteChildrenOf( m_lv );
    	deleteChildrenOf( componentView );
    	deleteChildrenOf( deploymentView );
    //Uncomment for using Luis diagram display code
    //	deleteChildrenOf( diagramFolder );
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
                    kdDebug() << "UMLListView::loadChildrenFromXMI:"
                    << " using id " << ID2STR(newID)
                    << " instead of " << ID2STR(nID) << endl;
                    nID = newID;
                }
            }
            /************ End of hack for copy/paste code ************/

            pObject = m_doc->findObjectById(nID);
            if (pObject && label.isEmpty())
                label = pObject->getName();

        } else if (typeIsFolder(lvType) ||
                   lvType == Uml::lvt_Diagrams) {
            // Pre-1.2 format: Folders did not have their ID set.
            // Pull a new ID now.
            nID = m_doc->getUniqueID();
        } else if (type != "801" && type != "802" &&
                   type != "821" && type != "827" && type != "836") {
            kdError() << "UMLListView::loadChildrenFromXMI: item of type "
            << type << " has no ID, skipping." << endl;
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
        case Uml::lvt_Component:
        case Uml::lvt_Node:
        case Uml::lvt_Artifact:
            item = findItem(nID);
            if (item == NULL) {
                if (pObject && pObject->getUMLPackage() &&
                        parent->getType() != Uml::lvt_Package) {
                    // Pre-1.2 file format:
                    // Objects were not nested in their packages.
                    // Synthesize the nesting here.
                    UMLPackage *umlpkg = pObject->getUMLPackage();
                    UMLListViewItem *pkgItem = findUMLObject(umlpkg);
                    if (pkgItem == NULL) {
                        kdDebug() << "UMLListView::loadChildrenFromXMI: "
                        << "synthesizing ListViewItem for package "
                        << ID2STR(umlpkg->getID()) << endl;
                        pkgItem = new UMLListViewItem(parent, umlpkg->getName(),
                                                      Uml::lvt_Package, umlpkg);
                        pkgItem->setOpen(true);
                    }
                    item = new UMLListViewItem(pkgItem, label, lvType, pObject);
                } else {
                    item = new UMLListViewItem(parent, label, lvType, pObject);
                }
            }
            else if (parent != item->parent())
            { // The existing item was created by the slot event triggered
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
                /* kdDebug() << "UMLListView::loadChildrenFromXMI: Reparenting "
                	  << item->getText() << endl;
                 */
                UMLListViewItem *newItem = item->deepCopy(parent);
                delete item;
                item = newItem;
            }
            break;
        case Uml::lvt_Attribute:
        case Uml::lvt_Template:
        case Uml::lvt_Operation:
            item = findItem(nID);
            if (item == NULL) {
                kdDebug() << "UMLListView::loadChildrenFromXMI: "
                << "item " << ID2STR(nID) << " (of type "
                << lvType << ") does not yet exist..."
                << endl;
                UMLObject* umlObject = parent->getUMLObject();
                if (!umlObject)
                    return false;
                if (nID == Uml::id_None) {
                    kdWarning() << "UMLListView::loadChildrenFromXMI: lvtype " << lvType
                    << " has id -1" << endl;
                } else {
                    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(umlObject);
                    if (classifier) {
                        umlObject = classifier->findChildObject(nID);
                        if (umlObject) {
                            connectNewObjectsSlots(umlObject);
                            label = umlObject->getName();
                            item = new UMLListViewItem( parent, label, lvType, umlObject);
                        } else {
                            kdDebug() << "UMLListView::loadChildrenFromXMI: "
                            << " lvtype " << lvType << " child object "
                            << ID2STR(nID) << " not found" << endl;
                        }
                    } else {
                        kdDebug() << "UMLListView::loadChildrenFromXMI: "
                        << "cast to classifier object failed" << endl;
                    }
                }
            }
            break;
        case Uml::lvt_Logical_View:
            item = m_lv;
            break;
        case Uml::lvt_Datatype_Folder:
            item = datatypeFolder;
            break;
        case Uml::lvt_UseCase_View:
            item = m_ucv;
            break;
        case Uml::lvt_Component_View:
            item = componentView;
            break;
        case Uml::lvt_Deployment_View:
            item = deploymentView;
            break;
        case Uml::lvt_EntityRelationship_Model:
            item = entityRelationshipModel;
            break;
        case Uml::lvt_Diagrams:
            //Uncomment for using Luis diagram display code
            //				item = diagramFolder;
            break;
        default:
            {
                Settings::OptionState optionState = UMLApp::app()->getOptionState();
                if (!optionState.generalState.tabdiagrams ||
                        //don't load diagrams any more, tabbed diagrams
                        !typeIsDiagram(lvType) ) {
                    item = new UMLListViewItem( parent, label, lvType, nID );
                    if (UMLListView::typeIsFolder(lvType)) {
                        QString folderFile = domElement.attribute( "external_file", "" );
                        if (!folderFile.isEmpty()) {
                            setCurrentItem(item);
                            item->setFolderFile(folderFile);
                            const QString fqfn(m_doc->URL().directory(false) + folderFile);
                            kdDebug() << "UMLListView::loadChildrenFromXMI:"
                            << " fully qualified folderFile is " << fqfn
                            << endl;
                            m_doc->loadFolderFile(fqfn);
                        }
                    }
                }
            }
            break;
        }//end switch

        if (item)  {
            item->setOpen( (bool)bOpen );
            if ( !loadChildrenFromXMI(item, domElement) ) {
                return false;
            }
        } else {
            kdWarning() << "unused list view item " << ID2STR(nID)
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

bool UMLListView::typeIsCanvasWidget(Uml::ListView_Type type) {
    if (type == Uml::lvt_Actor ||
            type == Uml::lvt_UseCase ||
            type == Uml::lvt_Class ||
            type == Uml::lvt_Package ||
            type == Uml::lvt_Component ||
            type == Uml::lvt_Node ||
            type == Uml::lvt_Artifact ||
            type == Uml::lvt_Interface ||
            type == Uml::lvt_Datatype ||
            type == Uml::lvt_Enum ||
            type == Uml::lvt_Entity) {
        return true;
    } else {
        return false;
    }
}

bool UMLListView::typeIsFolder(Uml::ListView_Type type) {
    if (type == Uml::lvt_Logical_Folder ||
            type == Uml::lvt_UseCase_Folder ||
            type == Uml::lvt_Component_Folder ||
            type == Uml::lvt_Deployment_Folder ||
            type == Uml::lvt_EntityRelationship_Folder ||
            type == Uml::lvt_Datatype_Folder) {
        return true;
    } else {
        return false;
    }
}

bool UMLListView::typeIsClassifierList(Uml::ListView_Type type) {
    if (type == Uml::lvt_Attribute ||
            type == Uml::lvt_Operation ||
            type == Uml::lvt_Template) {
        return true;
    } else {
        return false;
    }
}

bool UMLListView::typeIsDiagram(Uml::ListView_Type type) {
    if (type == Uml::lvt_Class_Diagram ||
            type == Uml::lvt_Collaboration_Diagram ||
            type == Uml::lvt_State_Diagram ||
            type == Uml::lvt_Activity_Diagram ||
            type == Uml::lvt_Sequence_Diagram ||
            type == Uml::lvt_UseCase_Diagram ||
            type == Uml::lvt_Component_Diagram ||
            type == Uml::lvt_Deployment_Diagram ||
            type == Uml::lvt_EntityRelationship_Diagram) {
        return true;
    } else {
        return false;
    }
}

void UMLListView::deleteChildrenOf(QListViewItem* parent) {
    if ( !parent ) {
        return;
    }

    while ( parent->firstChild() ) {
        delete parent->firstChild();
    }
    if (parent == m_lv)  {
        datatypeFolder = new UMLListViewItem(m_lv, i18n("Datatypes"), Uml::lvt_Datatype_Folder);
    }
}

void UMLListView::closeDatatypesFolder()  {
    datatypeFolder->setOpen(false);
}

#include "umllistview.moc"
