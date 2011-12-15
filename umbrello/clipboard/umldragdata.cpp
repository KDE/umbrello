/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umldragdata.h"

// local includes
#include "debug_utils.h"
#include "idchangelog.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlobject.h"
#include "folder.h"
#include "classifier.h"
#include "umlwidget.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "associationwidget.h"
#include "object_factory.h"
#include "model_utils.h"

// qt includes
#include <QtCore/QTextStream>
#include <QtXml/QDomDocument>
#include <QtGui/QPixmap>

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
UMLDragData::UMLDragData(UMLObjectList& objects, UMLListViewItemList& umlListViewItems, UMLViewList& diagrams,
                 QWidget* dragSource /* = 0 */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip2(objects, umlListViewItems, diagrams);
}

/**
 *  Constructor.
 */
UMLDragData::UMLDragData(UMLListViewItemList& umlListViewItems, QWidget* dragSource /* = 0 */)
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
                 QPixmap& pngImage, Uml::DiagramType dType, QWidget* dragSource /* = 0 */)
{
    Q_UNUSED(dragSource);
    setUMLDataClip4(objects, widgets, associationDatas, pngImage, dType);
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
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement("umlobjects");
    xmiclip.appendChild(objectsTag);

    UMLObjectListIt object_it(objects);
    UMLObject* obj = 0;
    while ( object_it.hasNext() ) {
        obj = object_it.next();
        obj->saveToXMI(domDoc, objectsTag);
    }

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    setData( "application/x-uml-clip1", domDoc.toString().toUtf8() );
}

/**
 * For use when the user selects UML Object and Diagrams
 * from the ListView to be copied
 */
void UMLDragData::setUMLDataClip2(UMLObjectList& objects, UMLListViewItemList& umlListViewItems,
                              UMLViewList& diagrams)
{
    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement("umlobjects");
    xmiclip.appendChild(objectsTag);

    UMLObjectListIt object_it(objects);
    UMLObject* obj = 0;
    while ( object_it.hasNext() ) {
        obj = object_it.next();
        obj->saveToXMI(domDoc, objectsTag);
    }

    QDomElement viewsTag = domDoc.createElement("umlviews");
    xmiclip.appendChild(viewsTag);

    foreach( UMLView* view, diagrams ) {
        view->saveToXMI(domDoc, viewsTag);
    }

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    foreach( UMLListViewItem* item, umlListViewItems ) {
        item->saveToXMI(domDoc, itemsTag);
    }

    setData( "application/x-uml-clip2", domDoc.toString().toUtf8() );
}

/**
 * For use when the user selects only empty folders from the ListView
 * to be copied.
 */
void UMLDragData::setUMLDataClip3(UMLListViewItemList& umlListViewItems)
{
    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    domDoc.appendChild(xmiclip);

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    foreach( UMLListViewItem* item, umlListViewItems ) {
        item->saveToXMI(domDoc, itemsTag);
    }

    setData( "application/x-uml-clip3", domDoc.toString().toUtf8() );
}

/**
 * For use when the user selects UML Objects from a
 * Diagram. The Selected widegets and the relationships
 * between only selected widgets will be copied and also
 * its respective ListView Items
 */
void UMLDragData::setUMLDataClip4(UMLObjectList& objects, UMLWidgetList& widgets, AssociationWidgetList& associations,
                              QPixmap& pngImage, Uml::DiagramType dType )
{
    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    xmiclip.setAttribute("diagramtype", dType);
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement("umlobjects");
    xmiclip.appendChild(objectsTag);

    foreach ( UMLObject* obj, objects ) {
        obj->saveToXMI(domDoc, objectsTag);
    }

    QDomElement widgetsTag = domDoc.createElement("widgets");
    xmiclip.appendChild(widgetsTag);

    foreach ( UMLWidget* widget, widgets ) {
        widget->saveToXMI(domDoc, widgetsTag);
    }

    QDomElement associationWidgetsTag = domDoc.createElement("associations");
    xmiclip.appendChild(associationWidgetsTag);

    foreach ( AssociationWidget* association, associations ) {
        association->saveToXMI(domDoc, associationWidgetsTag);
    }

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    setData( "application/x-uml-clip4", domDoc.toString().toUtf8() );

    QImage img = pngImage.toImage();
    int l_size = img.numBytes();
    QByteArray clipdata;
    clipdata.resize(l_size);
    QDataStream clipstream( &clipdata,QIODevice::WriteOnly);
    clipstream << img;

    setImageData( clipdata );
}

/**
 * For use when the user selects only Attirbutes and/or
 * Operation from the ListView
 */
void UMLDragData::setUMLDataClip5(UMLObjectList& objects)
{
    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement("umlobjects");
    xmiclip.appendChild(objectsTag);

    foreach ( UMLObject* obj, objects ) {
        obj->saveToXMI(domDoc, objectsTag);
    }

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    setData( "application/x-uml-clip5", domDoc.toString().toUtf8() );
}

/**
 * For use when the user selects only UML Objects
 * from the ListView but no diagrams to be
 * copied, decodes Mime type =
 * "application/x-uml-clip1
 */
bool UMLDragData::decodeClip1(const QMimeData* mimeData, UMLObjectList& objects)
{
    UMLDoc* doc = UMLApp::app()->document();
    if ( !mimeData->hasFormat("application/x-uml-clip1") ) {
        return false;
    }
    QByteArray payload = mimeData->data("application/x-uml-clip1");
    if ( !payload.size() ) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        uWarning() << "Can not set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if ( root.isNull() ) {
        return false;
    }
    //  make sure it is an XMI clip
    if ( root.tagName() != "xmiclip" ) {
        return false;
    }

    UMLListView *listView = UMLApp::app()->listView();

    //UMLObjects
    QDomNode objectsNode = xmiClipNode.firstChild();
    QDomNode objectElement = objectsNode.firstChild();
    QDomElement element = objectElement.toElement();
    if ( element.isNull() ) {
        return false;//return ok as it means there is no umlobjects
    }
    UMLObject* pObject = 0;
    while ( !element.isNull() ) {
        pObject = 0;
        QString type = element.tagName();
        if (type == "UML:Association") {
            objectElement = objectElement.nextSibling();
            element = objectElement.toElement();
            continue;
        }
        pObject = Object_Factory::makeObjectFromXMI(type);

        if( !pObject ) {
            uWarning() << "Given wrong type of umlobject to create: " << type;
            return false;
        }
        pObject->setInPaste( true );
        if( !pObject->loadFromXMI( element ) ) {
            uWarning() << "Failed to load object of type " << type << " from XMI";
            delete pObject;
            return false;
        }
        pObject->setInPaste( false );
        if (listView->startedCopy()) {
            /****************************************************************
            * If the clone() methods called IDChangeLog::addIDChange(),
            * we could do the following:
            UMLObject *newObj = pObject->clone();
            delete pObject;
            pObject = newObj;
            * but since that's not currently the case we do: */
            if(!doc->assignNewIDs(pObject)) {
                return false;
            }
            UMLObject::ObjectType type = pObject->baseType();
            QString newName = Model_Utils::uniqObjectName(type, pObject->umlPackage(),
                                                                pObject->name());
            pObject->setName(newName);
            /****************************************************************/
        }

        pObject->resolveRef();

        objects.append(pObject);
        objectElement = objectElement.nextSibling();
        element = objectElement.toElement();
    }

    return true;
}

/**
 * For use when the user selects UML Object and Diagrams
 * from the ListView to be copied, decodes Mime type =
 * "application/x-uml-clip2
 */
bool UMLDragData::decodeClip2(const QMimeData* mimeData, UMLObjectList& objects,
                          UMLListViewItemList& umlListViewItems, UMLViewList& diagrams)
{
    if ( !mimeData->hasFormat("application/x-uml-clip2") ) {
        return false;
    }
    QByteArray payload = mimeData->data("application/x-uml-clip2");
    if ( !payload.size() ) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        uWarning() << "Can not set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if ( root.isNull() ) {
        return false;
    }
    //  make sure it is an XMI clip
    if ( root.tagName() != "xmiclip" ) {
        return false;
    }

    //UMLObjects
    QDomNode objectsNode = xmiClipNode.firstChild();
    QDomNode objectElement = objectsNode.firstChild();
    QDomElement element = objectElement.toElement();
    if ( element.isNull() ) {
        return false;//return ok as it means there is no umlobjects
    }
    UMLObject* pObject = 0;
    while ( !element.isNull() ) {
        pObject = 0;
        QString type = element.tagName();
        if (type != "UML:Association") {
            pObject = Object_Factory::makeObjectFromXMI(type);

            if( !pObject ) {
                uWarning() << "Given wrong type of umlobject to create:" << type;
                return false;
            }
            if( !pObject->loadFromXMI(element) ) {
                uWarning() << "Failed to load object from XMI.";
                return false;
            }
            objects.append(pObject);
        }
        objectElement = objectElement.nextSibling();
        element = objectElement.toElement();
    }

    //UMLViews (diagrams)
    QDomNode umlviewsNode = objectsNode.nextSibling();
    QDomNode diagramNode = umlviewsNode.firstChild();
    QDomElement diagramElement = diagramNode.toElement();
    if ( diagramElement.isNull() ) {
        uWarning() << "No diagrams in XMI clip.";
        return false;
    }
    UMLListView *listView = UMLApp::app()->listView();
    while ( !diagramElement.isNull() ) {
        QString type = diagramElement.attribute("type", "0");
        Uml::DiagramType dt = Uml::DiagramType(Uml::DiagramType::Value(type.toInt()));
        UMLListViewItem *parent = listView->findFolderForDiagram(dt);
        if (parent == NULL)
            return false;
        UMLObject *po = parent->umlObject();
        if (po == NULL || po->baseType() != UMLObject::ot_Folder) {
            uError() << "Bad parent for view.";
            return false;
        }
        UMLFolder *f = static_cast<UMLFolder*>(po);
        UMLView* view = new UMLView(f);
        view->loadFromXMI(diagramElement);
        diagrams.append(view);
        diagramNode = diagramNode.nextSibling();
        diagramElement = diagramNode.toElement();
    }

    //listviewitems
    QDomNode listItemNode = umlviewsNode.nextSibling();
    QDomNode listItems = listItemNode.firstChild();
    QDomElement listItemElement = listItems.toElement();
    if ( listItemElement.isNull() ) {
        uWarning() << "No listitems in XMI clip.";
        return false;
    }
    UMLListViewItem *currentItem = (UMLListViewItem*)listView->currentItem();
    while ( !listItemElement.isNull() ) {
        UMLListViewItem* itemData;
        if (currentItem)
            itemData = new UMLListViewItem( currentItem );
        else
            itemData = new UMLListViewItem( listView );
        if ( itemData->loadFromXMI(listItemElement) )
            umlListViewItems.append(itemData);
        else
            delete itemData;
        listItems = listItems.nextSibling();
        listItemElement = listItems.toElement();
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
    if ( !mimeData->hasFormat("application/x-uml-clip3") ) {
        return false;
    }
    QByteArray payload = mimeData->data("application/x-uml-clip3");
    if ( !payload.size() ) {
        return false;
    }
    QTextStream clipdata(payload, QIODevice::ReadOnly);
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        uWarning() << "getClip3Type: Can not set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if ( root.isNull() ) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != "xmiclip") {
        return false;
    }

    QDomNode listItemNode = xmiClipNode.firstChild();
    QDomNode listItems = listItemNode.firstChild();
    QDomElement listItemElement = listItems.toElement();
    if ( listItemElement.isNull() ) {
        uWarning() << "getClip3Type: No listitems in XMI clip.";
        return false;
    }
    while ( !listItemElement.isNull() ) {
        QString typeStr = listItemElement.attribute( "type", "-1" );
        if (typeStr == "-1") {
            uDebug() << "getClip3Type: bad type.";
            return false;
        }
        QString idStr = listItemElement.attribute( "id", "-1" );
        if (idStr == "-1") {
            uDebug() << "getClip3Type: bad id";
            return false;
        }
        LvTypeAndID * pData = new LvTypeAndID;
        pData->type = (UMLListViewItem::ListViewType)(typeStr.toInt());
        pData->id = STR2ID(idStr);
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
    if ( !mimeData->hasFormat("application/x-uml-clip3") ) {
        return false;
    }
    QByteArray payload = mimeData->data("application/x-uml-clip3");
    if ( !payload.size() ) {
        return false;
    }
    QTextStream clipdata(payload, QIODevice::ReadOnly);
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        uWarning() << "Can not set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if ( root.isNull() ) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != "xmiclip") {
        return false;
    }

    //listviewitems
    QDomNode listItemNode = xmiClipNode.firstChild();
    QDomNode listItems = listItemNode.firstChild();
    QDomElement listItemElement = listItems.toElement();
    if ( listItemElement.isNull() ) {
        uWarning() << "no listitems in XMI clip";
        return false;
    }
    while ( !listItemElement.isNull() ) {
        // Get the ListViewType beforehand so that we can construct an
        // UMLListViewItem instance.
        QString type = listItemElement.attribute( "type", "-1" );
        if (type == "-1") {
            uDebug() << "Type not found.";
            continue;
        }
        UMLListViewItem::ListViewType t = (UMLListViewItem::ListViewType)(type.toInt());
        UMLListViewItem* parent = parentListView->determineParentItem(t);
        UMLListViewItem* itemData = new UMLListViewItem(parent);
        if ( itemData->loadFromXMI(listItemElement) )
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
 * Diagram. The Selected widegets and the relationships
 * between only * selected widgets will be copied and
 * also its respective ListView Items, * decodes Mime
 * type = "application/x-uml-clip4
 */
bool UMLDragData::decodeClip4(const QMimeData* mimeData, UMLObjectList& objects,
                          UMLWidgetList& widgets,
                          AssociationWidgetList& associations, Uml::DiagramType & dType)
{
    if ( !mimeData->hasFormat("application/x-uml-clip4") ) {
        return false;
    }
    QByteArray payload = mimeData->data("application/x-uml-clip4");
    if ( !payload.size() ) {
        return false;
    }

    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        uWarning() << "Can not set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if ( root.isNull() ) {
        return false;
    }
    //  make sure it is an XMI clip
    if ( root.tagName() != "xmiclip" ) {
        return false;
    }

    dType = Uml::DiagramType(Uml::DiagramType::Value(root.attribute("diagramtype", "0").toInt()));

    //UMLObjects
    QDomNode objectsNode = xmiClipNode.firstChild();
    QDomNode objectElement = objectsNode.firstChild();
    QDomElement element = objectElement.toElement();
    while ( !element.isNull() ) {
        UMLObject* pObject = 0;
        QString type = element.tagName();
        //FIXME associations don't load
        if (type == "UML:Association")
            continue;
        pObject = Object_Factory::makeObjectFromXMI(type);

        if ( !pObject ) {
            uWarning() << "Given wrong type of umlobject to create: " << type;
            return false;
        }

        if ( !pObject->loadFromXMI( element ) ) {
            uWarning() << "Failed to load object from XMI.";
            return false;
        }

        objects.append(pObject);
        objectElement = objectElement.nextSibling();
        element = objectElement.toElement();
    }

    //widgets
    QDomNode widgetsNode = objectsNode.nextSibling();
    QDomNode widgetNode = widgetsNode.firstChild();
    QDomElement widgetElement = widgetNode.toElement();
    if ( widgetElement.isNull() ) {
        uWarning() << "No widgets in XMI clip.";
        return false;
    }

    UMLView *view = UMLApp::app()->currentView();
    while ( !widgetElement.isNull() ) {

        UMLWidget* widget = view->loadWidgetFromXMI(widgetElement);
        if (widget)
            widgets.append(widget);

        widgetNode = widgetNode.nextSibling();
        widgetElement = widgetNode.toElement();
    }

    //AssociationWidgets
    QDomNode associationWidgetsNode = widgetsNode.nextSibling();
    QDomNode associationWidgetNode = associationWidgetsNode.firstChild();
    QDomElement associationWidgetElement = associationWidgetNode.toElement();
    while ( !associationWidgetElement.isNull() ) {
        AssociationWidget* associationWidget = new AssociationWidget(view->umlScene());
        if (associationWidget->loadFromXMI(associationWidgetElement, widgets))
            associations.append(associationWidget);
        else {
            //associationWidget->cleanup();
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
    if ( !mimeData->hasFormat("application/x-uml-clip5") ) {
        return false;
    }
    QByteArray payload = mimeData->data("application/x-uml-clip5");
    if ( !payload.size() ) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        uWarning() << "Can not set content:" << error << " Line:" << line;
        return false;
    }
    QDomNode xmiClipNode = domDoc.firstChild();
    QDomElement root = xmiClipNode.toElement();
    if ( root.isNull() ) {
        return false;
    }
    //  make sure it is an XMI clip
    if (root.tagName() != "xmiclip") {
        return false;
    }

    //UMLObjects
    QDomNode objectsNode = xmiClipNode.firstChild();
    QDomNode objectElement = objectsNode.firstChild();
    QDomElement element = objectElement.toElement();
    if ( element.isNull() ) {
        return false;//return ok as it means there is no umlobjects
    }
    while ( !element.isNull() ) {
        QString type = element.tagName();
        UMLClassifierListItem *pObject = newParent->makeChildObject(type);
        if( !pObject ) {
            uWarning() << "Given wrong type of umlobject to create:" << type;
            return false;
        }
        if( !pObject->loadFromXMI( element ) ) {
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
 * Converts application/x-uml-clip[1-5] clip type to an integer
 */
int UMLDragData::getCodingType(const QMimeData* mimeData)
{
    int result = 0;
    if (mimeData->hasFormat("application/x-uml-clip1") ) {
        result = 1;
    }
    if (mimeData->hasFormat("application/x-uml-clip2") ) {
        result = 2;
    }
    if (mimeData->hasFormat("application/x-uml-clip3") ) {
        result = 3;
    }
    if (mimeData->hasFormat("application/x-uml-clip4") ) {
        result = 4;
    }
    if (mimeData->hasFormat("application/x-uml-clip5") ) {
        result = 5;
    }
    return result;
}

#include "umldragdata.moc"
