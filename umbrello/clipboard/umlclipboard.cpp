/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "umlclipboard.h"

// local includes
#include "debug_utils.h"
#include "umldragdata.h"
#include "idchangelog.h"
#include "associationwidget.h"
#include "attribute.h"
#include "classifier.h"
#include "enum.h"
#include "entity.h"
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "operation.h"
#include "template.h"
#include "enumliteral.h"
#include "entityattribute.h"
#include "model_utils.h"
#include "notewidget.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlobjectlist.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"
#include "uml.h"

// kde includes
#include <KMessageBox>
#include <KLocalizedString>

// qt includes
#include <QMimeData>
#include <QPixmap>

/**
 * Constructor.
 */
UMLClipboard::UMLClipboard()
{
    m_type = clip1;
}

/**
 * Deconstructor.
 */
UMLClipboard::~UMLClipboard()
{
}

/**
 * Copy operation.
 * @param fromView   flag if it is from view
 * @return           the mime data
 */
QMimeData* UMLClipboard::copy(bool fromView/*=false*/)
{
    // Clear previous copied data
    m_AssociationList.clear();
    m_ObjectList.clear();
    m_ViewList.clear();

    UMLDragData *data = 0;
    QPixmap* png = 0;

    UMLListView * listView = UMLApp::app()->listView();

    if (fromView) {
        m_type = clip4;
        UMLView *view = UMLApp::app()->currentView();
        if (view == NULL) {
            uError() << "UMLApp::app()->currentView() is NULL";
            return NULL;
        }
        UMLScene *scene = view->umlScene();
        if (scene == NULL) {
            uError() << "currentView umlScene() is NULL";
            return NULL;
        }
        m_WidgetList = scene->selectedWidgetsExt();
        //if there is no selected widget then there is no copy action
        if (!m_WidgetList.count()) {
            return 0;
        }
        m_AssociationList = scene->selectedAssocs();
        scene->copyAsImage(png);

        // Clip4 needs related widgets.
        addRelatedWidgets();

        // Clip4 needs UMLObjects because it's possible the UMLObject
        // is no longer there when pasting this mime data. This happens for
        // example when using cut-paste or pasting to another Umbrello
        // instance.
        fillObjectListForWidgets(m_WidgetList);

        foreach (WidgetBase* widget, m_AssociationList) {
            if (widget->umlObject() != 0) {
                m_ObjectList.append(widget->umlObject());
            }
        }
    } else {
        // The copy action is being performed from the ListView
        UMLListViewItemList itemsSelected = listView->selectedItems();
        if (itemsSelected.count() <= 0) {
            return 0;
        }

        // Set What type of copy operation are we performing and
        // also fill m_ViewList with all the selected Diagrams
        setCopyType(itemsSelected);

        // If we are copying a diagram or part of a diagram, select the items
        // on the ListView that correspond to a UseCase, Actor or Concept
        // in the Diagram
        if (m_type == clip2) {
            foreach (UMLView* view, m_ViewList) {
                fillObjectListForWidgets(view->umlScene()->widgetList());

                AssociationWidgetList associations = view->umlScene()->associationList();
                foreach (AssociationWidget* association, associations) {
                    if (association->umlObject() != 0) {
                        m_ObjectList.append(association->umlObject());
                    }
                }
            }
        } else {
            // Clip1, 4 and 5: fill the clip with only the specific objects
            // selected in the list view
            if (!fillSelectionLists(itemsSelected)) {
                return 0;
            }

            if (itemsSelected.count() <= 0) {
                return 0;
            }
        }
    }

    int i = 0;
    switch(m_type) {
    case clip1:
        data = new UMLDragData(m_ObjectList);
        break;
    case clip2:
        data = new UMLDragData(m_ObjectList, m_ViewList);
        break;
    case clip3:
        data = new UMLDragData(m_ItemList);
        break;
    case clip4:
        if (png) {
            UMLView *view = UMLApp::app()->currentView();
            data = new UMLDragData(m_ObjectList, m_WidgetList,
                                   m_AssociationList, *png, view->umlScene());
        } else {
            return 0;
        }
        break;
    case clip5:
        data = new UMLDragData(m_ObjectList, i);
        // The int i is used to differentiate
        // which UMLDragData constructor gets called.
        break;
    }

    return (QMimeData*)data;
}

/**
 * Inserts the clipboard's contents.
 *
 * @param data   Pointer to the MIME format clipboard data.
 * @return       True for successful operation.
 */
bool UMLClipboard::paste(const QMimeData* data)
{
    UMLDoc *doc = UMLApp::app()->document();

    int codingType = UMLDragData::getCodingType(data);

    QString mimeType = QLatin1String("application/x-uml-clip") + QString::number(codingType);
    uDebug() << "Pasting mimeType=" << mimeType << "data=" << data->data(mimeType);

    bool result = false;
    doc->beginPaste();

    switch (codingType) {
    case 1:
        result = pasteClip1(data);
        break;
    case 2:
        result = pasteClip2(data);
        break;
    case 3:
        result = pasteClip3(data);
        break;
    case 4:
        result = pasteClip4(data);
        break;
    case 5:
        result = pasteClip5(data);
        break;
    default:
        break;
    }
    doc->endPaste();
    return result;
}

/**
 * Fills object list based on a selection of widgets
 *
 * @param UMLWidgetList& widgets
 */
void UMLClipboard::addRelatedWidgets()
{
    UMLWidgetList relatedWidgets;
    UMLWidget *pWA =0, *pWB = 0;

    foreach (UMLWidget* widget, m_WidgetList) {
        if (widget->baseType() == WidgetBase::wt_Message) {
            MessageWidget * pMessage = static_cast<MessageWidget *>(widget);
            pWA = (UMLWidget*)pMessage->objectWidget(Uml::RoleType::A);
            pWB = (UMLWidget*)pMessage->objectWidget(Uml::RoleType::B);
            if (!relatedWidgets.contains(pWA))
                relatedWidgets.append(pWA);
            if (!relatedWidgets.contains(pWB))
                relatedWidgets.append(pWB);
        }
    }

    foreach(AssociationWidget *pAssoc, m_AssociationList) {
        pWA = pAssoc->widgetForRole(Uml::RoleType::A);
        pWB = pAssoc->widgetForRole(Uml::RoleType::B);
        if (!relatedWidgets.contains(pWA))
            relatedWidgets.append(pWA);
        if (!relatedWidgets.contains(pWB))
            relatedWidgets.append(pWB);
    }

    foreach(UMLWidget *widget, relatedWidgets) {
        if (!m_WidgetList.contains(widget))
            m_WidgetList.append(widget);
    }
}

/**
 * Fills object list based on a selection of widgets
 *
 * @param UMLWidgetList& widgets
 */
void UMLClipboard::fillObjectListForWidgets(UMLWidgetList& widgets)
{
    // The order of the packages in the clip matters. So we collect
    // the packages and add them from the root package to the deeper levels
    UMLObjectList packages;

    foreach (UMLWidget* widget, widgets) {
        UMLObject* widgetObject = widget->umlObject();
        if (widgetObject != 0) {
            packages.clear();

            UMLPackage* package = widgetObject->umlPackage();
            while (package != 0) {
                packages.prepend(package);
                package = package->umlPackage();
            }

            foreach (UMLObject* package, packages) {
                if (!m_ObjectList.contains(package)) {
                    m_ObjectList.append(package);
                }
            }

            if (!m_ObjectList.contains(widgetObject)) {
                m_ObjectList.append(widgetObject);
            }
        }
    }
}

/**
 * Fills the member lists with all the objects and other
 * stuff to be copied to the clipboard.
 * @param selectedItems   list of selected items
 */
bool UMLClipboard::fillSelectionLists(UMLListViewItemList& selectedItems)
{
    UMLListViewItem::ListViewType type;
    switch(m_type) {
    case clip4:
        break;
    case clip3:
        foreach (UMLListViewItem* item, selectedItems) {
            type = item->type();
            if (!Model_Utils::typeIsClassifierList(type)) {
                m_ItemList.append(item);
                insertItemChildren(item, selectedItems);
            }
        }
        break;
    case clip2:
    case clip1:
        foreach (UMLListViewItem* item, selectedItems) {
            type = item->type();
            if (!Model_Utils::typeIsClassifierList(type)) {
                if (Model_Utils::typeIsCanvasWidget(type)) {
                    m_ObjectList.append(item->umlObject());
                }
                insertItemChildren(item, selectedItems);
            }
        }
        break;
    case clip5:
        foreach (UMLListViewItem* item, selectedItems) {
            type = item->type();
            if(Model_Utils::typeIsClassifierList(type)) {
                m_ObjectList.append(item->umlObject());
            } else {
                return false;
            }
        }
        break;
    }

    return true;
}

/**
 * Checks the whole list to determine the copy action
 * type to be performed, sets the type in the m_type
 * member variable.
 * @param selectedItems   list of selected items
 */
void UMLClipboard::setCopyType(UMLListViewItemList& selectedItems)
{
    bool withDiagrams = false; //If the selection includes diagrams
    bool withObjects = false; //If the selection includes objects
    bool onlyAttsOps = false; //If the selection only includes Attributes and/or Operations

    foreach (UMLListViewItem* item, selectedItems) {
        checkItemForCopyType(item, withDiagrams, withObjects, onlyAttsOps);
    }
    if (onlyAttsOps) {
        m_type = clip5;
    } else if (withDiagrams) {
        m_type = clip2;
    } else if(withObjects) {
        m_type = clip1;
    } else {
        m_type = clip3;
    }
}

/**
 * Searches the child items of a UMLListViewItem to
 * establish which Copy type is to be perfomed.
 * @param item          parent of the children
 * @param withDiagrams  includes diagrams
 * @param withObjects   includes objects
 * @param onlyAttsOps   includes only attributes and/or operations
 */
void UMLClipboard::checkItemForCopyType(UMLListViewItem* item, bool & withDiagrams, bool & withObjects,
                                        bool & onlyAttsOps)
{
    if(!item) {
        return;
    }
    UMLDoc *doc = UMLApp::app()->document();
    onlyAttsOps = true;
    UMLView * view = 0;
    UMLListViewItem * child = 0;
    UMLListViewItem::ListViewType type = item->type();
    if (Model_Utils::typeIsCanvasWidget(type)) {
        withObjects = true;
        onlyAttsOps = false;
    } else if (Model_Utils::typeIsDiagram(type)) {
        withDiagrams = true;
        onlyAttsOps = false;
        view = doc->findView(item->ID());
        if (view)
            m_ViewList.append(view);
        else
            uError() << "doc->findView(" << Uml::ID::toString(item->ID()) << ") returns NULL";
    } else if (Model_Utils::typeIsFolder(type)) {
        onlyAttsOps = false;
        for (int i =0; i < item->childCount(); i++) {
            child = (UMLListViewItem*)item->child(i);
            checkItemForCopyType(child, withDiagrams, withObjects, onlyAttsOps);
        }
    }
}

/**
 * Traverse children of a UMLListViewItem and add its UMLObjects to the list
 *
 * @param item            parent of the children to insert
 * @param selectedItems   list of selected items
 * @return                success flag
 */
bool UMLClipboard::insertItemChildren(UMLListViewItem * item, UMLListViewItemList& selectedItems)
{
    if (item->childCount()) {
        for(int i = 0; i < item->childCount(); i++) {
            UMLListViewItem * child = (UMLListViewItem*)item->child(i);
            m_ItemList.append(child);
            UMLListViewItem::ListViewType type = child->type();
            if (!Model_Utils::typeIsClassifierList(type) &&
                !Model_Utils::typeIsDiagram(type)) {
                m_ObjectList.append(child->umlObject());
            }
            // If the child is selected, remove it from the list of selected items
            // otherwise it will be inserted twice in m_ObjectList.
            if (child->isSelected()) {
                selectedItems.removeAll(child);
            }
            insertItemChildren(child, selectedItems);
        }
    }
    return true;
}

/**
 * If clipboard has mime type application/x-uml-clip1,
 * Pastes the data from the clipboard into the current Doc.
 * @param data   mime type
 */
bool UMLClipboard::pasteClip1(const QMimeData* data)
{
    UMLObjectList objects;
    return UMLDragData::decodeClip1(data, objects);
}

/**
 * If clipboard has mime type application/x-uml-clip2,
 * Pastes the data from the clipboard into the current Doc.
 * @param data   mime type
 * @return       success flag
 */
bool UMLClipboard::pasteClip2(const QMimeData* data)
{
    UMLDoc*             doc = UMLApp::app()->document();
    UMLObjectList       objects;
    UMLViewList         views;

    if (!UMLDragData::decodeClip2(data, objects, views)) {
        uDebug() << "UMLDragData::decodeClip2 returned error";
        return false;
    }

    if (NoteWidget::s_pCurrentNote) {
        NoteWidget::s_pCurrentNote = NULL;
    } else {
        foreach (UMLView* pView, views) {
            if (!doc->addUMLView(pView)) {
                return false;
            }
        }
    }

    return true;
}

/**
 * If clipboard has mime type application/x-uml-clip3,
 * Pastes the data from the clipboard into the current Doc.
 *
 * Note: clip3 is only used to determine if the selected items can be dragged
 * onto the view. Pasting only listview items makes no sense. Clip3 is implemented
 * as a fallback-clip when clip 1, 2, 4 or 5 are not applicable. But that should
 * never happen.
 *
 * Todo: remove clip3 alltogether.
 *
 * @param data   mime type
 * @return       success flag
 */
bool UMLClipboard::pasteClip3(const QMimeData* data)
{
    UMLDoc *doc = UMLApp::app()->document();
    UMLListViewItemList itemdatalist;
    IDChangeLog* idchanges = doc->changeLog();

    if(!idchanges) {
        return false;
    }

    UMLListView *listView = UMLApp::app()->listView();
    return UMLDragData::decodeClip3(data, itemdatalist, listView);
}

/**
 * If clipboard has mime type application/x-uml-clip4,
 * Pastes the data from the clipboard into the current Doc.
 * @param data   mime type
 * @return       success flag
 */
bool UMLClipboard::pasteClip4(const QMimeData* data)
{
    UMLDoc *doc = UMLApp::app()->document();

    UMLObjectList objects;
    UMLWidgetList widgets;
    AssociationWidgetList assocs;

    IDChangeLog* idchanges = 0;

    Uml::DiagramType::Enum diagramType;

    if(!UMLDragData::decodeClip4(data, objects, widgets, assocs, diagramType)) {
        return false;
    }

    UMLScene *currentScene = UMLApp::app()->currentView()->umlScene();

    idchanges = doc->changeLog();
    if(!idchanges) {
        return false;
    }
     //make sure the file we are pasting into has the objects
     //we need if there are widgets to be pasted
    foreach (UMLObject* obj, objects) {
        if(!doc->assignNewIDs(obj)) {
            return false;
        }

     }

    //now add any widget we are want to paste
    bool objectAlreadyExists = false;
    currentScene->beginPartialWidgetPaste();

    foreach (UMLWidget* widget, widgets) {

        Uml::ID::Type oldId = widget->id();
        Uml::ID::Type newId = idchanges->findNewID(oldId);
        // how should findWidget find ::None id, which is returned for the first entry ?
        if (currentScene->findWidget(newId)) {
            uError() << "widget (oldID=" << Uml::ID::toString(oldId) << ", newID="
                << Uml::ID::toString(newId) << ") already exists in target view.";
            widgets.removeAll(widget);
            delete widget;
            objectAlreadyExists = true;
        } else {
            if (currentScene->type() == Uml::DiagramType::Activity ||
                currentScene->type() == Uml::DiagramType::State) {
                widget->setID(doc->assignNewID(widget->id()));
            }
        }
    }

    //now paste the associations
    foreach (AssociationWidget* assoc, assocs) {
        if (!currentScene->addAssociation(assoc, true)) {
            currentScene->endPartialWidgetPaste();
            return false;
        }
    }

    currentScene->clearSelected();
    currentScene->selectWidgets(widgets);
    foreach (AssociationWidget* assoc, assocs) {
        currentScene->selectWidgetsOfAssoc(assoc);
    }

    //Activate all the pasted associations and widgets
    currentScene->activate();
    currentScene->endPartialWidgetPaste();

    if (objectAlreadyExists) {
        pasteItemAlreadyExists();
    }
    return true;
}

/**
 * If clipboard has mime type application/x-uml-clip5,
 * Pastes the data from the clipboard into the current Doc.
 * @param data   mime type
 * @return       success flag
 */
bool UMLClipboard::pasteClip5(const QMimeData* data)
{
    UMLDoc *doc = UMLApp::app()->document();
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem* lvitem = dynamic_cast<UMLListViewItem *>(listView->currentItem());
    if (!lvitem || !Model_Utils::typeIsClassifier(lvitem->type())) {
        return false;
    }
    UMLClassifier *parent = dynamic_cast<UMLClassifier*>(lvitem->umlObject());

    if (parent == NULL) {
        uError() << "parent is not a UMLClassifier";
        return false;
    }

    UMLObjectList objects;

    IDChangeLog* idchanges = 0;
    bool result = UMLDragData::decodeClip5(data, objects, parent);

    if(!result) {
        return false;
    }

    doc->setModified(true);
    idchanges = doc->changeLog();
    // Assume success if at least one child object could be pasted
    if (objects.count())
        result = false;

    foreach (UMLObject* obj, objects) {
        obj->setID(doc->assignNewID(obj->id()));
        switch(obj->baseType()) {
        case UMLObject::ot_Attribute :
            {
                UMLObject *exist = parent->findChildObject(obj->name(), UMLObject::ot_Attribute);
                if (exist) {
                    QString newName = parent->uniqChildName(UMLObject::ot_Attribute, obj->name());
                    obj->setName(newName);
                }
                UMLAttribute *att = static_cast<UMLAttribute*>(obj);
                if (parent->addAttribute(att, idchanges)) {
                    result = true;
                } else {
                    uError() << parent->name() << "->addAttribute("
                             << att->name() << ") failed";
                }
                break;
            }
        case UMLObject::ot_Operation :
            {
                UMLOperation *op = static_cast<UMLOperation*>(obj);
                UMLOperation *exist = parent->checkOperationSignature(op->name(), op->getParmList());
                if (exist) {
                    QString newName = parent->uniqChildName(UMLObject::ot_Operation, obj->name());
                    op->setName(newName);
                }
                if (parent->addOperation(op, idchanges)) {
                    result = true;
                } else {
                    uError() << parent->name() << "->addOperation("
                             << op->name() << ") failed";
                }
                break;
            }
        case UMLObject::ot_Template:
            {
                UMLTemplate* tp = static_cast<UMLTemplate*>(obj);
                UMLTemplate* exist = parent->findTemplate(tp->name());
                if (exist) {
                    QString newName = parent->uniqChildName(UMLObject::ot_Template, obj->name());
                    tp->setName(newName);
                }
                if (parent->addTemplate(tp, idchanges)) {
                    result = true;
                } else {
                    uError() << parent->name() << "->addTemplate("
                             << tp->name() << ") failed";
                }
                break;
            }
        case UMLObject::ot_EnumLiteral:
           {
               UMLEnum* enumParent = dynamic_cast<UMLEnum*>(parent);
               // if parent is not a UMLEnum, bail out immediately;
               if (!enumParent) {
                   result = false;
                   uError() << "Parent is not an UMLEnum";
                   break;
               }

               UMLObject* exist = enumParent->findChildObject(obj->name(), UMLObject::ot_EnumLiteral);
               if (exist) {
                   QString newName = enumParent->uniqChildName(UMLObject::ot_EnumLiteral, obj->name());
                   obj->setName(newName);
               }
               UMLEnumLiteral* enl = static_cast<UMLEnumLiteral*>(obj);

               if (enumParent->addEnumLiteral(enl, idchanges)) {
                   result = true;
               } else {
                   uError() << enumParent->name() << "->addEnumLiteral("
                            << enl->name() << ") failed";
               }
               break;
           }
        case UMLObject::ot_EntityAttribute :
            {
                UMLEntity* entityParent = dynamic_cast<UMLEntity*>(parent);
                // if parent is not a UMLEntity, bail out immediately;
                if (!entityParent) {
                    result = false;
                    uError() << "Parent is not an UMLEntity";
                    break;
                }
                UMLObject *exist = entityParent->findChildObject(obj->name(), UMLObject::ot_EntityAttribute);
                if (exist) {
                    QString newName = entityParent->uniqChildName(UMLObject::ot_EntityAttribute, obj->name());
                    obj->setName(newName);
                }
                UMLEntityAttribute *att = static_cast<UMLEntityAttribute*>(obj);

                if (entityParent->addEntityAttribute(att, idchanges)) {
                    result = true;
                } else {
                    uError() << parent->name() << "->addEntityAttribute(" << att->name() << ") failed";
                }
                break;
            }
        default :
            uWarning() << "pasting unknown children type in clip type 5";
            return false;
        }
    }

    return result;
}

/**
 * Gives a `sorry' message box if you're pasting an item which
 * already exists and can't be duplicated.
 */
void UMLClipboard::pasteItemAlreadyExists()
{
    UMLView *currentView = UMLApp::app()->currentView();
    KMessageBox::sorry(currentView,
                        i18n("At least one of the items in the clipboard "
                             "could not be pasted because an item of the "
                             "same name already exists.  Any other items "
                             "have been pasted."),
                        i18n("Paste Error"));
}

