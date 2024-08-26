/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umldragdata.h"

// local includes
#include "associationwidget.h"
#include "classifier.h"
#include "cmds.h"
#define DBG_SRC QStringLiteral("UMLDragData")
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
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(UMLDragData)

/**
 *  Constructor.
 */
UMLDragData::UMLDragData(UMLObjectList& objects, QWidget* dragSource /* = nullptr */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip1(objects);
}

/**
 * For use when the user selects UML Object and Diagrams
 * from the ListView to be copied, Mime type =
 * "application/x-uml-clip2
 */
UMLDragData::UMLDragData(UMLObjectList& objects, UMLViewList& diagrams, QWidget* dragSource /* = nullptr */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip2(objects, diagrams);
}

/**
 *  Constructor.
 */
UMLDragData::UMLDragData(UMLListViewItemList& umlListViewItems,
                         QWidget* dragSource /* = nullptr */)
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
                         QPixmap& pngImage, UMLScene *scene, QWidget *dragSource /* = nullptr */)
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
                         QWidget* dragSource /* = nullptr */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip5(objects);
}

/**
 *  Constructor.
 */
UMLDragData::UMLDragData(QWidget* dragSource /* = nullptr */)
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
    QString xmiClip;
    QXmlStreamWriter stream(&xmiClip);
    stream.writeStartElement(QStringLiteral("xmiclip"));
    stream.writeStartElement(QStringLiteral("umlobjects"));

    UMLObjectListIt object_it(objects);
    UMLObject *obj = nullptr;
    while (object_it.hasNext()) {
        obj = object_it.next();
        obj->saveToXMI(stream);
    }

    stream.writeEndElement();  // umlobjects
    stream.writeEndElement();  // xmiclip
    setData(QStringLiteral("application/x-uml-clip1"), xmiClip.toUtf8());
}

/**
 * For use when the user selects UML Object and Diagrams
 * from the ListView to be copied
 */
void UMLDragData::setUMLDataClip2(UMLObjectList& objects, UMLViewList& diagrams)
{
    QString xmiClip;
    QXmlStreamWriter stream(&xmiClip);
    stream.writeStartElement(QStringLiteral("xmiclip"));
    stream.writeStartElement(QStringLiteral("umlobjects"));

    UMLObjectListIt object_it(objects);
    UMLObject *obj = nullptr;
    while (object_it.hasNext()) {
        obj = object_it.next();
        obj->saveToXMI(stream);
    }

    stream.writeEndElement();  // umlobjects
    stream.writeStartElement(QStringLiteral("umlviews"));

    for(UMLView* view : diagrams) {
        view->umlScene()->saveToXMI(stream);
    }

    stream.writeEndElement();  // umlviews
    stream.writeEndElement();  // xmiclip
    setData(QStringLiteral("application/x-uml-clip2"), xmiClip.toUtf8());
}

/**
 * For use when the user selects only empty folders from the ListView
 * to be copied.
 */
void UMLDragData::setUMLDataClip3(UMLListViewItemList& umlListViewItems)
{
    QString xmiClip;
    QXmlStreamWriter stream(&xmiClip);
    stream.writeStartElement(QStringLiteral("xmiclip"));
    stream.writeStartElement(QStringLiteral("umllistviewitems"));

    for(UMLListViewItem* item : umlListViewItems) {
        item->saveToXMI(stream);
    }

    stream.writeEndElement();  // umllistviewitems
    stream.writeEndElement();  // xmiclip
    setData(QStringLiteral("application/x-uml-clip3"), xmiClip.toUtf8());
}

/**
 * For use when the user selects UML Objects from a
 * Diagram. The Selected widgets and the relationships
 * between only selected widgets will be copied and also
 * its respective ListView Items
 */
void UMLDragData::setUMLDataClip4(UMLObjectList& objects,
                                  UMLWidgetList& widgets,
                                  AssociationWidgetList& associations,
                                  QPixmap& pngImage, UMLScene *scene)
{
    QString xmiClip;
    QXmlStreamWriter stream(&xmiClip);
    stream.writeStartElement(QStringLiteral("xmiclip"));
    stream.writeAttribute(QStringLiteral("diagramtype"), QString::number(scene->type()));
    stream.writeAttribute(QStringLiteral("diagramid"), Uml::ID::toString(scene->ID()));
    stream.writeStartElement(QStringLiteral("umlobjects"));

    for(UMLObject* obj : objects) {
        obj->saveToXMI(stream);
    }

    stream.writeEndElement();  // umlobjects
    stream.writeStartElement(QStringLiteral("widgets"));

    for(UMLWidget* widget : widgets) {
        widget->saveToXMI(stream);
    }

    stream.writeEndElement();  // widgets
    stream.writeStartElement(QStringLiteral("associations"));

    for(AssociationWidget* association : associations) {
        association->saveToXMI(stream);
    }

    stream.writeEndElement();  // associations
    stream.writeEndElement();  // xmiclip
    setData(QStringLiteral("application/x-uml-clip4"), xmiClip.toUtf8());

    QImage img = pngImage.toImage();
    int l_size = img.sizeInBytes();
    QByteArray clipdata;
    clipdata.resize(l_size);
    QDataStream clipstream(&clipdata, QIODevice::WriteOnly);
    clipstream << img;

    setImageData(clipdata);
}

/**
 * For use when the user selects only Attributes and/or
 * Operation from the ListView
 */
void UMLDragData::setUMLDataClip5(UMLObjectList& objects)
{
    QString xmiClip;
    QXmlStreamWriter stream(&xmiClip);
    stream.writeStartElement(QStringLiteral("xmiclip"));
    stream.writeStartElement(QStringLiteral("umlobjects"));

    for(UMLObject* obj : objects) {
        obj->saveToXMI(stream);
    }

    stream.writeEndElement();  // umlobjects
    stream.writeEndElement();  // xmiclip
    setData(QStringLiteral("application/x-uml-clip5"), xmiClip.toUtf8());
}

/**
 * For use when the user selects only UML Objects
 * from the ListView but no diagrams to be
 * copied, decodes Mime type =
 * "application/x-uml-clip1
 */
bool UMLDragData::decodeClip1(const QMimeData* mimeData, UMLObjectList& objects)
{
    if (!mimeData->hasFormat(QStringLiteral("application/x-uml-clip1"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QStringLiteral("application/x-uml-clip1"));
    if (!payload.size()) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        logWarn2("UMLDragData::decodeClip1: Cannot set content. Error %1 line %2", error, line);
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QStringLiteral("xmiclip")) {
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
    if (!mimeData->hasFormat(QStringLiteral("application/x-uml-clip2"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QStringLiteral("application/x-uml-clip2"));
    if (!payload.size()) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        logWarn2("UMLDragData::decodeClip2: Cannot set content. Error %1 line %2", error, line);
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QStringLiteral("xmiclip")) {
        return false;
    }

    // Load UMLObjects
    QDomNode objectsNode = xmiClipNode.firstChild();
    if (NoteWidget::s_pCurrentNote == nullptr) {
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
    if (!mimeData->hasFormat(QStringLiteral("application/x-uml-clip3"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QStringLiteral("application/x-uml-clip3"));
    if (!payload.size()) {
        return false;
    }
    QTextStream clipdata(payload, QIODevice::ReadOnly);
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        logWarn2("UMLDragData::getClip3TypeAndID: Cannot set content. Error %1 line %2", error, line);
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QStringLiteral("xmiclip")) {
        return false;
    }

    QDomNode listItemNode = xmiClipNode.firstChild();
    QDomNode listItems = listItemNode.firstChild();
    QDomElement listItemElement = listItems.toElement();
    if (listItemElement.isNull()) {
        logWarn0("UMLDragData::getClip3TypeAndID: No listitems in XMI clip.");
        return false;
    }
    while (!listItemElement.isNull()) {
        QString typeStr = listItemElement.attribute(QStringLiteral("type"), QStringLiteral("-1"));
        if (typeStr == QStringLiteral("-1")) {
            logDebug0("UMLDragData::getClip3TypeAndID: bad type.");
            return false;
        }
        QString idStr = listItemElement.attribute(QStringLiteral("id"), QStringLiteral("-1"));
        if (idStr == QStringLiteral("-1")) {
            logDebug0("UMLDragData::getClip3TypeAndID: bad id");
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
 * For use when the user selects UMLObjects from the ListView to be copied,
 * decodes Mime type = "application/x-uml-clip3
 */
bool UMLDragData::decodeClip3(const QMimeData* mimeData, UMLListViewItemList& umlListViewItems,
                            const UMLListView* parentListView)
{
    if (!mimeData->hasFormat(QStringLiteral("application/x-uml-clip3"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QStringLiteral("application/x-uml-clip3"));
    if (!payload.size()) {
        return false;
    }
    QTextStream clipdata(payload, QIODevice::ReadOnly);
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        logWarn2("UMLDragData::decodeClip3: Cannot set content. Error %1 line %2", error, line);
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QStringLiteral("xmiclip")) {
        return false;
    }

    //listviewitems
    QDomNode listItemNode = xmiClipNode.firstChild();
    QDomNode listItems = listItemNode.firstChild();
    QDomElement listItemElement = listItems.toElement();
    if (listItemElement.isNull()) {
        logWarn0("UMLDragData::decodeClip3: no listitems in XMI clip");
        return false;
    }
    while (!listItemElement.isNull()) {
        // Get the ListViewType beforehand so that we can construct an
        // UMLListViewItem instance.
        QString type = listItemElement.attribute(QStringLiteral("type"), QStringLiteral("-1"));
        if (type == QStringLiteral("-1")) {
            logDebug0("UMLDragData::decodeClip3: Type not found.");
            listItems = listItems.nextSibling();
            listItemElement = listItems.toElement();
            continue;
        }
        UMLListViewItem::ListViewType t = (UMLListViewItem::ListViewType)(type.toInt());
        UMLListViewItem* parent = parentListView->determineParentItem(t);
        UMLListViewItem* itemData = new UMLListViewItem(parent);
        if (itemData->loadFromXMI(listItemElement))
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
    if (!mimeData->hasFormat(QStringLiteral("application/x-uml-clip4"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QStringLiteral("application/x-uml-clip4"));
    if (!payload.size()) {
        return false;
    }

    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        logWarn2("UMLDragData::decodeClip4: Cannot set content. Error %1 line %2", error, line);
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QStringLiteral("xmiclip")) {
        return false;
    }

    dType = Uml::DiagramType::fromInt(root.attribute(QStringLiteral("diagramtype"), QStringLiteral("0")).toInt());
    QDomNode objectsNode = xmiClipNode.firstChild();

    // Load UMLObjects and do not fail if there are none in the clip
    bool hasObjects = !objectsNode.firstChild().toElement().isNull();

    if (hasObjects && !UMLDragData::decodeObjects(objectsNode, objects, true)) {
        return false;
    }

    UMLDoc *doc = UMLApp::app()->document();
    UMLView *view = UMLApp::app()->currentView();
    UMLScene *scene = view->umlScene();

    QString sourceDiagramID = root.attribute(QStringLiteral("diagramid"), QStringLiteral(""));
    UMLView *sourceView = doc->findView(Uml::ID::fromString(sourceDiagramID));

    bool fromSameDiagram = sourceView && sourceView->umlScene()->ID() == scene->ID();
    bool fromAnotherInstance = !sourceView;
    bool fromDifferentDiagramType = dType != scene->type();

    // Load widgets
    QDomNode widgetsNode = objectsNode.nextSibling();
    QDomNode widgetNode = widgetsNode.firstChild();
    QDomElement widgetElement = widgetNode.toElement();
    if (widgetElement.isNull()) {
        logWarn0("UMLDragData::decodeClip4: No widgets in XMI clip.");
        return false;
    }

    while (!widgetElement.isNull()) {

        UMLWidget* widget = scene->loadWidgetFromXMI(widgetElement);
        if (widget) {
            if (fromSameDiagram && widget->isObjectWidget()) {
                delete widget;
                widgetNode = widgetNode.nextSibling();
                widgetElement = widgetNode.toElement();
                continue;
            }

            // check if widget is pastable
            if (fromAnotherInstance || fromDifferentDiagramType) {
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
            } else if (Model_Utils::isCloneable(widget->baseType())) {
                if (widget->umlObject()) {
                    UMLObject *clone = widget->umlObject()->clone();
                    widget->setUMLObject(clone);
                    // we do not want to recreate an additional widget,
                    // which would be the case if calling scene->addUMLObject()
                    UMLApp::app()->document()->addUMLObject(clone);
                    UMLApp::app()->listView()->slotObjectCreated(clone);
                    if (Model_Utils::hasAssociations(clone->baseType()))
                    {
                        scene->createAutoAssociations(widget);
                        scene->createAutoAttributeAssociations2(widget);
                    }
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
            logWarn1("UMLDragData::decodeClip4: Unable to paste widget %1", widgetElement.tagName());
        }

        widgetNode = widgetNode.nextSibling();
        widgetElement = widgetNode.toElement();
    }

    if (widgets.size() == 0)
        return false;

    IDChangeLog* log = doc->changeLog();

    // Make sure all object widgets are loaded before adding messages or
    // preconditions
    if (!fromSameDiagram) {
        for(UMLWidget *widget : widgets) {
            if (widget->isObjectWidget()) {
                executeCreateWidgetCommand(widget);
            }
        }
    }

    // Now add all remaining widgets
    for(UMLWidget *widget : widgets) {
        if (!fromSameDiagram && widget->isMessageWidget()) {
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
        if (associationWidget->loadFromXMI(associationWidgetElement, widgets, nullptr))
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
    if (!mimeData->hasFormat(QStringLiteral("application/x-uml-clip5"))) {
        return false;
    }
    QByteArray payload = mimeData->data(QStringLiteral("application/x-uml-clip5"));
    if (!payload.size()) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if(!domDoc.setContent(xmiClip, false, &error, &line)) {
        logWarn2("UMLDragData::decodeClip5: Cannot set content. Error %1 line %2", error, line);
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if (root.isNull()) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != QStringLiteral("xmiclip")) {
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
            logWarn1("UMLDragData::decodeClip5 given wrong type of umlobject to create: %1", type);
            return false;
        }
        if(!pObject->loadFromXMI(element)) {
            logWarn0("UMLDragData::decodeClip5 failed to load object from XMI.");
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
    UMLObject *pObject = nullptr;
    while (!element.isNull()) {
        pObject = nullptr;
        QString type = element.tagName();
        Uml::ID::Type elmId = Uml::ID::fromString(Model_Utils::getXmiId(element));
        QString stereotype = element.attribute(QStringLiteral("stereotype"));

        bool objectExists = (doc->findObjectById(elmId) != nullptr);

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
        if (type == QStringLiteral("UML:Package") ||
            type == QStringLiteral("UML:Class") ||
            type == QStringLiteral("UML:Interface") ||
            type == QStringLiteral("UML:Component")) {
            QDomNodeList list = element.childNodes();
            for (int i = list.length() - 1; i >= 0; i--) {
                QDomNode child = list.at(i);
                QString tagName = child.toElement().tagName();
                if (tagName == QStringLiteral("UML:Namespace.ownedElement") ||
                    tagName == QStringLiteral("UML:Namespace.contents")) {
                    element.removeChild(child);
                }
            }
        }

        pObject = Object_Factory::makeObjectFromXMI(type, stereotype);
        if(!pObject) {
            logWarn1("UMLDragData::decodeObjects given wrong type of umlobject to create: %1", type);
            return false;
        }

        Uml::ID::Type oldParentId = Uml::ID::fromString(
            element.attribute(QStringLiteral("namespace"), QStringLiteral("-1"))
        );

        // Determine the parent package of the pasted object
        UMLPackage *newParent = nullptr;
        if (oldParentId != Uml::ID::None) {
            Uml::ID::Type newParentId = doc->changeLog()->findNewID(oldParentId);

            if (newParentId == Uml::ID::None) {
                // Fallback to parent ID before paste (folder was not pasted in
                // this paste operation)
                newParentId = oldParentId;
            }

            newParent = doc->findObjectById(newParentId)->asUMLPackage();
        }

        if (newParent == nullptr) {
            // Package is not in this clip, determine the parent based
            // on the selected tree view item
            newParent = Model_Utils::treeViewGetPackageFromCurrent();
        }

        pObject->setUMLPackage(newParent);

        // Note: element should not be used after calling loadFromXMI() because
        // it can point to an arbitrary child node
        if (!pObject->loadFromXMI(element)) {
            logWarn1("UMLDragData::decodeObjects failed to load object of type %1 from XMI", type);
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
        logWarn0("UMLDragData::decodeViews: No diagrams in XMI clip.");
        return false;
    }
    UMLListView *listView = UMLApp::app()->listView();
    while (!diagramElement.isNull()) {
        if (NoteWidget::s_pCurrentNote) {
            QString idStr = diagramElement.attribute(QStringLiteral("xmi.id"), QStringLiteral("-1"));
            Uml::ID::Type id = Uml::ID::fromString(idStr);
            if (id == Uml::ID::None) {
                logDebug0("UMLDragData::decodeViews: Cannot paste diagram hyperlink to note because decoding of xmi.id failed");
                return false;
            }
            NoteWidget::s_pCurrentNote->setDiagramLink(id);
            return true;
        }
        QString type = diagramElement.attribute(QStringLiteral("type"), QStringLiteral("0"));
        Uml::DiagramType::Enum dt = Uml::DiagramType::fromInt(type.toInt());
        UMLListViewItem *parent = listView->findFolderForDiagram(dt);
        if (parent == nullptr)
            return false;
        UMLObject *po = parent->umlObject();
        if (po == nullptr || po->baseType() != UMLObject::ot_Folder) {
            logError0("UMLDragData::decodeViews: Bad parent for view.");
            return false;
        }
        UMLFolder *f = po->asUMLFolder();
        UMLView* view = new UMLView(f);
        view->umlScene()->loadFromXMI(diagramElement);
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
    if (mimeData->hasFormat(QStringLiteral("application/x-uml-clip1"))) {
        result = 1;
    }
    if (mimeData->hasFormat(QStringLiteral("application/x-uml-clip2"))) {
        result = 2;
    }
    if (mimeData->hasFormat(QStringLiteral("application/x-uml-clip3"))) {
        result = 3;
    }
    if (mimeData->hasFormat(QStringLiteral("application/x-uml-clip4"))) {
        result = 4;
    }
    if (mimeData->hasFormat(QStringLiteral("application/x-uml-clip5"))) {
        result = 5;
    }
    if (mimeData->hasFormat(QStringLiteral("text/plain"))) {
        result = 6;
    }
    return result;
}

