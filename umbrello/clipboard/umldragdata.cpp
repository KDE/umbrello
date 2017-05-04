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
#include "umldragdata.h"

// local includes
#include "associationwidget.h"
#include "classifier.h"
#include "cmds.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "folder.h"
#include "idchangelog.h"
#include "model_utils.h"
#include "object_factory.h"
#include "objectwidget.h"
#include "notewidget.h"
#include "preconditionwidget.h"
#include "messagewidget.h"
#include "uniqueid.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlobject.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"

// qt includes
#include <QDomDocument>
#include <QPixmap>
#include <QTextStream>

/**
 *  Constructor.
 */
UMLDragData::UMLDragData(UMLObjectList& objects, QWidget* dragSource /* = 0 */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip1(objects);
}

/**
 * For use when the user selects UML Object and Diagrams
 * from the ListView to be copied, Mime type =
 * "application/x-uml-clip2
 */
UMLDragData::UMLDragData(UMLObjectList& objects, UMLViewList& diagrams, QWidget* dragSource /* = 0 */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip2(objects, diagrams);
}

/**
 *  Constructor.
 */
UMLDragData::UMLDragData(UMLListViewItemList& umlListViewItems,
                         QWidget* dragSource /* = 0 */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip3(umlListViewItems);
}

/**
 * For use when the user selects UMLObjects from a
 * Diagram. The Selected widgets and the relationships
 * between only selected widgets will be copied and also
 * its respective ListView Items, Mime type =
 * "application/x-uml-clip4
 */
UMLDragData::UMLDragData(UMLObjectList& objects,
                         UMLWidgetList& widgets, AssociationWidgetList& associationDatas,
                         QPixmap& pngImage, UMLScene* scene, QWidget* dragSource /* = 0 */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip4(objects, widgets, associationDatas, pngImage, scene);
}

/**
 * For use when the user selects only Operations and/or
 * Attributes from the ListView, Mime type =
 * "application/x-uml-clip5
 */
UMLDragData::UMLDragData(UMLObjectList& objects, int,
                         QWidget* dragSource /* = 0 */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip5(objects);
}

/**
 *  Constructor.
 */
UMLDragData::UMLDragData(QWidget* dragSource /* = 0 */)
{
    Q_UNUSED(dragSource);
}

/**
 *  Deconstructor.
 */
UMLDragData::~UMLDragData()
{
}

/**
 * For use when the user selects only UMLObjects from the
 * ListView but no diagrams to be copied
 */
void UMLDragData::setUMLDataClip1(UMLObjectList& objects)
{
    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement(QLatin1String("xmiclip"));
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement(QLatin1String("umlobjects"));
    xmiclip.appendChild(objectsTag);

    UMLObjectListIt object_it(objects);
    UMLObject* obj = 0;
    while (object_it.hasNext()) {
        obj = object_it.next();
        obj->saveToXMI1(domDoc, objectsTag);
    }

    setData(QLatin1String("application/x-uml-clip1"), domDoc.toString().toUtf8());
}

/**
 * For use when the user selects UML Object and Diagrams
 * from the ListView to be copied
 */
void UMLDragData::setUMLDataClip2(UMLObjectList& objects, UMLViewList& diagrams)
{
    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement(QLatin1String("xmiclip"));
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement(QLatin1String("umlobjects"));
    xmiclip.appendChild(objectsTag);

    UMLObjectListIt object_it(objects);
    UMLObject* obj = 0;
    while (object_it.hasNext()) {
        obj = object_it.next();
        obj->saveToXMI1(domDoc, objectsTag);
    }

    QDomElement viewsTag = domDoc.createElement(QLatin1String("umlviews"));
    xmiclip.appendChild(viewsTag);

    foreach(UMLView* view, diagrams) {
        view->umlScene()->saveToXMI1(domDoc, viewsTag);
    }

    setData(QLatin1String("application/x-uml-clip2"), domDoc.toString().toUtf8());
}

/**
 * For use when the user selects only empty folders from the ListView
 * to be copied.
 */
void UMLDragData::setUMLDataClip3(UMLListViewItemList& umlListViewItems)
{
    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement(QLatin1String("xmiclip"));
    domDoc.appendChild(xmiclip);

    QDomElement itemsTag = domDoc.createElement(QLatin1String("umllistviewitems"));
    xmiclip.appendChild(itemsTag);

    foreach(UMLListViewItem* item, umlListViewItems) {
        item->saveToXMI1(domDoc, itemsTag);
    }

    setData(QLatin1String("application/x-uml-clip3"), domDoc.toString().toUtf8());
}

/**
 * For use when the user selects UML Objects from a
 * Diagram. The Selected widgets and the relationships
 * between only selected widgets will be copied and also
 * its respective ListView Items
 */
void UMLDragData::setUMLDataClip4(UMLObjectList& objects, UMLWidgetList& widgets, AssociationWidgetList& associations,
                                  QPixmap& pngImage, UMLScene *scene)
{
    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement(QLatin1String("xmiclip"));
    xmiclip.setAttribute(QLatin1String("diagramtype"), scene->type());
    xmiclip.setAttribute(QLatin1String("diagramid"), Uml::ID::toString(scene->ID()));
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement(QLatin1String("umlobjects"));
    xmiclip.appendChild(objectsTag);

    foreach (UMLObject* obj, objects) {
        obj->saveToXMI1(domDoc, objectsTag);
    }

    QDomElement widgetsTag = domDoc.createElement(QLatin1String("widgets"));
    xmiclip.appendChild(widgetsTag);

    foreach (UMLWidget* widget, widgets) {
        widget->saveToXMI1(domDoc, widgetsTag);
    }

    QDomElement associationWidgetsTag = domDoc.createElement(QLatin1String("associations"));
    xmiclip.appendChild(associationWidgetsTag);

    foreach (AssociationWidget* association, associations) {
        association->saveToXMI1(domDoc, associationWidgetsTag);
    }

    setData(QLatin1String("application/x-uml-clip4"), domDoc.toString().toUtf8());

    QImage img = pngImage.toImage();
    int l_size = img.byteCount();
    QByteArray clipdata;
    clipdata.resize(l_size);
    QDataStream clipstream(&clipdata, QIODevice::WriteOnly);
    clipstream << img;

    setImageData(clipdata);
}

/**
 * For use when the user selects only Attirbutes and/or
 * Operation from the ListView
 */
void UMLDragData::setUMLDataClip5(UMLObjectList& objects)
{
    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement(QLatin1String("xmiclip"));
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement(QLatin1String("umlobjects"));
    xmiclip.appendChild(objectsTag);

    foreach (UMLObject* obj, objects) {
        obj->saveToXMI1(domDoc, objectsTag);
    }

    setData(QLatin1String("application/x-uml-clip5"), domDoc.toString().toUtf8());
}

/**
 * For use when the user selects only UML Objects
 * from the ListView but no diagrams to be
 * copied, decodes Mime type =
 * "application/x-uml-clip1
 */
bool UMLDragData::decodeClip1(const QMimeData* mimeData, UMLObjectList& objects)
{
    if (!mimeData->hasFormat(QLatin1String("application/x-uml-clip1"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QLatin1String("application/x-uml-clip1"));
    if (!payload.size()) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        uWarning() << "Cannot set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QLatin1String("xmiclip")) {
        return false;
    }

    QDomNode objectsNode = xmiClipNode.firstChild();
    if (!UMLDragData::decodeObjects(objectsNode, objects, false)) {
        return false;
    }

    return true;
}

/**
 * For use when the user selects UML Object and Diagrams
 * from the ListView to be copied, decodes Mime type =
 * "application/x-uml-clip2
 */
bool UMLDragData::decodeClip2(const QMimeData* mimeData, UMLObjectList& objects, UMLViewList& diagrams)
{
    if (!mimeData->hasFormat(QLatin1String("application/x-uml-clip2"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QLatin1String("application/x-uml-clip2"));
    if (!payload.size()) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        uWarning() << "Cannot set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QLatin1String("xmiclip")) {
        return false;
    }

    // Load UMLObjects
    QDomNode objectsNode = xmiClipNode.firstChild();
    if (NoteWidget::s_pCurrentNote == 0) {
        if (!UMLDragData::decodeObjects(objectsNode, objects, true)) {
            return false;
        }
    }

    // Load UMLViews (diagrams)
    QDomNode umlviewsNode = objectsNode.nextSibling();
    if (!UMLDragData::decodeViews(umlviewsNode, diagrams)) {
        return false;
    }

    return true;
}

/**
 * Return just the LvTypeAndID of a Clip3.
 *
 * @param mimeData        The encoded source.
 * @param typeAndIdList   The LvTypeAndID_List decoded from the source.
 * @return  True if decoding was successful.
 */
bool UMLDragData::getClip3TypeAndID(const QMimeData* mimeData,
                                LvTypeAndID_List& typeAndIdList)
{
    if (!mimeData->hasFormat(QLatin1String("application/x-uml-clip3"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QLatin1String("application/x-uml-clip3"));
    if (!payload.size()) {
        return false;
    }
    QTextStream clipdata(payload, QIODevice::ReadOnly);
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        uWarning() << "getClip3Type: Cannot set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QLatin1String("xmiclip")) {
        return false;
    }

    QDomNode listItemNode = xmiClipNode.firstChild();
    QDomNode listItems = listItemNode.firstChild();
    QDomElement listItemElement = listItems.toElement();
    if (listItemElement.isNull()) {
        uWarning() << "getClip3Type: No listitems in XMI clip.";
        return false;
    }
    while (!listItemElement.isNull()) {
        QString typeStr = listItemElement.attribute(QLatin1String("type"), QLatin1String("-1"));
        if (typeStr == QLatin1String("-1")) {
            uDebug() << "getClip3Type: bad type.";
            return false;
        }
        QString idStr = listItemElement.attribute(QLatin1String("id"), QLatin1String("-1"));
        if (idStr == QLatin1String("-1")) {
            uDebug() << "getClip3Type: bad id";
            return false;
        }
        LvTypeAndID * pData = new LvTypeAndID;
        pData->type = (UMLListViewItem::ListViewType)(typeStr.toInt());
        pData->id = Uml::ID::fromString(idStr);
        typeAndIdList.append(pData);
        listItems = listItems.nextSibling();
        listItemElement = listItems.toElement();
    }
    return true;
}
 /**
  * For use when the user selects UMLObjects from
  * the ListView to be copied, decodes Mime * type =
  * "application/x-uml-clip3
  */
bool UMLDragData::decodeClip3(const QMimeData* mimeData, UMLListViewItemList& umlListViewItems,
                            const UMLListView* parentListView)
{
    if (!mimeData->hasFormat(QLatin1String("application/x-uml-clip3"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QLatin1String("application/x-uml-clip3"));
    if (!payload.size()) {
        return false;
    }
    QTextStream clipdata(payload, QIODevice::ReadOnly);
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        uWarning() << "Cannot set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QLatin1String("xmiclip")) {
        return false;
    }

    //listviewitems
    QDomNode listItemNode = xmiClipNode.firstChild();
    QDomNode listItems = listItemNode.firstChild();
    QDomElement listItemElement = listItems.toElement();
    if (listItemElement.isNull()) {
        uWarning() << "no listitems in XMI clip";
        return false;
    }
    while (!listItemElement.isNull()) {
        // Get the ListViewType beforehand so that we can construct an
        // UMLListViewItem instance.
        QString type = listItemElement.attribute(QLatin1String("type"), QLatin1String("-1"));
        if (type == QLatin1String("-1")) {
            uDebug() << "Type not found.";
            listItems = listItems.nextSibling();
            listItemElement = listItems.toElement();
            continue;
        }
        UMLListViewItem::ListViewType t = (UMLListViewItem::ListViewType)(type.toInt());
        UMLListViewItem* parent = parentListView->determineParentItem(t);
        UMLListViewItem* itemData = new UMLListViewItem(parent);
        if (itemData->loadFromXMI1(listItemElement))
            umlListViewItems.append(itemData);
        else
            delete itemData;
        listItems = listItems.nextSibling();
        listItemElement = listItems.toElement();
    }
    return true;
}

/**
 * For use when the user selects UML Objects from a
 * Diagram. The Selected widgets and the relationships
 * between only selected widgets will be copied
 *
 * decodes Mime type = "application/x-uml-clip4"
 */
bool UMLDragData::decodeClip4(const QMimeData* mimeData, UMLObjectList& objects,
                          UMLWidgetList& widgets,
                          AssociationWidgetList& associations, Uml::DiagramType::Enum &dType)
{
    if (!mimeData->hasFormat(QLatin1String("application/x-uml-clip4"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QLatin1String("application/x-uml-clip4"));
    if (!payload.size()) {
        return false;
    }

    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        uWarning() << "Cannot set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QLatin1String("xmiclip")) {
        return false;
    }

    dType = Uml::DiagramType::fromInt(root.attribute(QLatin1String("diagramtype"), QLatin1String("0")).toInt());
    QDomNode objectsNode = xmiClipNode.firstChild();

    // Load UMLObjects and do not fail if there are none in the clip
    bool hasObjects = !objectsNode.firstChild().toElement().isNull();

    if (hasObjects && !UMLDragData::decodeObjects(objectsNode, objects, true)) {
        return false;
    }

    UMLDoc *doc = UMLApp::app()->document();
    UMLView *view = UMLApp::app()->currentView();
    UMLScene *scene = view->umlScene();

    QString sourceDiagramID = root.attribute(QLatin1String("diagramid"), QLatin1String(""));
    UMLView *sourceView = doc->findView(Uml::ID::fromString(sourceDiagramID));

    bool pasteToDiagramCopiedFrom = sourceView && sourceView->umlScene()->ID() == scene->ID();

    // Load widgets
    QDomNode widgetsNode = objectsNode.nextSibling();
    QDomNode widgetNode = widgetsNode.firstChild();
    QDomElement widgetElement = widgetNode.toElement();
    if (widgetElement.isNull()) {
        uWarning() << "No widgets in XMI clip.";
        return false;
    }

    while (!widgetElement.isNull()) {

        UMLWidget* widget = scene->loadWidgetFromXMI(widgetElement);
        if (widget) {
            if (pasteToDiagramCopiedFrom && widget->isObjectWidget()) {
                delete widget;
                widgetNode = widgetNode.nextSibling();
                widgetElement = widgetNode.toElement();
                continue;
            }

            // check if widget is pastable
            if (!sourceView || sourceView->umlScene()->type() != scene->type()) {
                UMLObject *object = widget->umlObject();
                if (object) {
                    if (!Model_Utils::typeIsAllowedInDiagram(object, scene)) {
                        delete widget;
                        widgetNode = widgetNode.nextSibling();
                        widgetElement = widgetNode.toElement();
                        continue;
                    }
                }
                else if (!Model_Utils::typeIsAllowedInDiagram(widget, scene)) {
                    delete widget;
                    widgetNode = widgetNode.nextSibling();
                    widgetElement = widgetNode.toElement();
                    continue;
                }
            }

            // Generate a new unique 'local ID' so a second widget for the same
            // UMLObject can be distinguished from the first widget
            widget->setLocalID(doc->assignNewID(widget->localID()));

            if (widget->isMessageWidget()) {
                MessageWidget *w = widget->asMessageWidget();
                if (w && w->floatingTextWidget()) {
                    w->floatingTextWidget()->setLocalID(doc->assignNewID(w->floatingTextWidget()->localID()));
                    w->floatingTextWidget()->setID(doc->assignNewID(w->floatingTextWidget()->id()));
                }
            }
            // Add the widget to the UMLWidgetList for reference in
            // UMLClipboard
            widgets.append(widget);
        } else {
            uWarning() << "Unable to paste widget" << widgetElement.tagName();
        }

        widgetNode = widgetNode.nextSibling();
        widgetElement = widgetNode.toElement();
    }

    if (widgets.size() == 0)
        return false;

    IDChangeLog* log = doc->changeLog();

    // Make sure all object widgets are loaded before adding messages or
    // preconditions
    if (!pasteToDiagramCopiedFrom) {
        foreach (UMLWidget* widget, widgets) {
            if (widget->isObjectWidget()) {
                executeCreateWidgetCommand(widget);
            }
        }
    }

    // Now add all remaining widgets
    foreach (UMLWidget* widget, widgets) {
        if (!pasteToDiagramCopiedFrom && widget->isMessageWidget()) {
            MessageWidget* message = widget->asMessageWidget();
            message->resolveObjectWidget(log);
        }

        if (widget->isPreconditionWidget()) {
            PreconditionWidget* precondition = widget->asPreconditionWidget();
            precondition->resolveObjectWidget(log);
        }

        if (!widget->isObjectWidget()) {
            executeCreateWidgetCommand(widget);
        }
    }

    // Load AssociationWidgets
    QDomNode associationWidgetsNode = widgetsNode.nextSibling();
    QDomNode associationWidgetNode = associationWidgetsNode.firstChild();
    QDomElement associationWidgetElement = associationWidgetNode.toElement();
    while (!associationWidgetElement.isNull()) {
        AssociationWidget* associationWidget = AssociationWidget::create(view->umlScene());
        if (associationWidget->loadFromXMI1(associationWidgetElement, widgets, 0))
            associations.append(associationWidget);
        else {
            delete associationWidget;
        }
        associationWidgetNode = associationWidgetNode.nextSibling();
        associationWidgetElement = associationWidgetNode.toElement();
    }

    return true;
}

/**
 * For use when the user selects only Attributes and/or
 * Operations from the ListView * copied, decodes Mime
 * type = "application/x-uml-clip5
 */
bool UMLDragData::decodeClip5(const QMimeData* mimeData, UMLObjectList& objects,
                          UMLClassifier* newParent)
{
    if (!mimeData->hasFormat(QLatin1String("application/x-uml-clip5"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QLatin1String("application/x-uml-clip5"));
    if (!payload.size()) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        uWarning() << "Cannot set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QLatin1String("xmiclip")) {
        return false;
    }

    //UMLObjects
    QDomNode objectsNode = xmiClipNode.firstChild();
    QDomNode objectElement = objectsNode.firstChild();
    QDomElement element = objectElement.toElement();
    if (element.isNull()) {
        return false;//return ok as it means there is no umlobjects
    }
    while (!element.isNull()) {
        QString type = element.tagName();
        UMLClassifierListItem *pObject = newParent->makeChildObject(type);
        if(!pObject) {
            uWarning() << "Given wrong type of umlobject to create:" << type;
            return false;
        }
        if(!pObject->loadFromXMI1(element)) {
            uWarning() << "Failed to load object from XMI.";
            return false;
        }
        pObject->resolveRef();
        objects.append(pObject);
        objectElement = objectElement.nextSibling();
        element = objectElement.toElement();
    }

    return true;
}

/**
 * Execute the CmdCreateWidget undo command
 */
void UMLDragData::executeCreateWidgetCommand(UMLWidget* widget)
{
    UMLApp::app()->executeCommand(new Uml::CmdCreateWidget(widget));
}

/**
 * Decode UMLObjects from clip
 */
bool UMLDragData::decodeObjects(QDomNode& objectsNode, UMLObjectList& objects, bool skipIfObjectExists)
{
    UMLDoc* doc = UMLApp::app()->document();
    QDomNode objectElement = objectsNode.firstChild();
    QDomElement element = objectElement.toElement();
    if (element.isNull()) {
        return false;//return ok as it means there is no umlobjects
    }
    UMLObject* pObject = 0;
    while (!element.isNull()) {
        pObject = 0;
        QString type = element.tagName();
        Uml::ID::Type elmId = Uml::ID::fromString(element.attribute(QLatin1String("xmi.id")));
        QString stereotype = element.attribute(QLatin1String("stereotype"));

        bool objectExists = (doc->findObjectById(elmId) != 0);

        // This happens when pasting clip4 (widgets): pasting widgets must
        // not duplicate the UMLObjects, unless they don't exists (other
        // instance of umbrello)
        if (skipIfObjectExists && objectExists) {
            objectElement = objectElement.nextSibling();
            element = objectElement.toElement();
            continue;
        }

        // Remove ownedElements from containers: the clip already contains all children
        // as a flat list (UMLClipboard::insertItemChildren)
        if (type == QLatin1String("UML:Package") ||
            type == QLatin1String("UML:Class") ||
            type == QLatin1String("UML:Interface") ||
            type == QLatin1String("UML:Component")) {
            QDomNodeList list = element.childNodes();
            for (int i=(list.length() - 1); i>=0; i--) {
                QDomNode child = list.at(i);
                QString tagName = child.toElement().tagName();
                if (tagName == QLatin1String("UML:Namespace.ownedElement") ||
                    tagName == QLatin1String("UML:Namespace.contents")) {
                    element.removeChild(child);
                }
            }
        }

        pObject = Object_Factory::makeObjectFromXMI(type, stereotype);
        if(!pObject) {
            uWarning() << "Given wrong type of umlobject to create: " << type;
            return false;
        }

        Uml::ID::Type oldParentId = Uml::ID::fromString(
            element.attribute(QLatin1String("namespace"), QLatin1String("-1"))
        );

        // Determine the parent package of the pasted object
        UMLPackage* newParent = 0;
        if (oldParentId != Uml::ID::None) {
            Uml::ID::Type newParentId = doc->changeLog()->findNewID(oldParentId);

            if (newParentId == Uml::ID::None) {
                // Fallback to parent ID before paste (folder was not pasted in
                // this paste operation)
                newParentId = oldParentId;
            }

            newParent = doc->findObjectById(newParentId)->asUMLPackage();
        }

        if (newParent == 0) {
            // Package is not in this clip, determine the parent based
            // on the selected tree view item
            newParent = Model_Utils::treeViewGetPackageFromCurrent();
        }

        pObject->setUMLPackage(newParent);

        // Note: element should not be used after calling loadFromXMI1() because
        // it can point to an arbitrary child node
        if(!pObject->loadFromXMI1(element)) {
            uWarning() << "Failed to load object of type " << type << " from XMI";
            delete pObject;
            return false;
        }

        // Assign a new ID if the object already existed before this paste,
        // this happens when pasting on listview items in the same document.
        if (objectExists) {
            pObject->setID(
                doc->assignNewID(pObject->id())
            );
        }

        UMLApp::app()->executeCommand(new Uml::CmdCreateUMLObject(pObject));
        doc->signalUMLObjectCreated(pObject);

        objects.append(pObject);
        objectElement = objectElement.nextSibling();
        element = objectElement.toElement();
    }

    return true;
}

/**
 * Decode views from clip
 */
bool UMLDragData::decodeViews(QDomNode& umlviewsNode, UMLViewList& diagrams)
{
    QDomNode diagramNode = umlviewsNode.firstChild();
    QDomElement diagramElement = diagramNode.toElement();
    if (diagramElement.isNull()) {
        uWarning() << "No diagrams in XMI clip.";
        return false;
    }
    UMLListView *listView = UMLApp::app()->listView();
    while (!diagramElement.isNull()) {
        if (NoteWidget::s_pCurrentNote) {
            QString idStr = diagramElement.attribute(QLatin1String("xmi.id"), QLatin1String("-1"));
            Uml::ID::Type id = Uml::ID::fromString(idStr);
            if (id == Uml::ID::None) {
                uDebug() << "Cannot paste diagram hyperlink to note because decoding of xmi.id failed";
                return false;
            }
            NoteWidget::s_pCurrentNote->setDiagramLink(id);
            return true;
        }
        QString type = diagramElement.attribute(QLatin1String("type"), QLatin1String("0"));
        Uml::DiagramType::Enum dt = Uml::DiagramType::fromInt(type.toInt());
        UMLListViewItem *parent = listView->findFolderForDiagram(dt);
        if (parent == 0)
            return false;
        UMLObject *po = parent->umlObject();
        if (po == 0 || po->baseType() != UMLObject::ot_Folder) {
            uError() << "Bad parent for view.";
            return false;
        }
        UMLFolder *f = po->asUMLFolder();
        UMLView* view = new UMLView(f);
        view->umlScene()->loadFromXMI1(diagramElement);
        diagrams.append(view);
        diagramNode = diagramNode.nextSibling();
        diagramElement = diagramNode.toElement();
    }

    return true;
}

/**
 * Converts application/x-uml-clip[1-5] clip type to an integer
 */
int UMLDragData::getCodingType(const QMimeData* mimeData)
{
    int result = 0;
    if (mimeData->hasFormat(QLatin1String("application/x-uml-clip1"))) {
        result = 1;
    }
    if (mimeData->hasFormat(QLatin1String("application/x-uml-clip2"))) {
        result = 2;
    }
    if (mimeData->hasFormat(QLatin1String("application/x-uml-clip3"))) {
        result = 3;
    }
    if (mimeData->hasFormat(QLatin1String("application/x-uml-clip4"))) {
        result = 4;
    }
    if (mimeData->hasFormat(QLatin1String("application/x-uml-clip5"))) {
        result = 5;
    }
    if (mimeData->hasFormat(QLatin1String("text/plain"))) {
        result = 6;
    }
    return result;
}

