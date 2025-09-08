/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlclipboard.h"

// local includes
#include "debug_utils.h"
#include "diagram_utils.h"
#include "umldragdata.h"
#include "idchangelog.h"
#include "associationwidget.h"
#include "umlattribute.h"
#include "umlclassifier.h"
#include "umlenum.h"
#include "umlentity.h"
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "umloperation.h"
#include "umltemplate.h"
#include "umlenumliteral.h"
#include "umlentityattribute.h"
#include "model_utils.h"
#include "notewidget.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlobjectlist.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlapp.h"

// kde includes
#include <KMessageBox>
#include <KLocalizedString>

// qt includes
#include <QMimeData>
#include <QPixmap>

DEBUG_REGISTER(UMLClipboard)

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

    UMLDragData  *data = nullptr;
    QPixmap *png = nullptr;

    UMLListView * listView = UMLApp::app()->listView();

    if (fromView) {
        m_type = clip4;
        UMLView *view = UMLApp::app()->currentView();
        if (view == nullptr) {
            logError0("UMLApp::app()->currentView() is NULL");
            return nullptr;
        }
        UMLScene *scene = view->umlScene();
        if (scene == nullptr) {
            logError0("UMLClipboard::copy: currentView umlScene() is NULL");
            return nullptr;
        }
        m_WidgetList = scene->selectedWidgetsExt();
        //if there is no selected widget then there is no copy action
        if (!m_WidgetList.count()) {
            return nullptr;
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

        for(WidgetBase *widget : m_AssociationList) {
            if (widget->umlObject() != nullptr) {
                m_ObjectList.append(widget->umlObject());
            }
        }
    } else {
        // The copy action is being performed from the ListView
        UMLListViewItemList itemsSelected = listView->selectedItems();
        if (itemsSelected.count() <= 0) {
            return nullptr;
        }

        // Set What type of copy operation are we performing and
        // also fill m_ViewList with all the selected Diagrams
        setCopyType(itemsSelected);

        // If we are copying a diagram or part of a diagram, select the items
        // on the ListView that correspond to a UseCase, Actor or Concept
        // in the Diagram
        if (m_type == clip2) {
            for(UMLView* view : m_ViewList) {
                UMLScene *scene = view->umlScene();
                if (scene == nullptr) {
                    logError0("UMLClipboard::copy: currentView umlScene() is NULL");
                    continue;
                }
                fillObjectListForWidgets(scene->widgetList());

                AssociationWidgetList associations = scene->associationList();
                for(AssociationWidget* association : associations) {
                    if (association->umlObject() != nullptr) {
                        m_ObjectList.append(association->umlObject());
                    }
                }
            }
        } else {
            // Clip1, 4 and 5: fill the clip with only the specific objects
            // selected in the list view
            if (!fillSelectionLists(itemsSelected)) {
                return nullptr;
            }

            if (itemsSelected.count() <= 0) {
                return nullptr;
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
            return nullptr;
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
    if (codingType == 6
            && UMLApp::app()->currentView()) {
        return Diagram_Utils::importGraph(data, UMLApp::app()->currentView()->umlScene());
    }
    QString mimeType = QStringLiteral("application/x-uml-clip") + QString::number(codingType);
    logDebug1("UMLClipboard::paste: Pasting mimeType %1", mimeType);

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
    UMLWidget   *pWA = nullptr, *pWB = nullptr;

    for(UMLWidget* widget : m_WidgetList) {
        if (widget->isMessageWidget()) {
            MessageWidget * pMessage = widget->asMessageWidget();
            pWA = (UMLWidget*)pMessage->objectWidget(Uml::RoleType::A);
            pWB = (UMLWidget*)pMessage->objectWidget(Uml::RoleType::B);
            if (!relatedWidgets.contains(pWA))
                relatedWidgets.append(pWA);
            if (!relatedWidgets.contains(pWB))
                relatedWidgets.append(pWB);
        }
    }

    for(AssociationWidget *pAssoc : m_AssociationList) {
        pWA = pAssoc->widgetForRole(Uml::RoleType::A);
        pWB = pAssoc->widgetForRole(Uml::RoleType::B);
        if (!relatedWidgets.contains(pWA))
            relatedWidgets.append(pWA);
        if (!relatedWidgets.contains(pWB))
            relatedWidgets.append(pWB);
    }

    for(UMLWidget *widget : relatedWidgets) {
        if (!m_WidgetList.contains(widget))
            m_WidgetList.append(widget);
    }
}

/**
 * Fills object list based on a selection of widgets
 *
 * @param widgets  the UMLWidgetList to fill
 */
void UMLClipboard::fillObjectListForWidgets(const UMLWidgetList& widgets)
{
    // The order of the packages in the clip matters. So we collect
    // the packages and add them from the root package to the deeper levels
    UMLObjectList packages;

    for(UMLWidget* widget : widgets) {
        UMLObject* widgetObject = widget->umlObject();
        if (widgetObject != nullptr) {
            packages.clear();

            UMLPackage* package = widgetObject->umlPackage();
            while (package != nullptr) {
                packages.prepend(package);
                package = package->umlPackage();
            }

            for(UMLObject *package : packages) {
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
        for(UMLListViewItem *item : selectedItems) {
            type = item->type();
            if (!Model_Utils::typeIsClassifierList(type)) {
                m_ItemList.append(item);
                insertItemChildren(item, selectedItems);
            }
        }
        break;
    case clip2:
    case clip1:
        for(UMLListViewItem *item : selectedItems) {
            type = item->type();
            if (!Model_Utils::typeIsClassifierList(type)) {
                if (Model_Utils::typeIsCanvasWidget(type)) {
                    if (item->umlObject() == nullptr)
                        logError0("UMLClipboard::fillSelectionLists: selected lvitem has no umlObject");
                    else
                        m_ObjectList.append(item->umlObject());
                }
                insertItemChildren(item, selectedItems);
            }
        }
        break;
    case clip5:
        for(UMLListViewItem *item : selectedItems) {
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

    for(UMLListViewItem *item : selectedItems) {
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
 * establish which Copy type is to be performed.
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
    UMLView  *view = nullptr;
    UMLListViewItem  *child = nullptr;
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
            logError1("UMLClipboard::checkItemForCopyType: doc->findView(%1) returns NULL",
                      Uml::ID::toString(item->ID()));
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
        logDebug0("UMLClipboard::pasteClip2: UMLDragData::decodeClip2 returned error");
        return false;
    }

    if (NoteWidget::s_pCurrentNote) {
        NoteWidget::s_pCurrentNote = nullptr;
    } else {
        for(UMLView *pView : views) {
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
 * Todo: remove clip3 altogether.
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

    IDChangeLog *idchanges = nullptr;

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
    for(UMLObject *obj : objects) {
        uIgnoreZeroPointer(obj);
        if(!doc->assignNewIDs(obj)) {
            return false;
        }

     }

    //now add any widget we are want to paste
    bool objectAlreadyExists = false;
    currentScene->beginPartialWidgetPaste();

    for(UMLWidget *widget : widgets) {

        Uml::ID::Type oldId = widget->id();
        Uml::ID::Type newId = idchanges->findNewID(oldId);
        // how should findWidget find ::None id, which is returned for the first entry ?
        if (currentScene->findWidget(newId)) {
            logError2("UMLClipboard::pasteClip4 widget (oldID=%1, newID=%2) already exists in target view",
                      Uml::ID::toString(oldId), Uml::ID::toString(newId));
            widgets.removeAll(widget);
            delete widget;
            objectAlreadyExists = true;
        } else {
            if (currentScene->isActivityDiagram() || currentScene->isStateDiagram()) {
                widget->setID(doc->assignNewID(widget->id()));
            }
        }
    }

    //now paste the associations
    for(AssociationWidget *assoc : assocs) {
        if (!currentScene->addAssociation(assoc, true)) {
            currentScene->endPartialWidgetPaste();
            return false;
        }
    }

    currentScene->clearSelected();
    currentScene->selectWidgets(widgets);
    for(AssociationWidget *assoc : assocs) {
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
    UMLClassifier *parent = lvitem->umlObject()->asUMLClassifier();

    if (parent == nullptr) {
        logError0("UMLClipboard::pasteClip5: parent is not a UMLClassifier");
        return false;
    }

    UMLObjectList objects;

    IDChangeLog *idchanges = nullptr;
    bool result = UMLDragData::decodeClip5(data, objects, parent);

    if(!result) {
        return false;
    }

    doc->setModified(true);
    idchanges = doc->changeLog();
    // Assume success if at least one child object could be pasted
    if (objects.count())
        result = false;

    for(UMLObject *obj : objects) {
        uIgnoreZeroPointer(obj);
        obj->setID(doc->assignNewID(obj->id()));
        switch(obj->baseType()) {
        case UMLObject::ot_Attribute :
            {
                UMLObject *exist = parent->findChildObject(obj->name(), UMLObject::ot_Attribute);
                if (exist) {
                    QString newName = parent->uniqChildName(UMLObject::ot_Attribute, obj->name());
                    obj->setName(newName);
                }
                UMLAttribute *att = obj->asUMLAttribute();
                if (parent->addAttribute(att, idchanges)) {
                    result = true;
                } else {
                    logError2("UMLClipboard::pasteClip5 %1->addAttribute(%2) failed",
                              parent->name(), att->name());
                }
                break;
            }
        case UMLObject::ot_Operation :
            {
                UMLOperation *op = obj->asUMLOperation();
                UMLOperation *exist = parent->checkOperationSignature(op->name(), op->getParmList());
                if (exist) {
                    QString newName = parent->uniqChildName(UMLObject::ot_Operation, obj->name());
                    op->setName(newName);
                }
                if (parent->addOperation(op, idchanges)) {
                    result = true;
                } else {
                    logError2("UMLClipboard::pasteClip5 %1->addOperation(%2) failed",
                              parent->name(), op->name());
                }
                break;
            }
        case UMLObject::ot_Template:
            {
                UMLTemplate* tp = obj->asUMLTemplate();
                UMLTemplate* exist = parent->findTemplate(tp->name());
                if (exist) {
                    QString newName = parent->uniqChildName(UMLObject::ot_Template, obj->name());
                    tp->setName(newName);
                }
                if (parent->addTemplate(tp, idchanges)) {
                    result = true;
                } else {
                    logError2("UMLClipboard::pasteClip5 %1->addTemplate(%2) failed",
                              parent->name(), tp->name());
                }
                break;
            }
        case UMLObject::ot_EnumLiteral:
           {
               UMLEnum* enumParent = parent->asUMLEnum();
               // if parent is not a UMLEnum, bail out immediately;
               if (!enumParent) {
                   result = false;
                   logError0("UMLClipboard::pasteClip5: Parent is not a UMLEnum");
                   break;
               }

               UMLObject* exist = enumParent->findChildObject(obj->name(), UMLObject::ot_EnumLiteral);
               if (exist) {
                   QString newName = enumParent->uniqChildName(UMLObject::ot_EnumLiteral, obj->name());
                   obj->setName(newName);
               }
               UMLEnumLiteral* enl = obj->asUMLEnumLiteral();

               if (enumParent->addEnumLiteral(enl, idchanges)) {
                   result = true;
               } else {
                   logError2("UMLClipboard::pasteClip5 %1->addEnumLiteral(%2) failed",
                              parent->name(), enl->name());
               }
               break;
           }
        case UMLObject::ot_EntityAttribute :
            {
                UMLEntity* entityParent = parent->asUMLEntity();
                // if parent is not a UMLEntity, bail out immediately;
                if (!entityParent) {
                    result = false;
                    logError0("UMLClipboard::pasteClip5: Parent is not a UMLEntity");
                    break;
                }
                UMLObject *exist = entityParent->findChildObject(obj->name(), UMLObject::ot_EntityAttribute);
                if (exist) {
                    QString newName = entityParent->uniqChildName(UMLObject::ot_EntityAttribute, obj->name());
                    obj->setName(newName);
                }
                UMLEntityAttribute *att = obj->asUMLEntityAttribute();

                if (entityParent->addEntityAttribute(att, idchanges)) {
                    result = true;
                } else {
                   logError2("UMLClipboard::pasteClip5 %1->addEntityAttribute(%2) failed",
                              parent->name(), att->name());
                }
                break;
            }
        default :
            logWarn0("pasting unknown children type in clip type 5");
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
    KMessageBox::information(currentView,
                        i18n("At least one of the items in the clipboard "
                             "could not be pasted because an item of the "
                             "same name already exists.  Any other items "
                             "have been pasted."),
                        i18n("Paste Error"));
}

