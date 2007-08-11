/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umldrag.h"

// qt/kde includes
#include <qdom.h>
#include <kdebug.h>

// local includes
#include "idchangelog.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../umlview.h"
#include "../umlobject.h"
#include "../folder.h"
#include "../classifier.h"
#include "../umlwidget.h"
#include "../umllistview.h"
#include "../umllistviewitem.h"
#include "../associationwidget.h"
#include "../object_factory.h"
#include "../model_utils.h"

#define nfmt 4
class UMLDragPrivate {
public:
    QCString fmt[nfmt];
    QCString subtype;
    QByteArray enc[nfmt];

    UMLDragPrivate() {
        setSubType("clip1", 0);
    }

    void setType(const QCString& st, int index) {
        if (index < nfmt) {
            fmt[index] = st.lower();
        }
    }

    void setSubType(const QCString& st, int index) {
        if (index < nfmt) {
            subtype = st.lower();
            fmt[index] = "application/x-uml-";
            fmt[index].append(subtype);
        }
    }

    const char* format(int i) const {
        if(i < nfmt) {
            return fmt[i];
        }
        return 0;
    }
};

UMLDrag::UMLDrag(UMLObjectList& objects, QWidget* dragSource /*= 0*/, const char* name /*= 0*/)
  : QDragObject(dragSource, name) {
    data = new UMLDragPrivate;
    setUMLDataClip1(objects);
}

UMLDrag::UMLDrag(UMLObjectList& objects, UMLListViewItemList& umlListViewItems, UMLViewList& diagrams,
                 QWidget* dragSource /*= 0*/, const char* name /*= 0*/ ): QDragObject(dragSource, name) {
    data = new UMLDragPrivate;
    setUMLDataClip2(objects, umlListViewItems, diagrams);
}

UMLDrag::UMLDrag(UMLListViewItemList& umlListViewItems, QWidget* dragSource /*= 0*/,
                 const char* name /*= 0*/ ): QDragObject(dragSource, name) {
    data = new UMLDragPrivate;
    setUMLDataClip3(umlListViewItems);
}

UMLDrag::UMLDrag(UMLObjectList& objects,
                 UMLWidgetList& widgets, AssociationWidgetList& associationDatas,
                 QPixmap& pngImage, Uml::Diagram_Type dType, QWidget * dragSource /*= 0*/,
                 const char * name /*= 0*/ ): QDragObject(dragSource, name) {
    data = new UMLDragPrivate;
    setUMLDataClip4(objects, widgets, associationDatas, pngImage, dType);
}

UMLDrag::UMLDrag(UMLObjectList& objects, int,
                 QWidget* /*dragSource = 0*/, const char* /*name = 0*/ ) {
    data = new UMLDragPrivate;
    setUMLDataClip5(objects);
}

UMLDrag::UMLDrag(QWidget* dragSource /*= 0*/, const char * name /*= 0*/ ): QDragObject(dragSource, name) {
    data = new UMLDragPrivate;
}

UMLDrag::~UMLDrag() {
    delete data;
}

void UMLDrag::setSubType(const QCString& string, int index) {
    data->setSubType(string, index);
}

void UMLDrag::setEncodedData(const QByteArray& encodedData, int index) {
    data->enc[index] = encodedData.copy();
}

QByteArray UMLDrag::encodedData(const char* dataName) const {
    QString str(dataName);
    for (int i = 0; i < 4; i++) {
        if ( !qstricmp(dataName,data->fmt[i]) ) {
            return data->enc[i];
        }
    }
    return QByteArray();
}

const char* UMLDrag::format(int index) const {
    char* result = (char*)data->format(index);
    return result;
}

void UMLDrag::setUMLDataClip1(UMLObjectList& objects) {
    setSubType("clip1", 0);

    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement("umlobjects");
    xmiclip.appendChild(objectsTag);

    UMLObjectListIt object_it(objects);
    UMLObject* obj = 0;
    while ( (obj=object_it.current()) != 0 ) {
        ++object_it;
        obj->saveToXMI(domDoc, objectsTag);
    }

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    setEncodedData(domDoc.toString().utf8(), 0);
}

void UMLDrag::setUMLDataClip2(UMLObjectList& objects, UMLListViewItemList& umlListViewItems,
                              UMLViewList& diagrams) {
    setSubType("clip2", 0);

    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement("umlobjects");
    xmiclip.appendChild(objectsTag);

    UMLObjectListIt object_it(objects);
    UMLObject* obj = 0;
    while ( (obj=object_it.current()) != 0 ) {
        ++object_it;
        obj->saveToXMI(domDoc, objectsTag);
    }

    QDomElement viewsTag = domDoc.createElement("umlviews");
    xmiclip.appendChild(viewsTag);

    UMLViewListIt diagram_it(diagrams);
    UMLView* view = 0;
    while ( (view=diagram_it.current()) != 0 ) {
        ++diagram_it;
        view->saveToXMI(domDoc, viewsTag);
    }

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    UMLListViewItemListIt item_it2(umlListViewItems);
    UMLListViewItem* item = 0;
    while ( (item=item_it2.current()) != 0 ) {
        ++item_it2;
        item->saveToXMI(domDoc, itemsTag);
    }
    setEncodedData(domDoc.toString().utf8(), 0);
}

void UMLDrag::setUMLDataClip3(UMLListViewItemList& umlListViewItems) {
    setSubType("clip3", 0);

    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    domDoc.appendChild(xmiclip);

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    UMLListViewItemListIt item_it2(umlListViewItems);
    UMLListViewItem* item = 0;
    while ( (item=item_it2.current()) != 0 ) {
        ++item_it2;
        item->saveToXMI(domDoc, itemsTag);
    }
    setEncodedData(domDoc.toString().utf8(), 0);
}

void UMLDrag::setUMLDataClip4(UMLObjectList& objects, UMLWidgetList& widgets, AssociationWidgetList& associations,
                              QPixmap& pngImage, Uml::Diagram_Type dType ) {
    setSubType("clip4", 0);

    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    xmiclip.setAttribute("diagramtype", dType);
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement("umlobjects");
    xmiclip.appendChild(objectsTag);

    UMLObjectListIt object_it(objects);
    UMLObject* obj = 0;
    while ( (obj=object_it.current()) != 0 ) {
        ++object_it;
        obj->saveToXMI(domDoc, objectsTag);
    }

    QDomElement widgetsTag = domDoc.createElement("widgets");
    xmiclip.appendChild(widgetsTag);

    UMLWidgetListIt widget_it(widgets);
    UMLWidget* widget = 0;
    while ( (widget=widget_it.current()) != 0 ) {
        ++widget_it;
        widget->saveToXMI(domDoc, widgetsTag);
    }

    QDomElement associationWidgetsTag = domDoc.createElement("associations");
    xmiclip.appendChild(associationWidgetsTag);

    AssociationWidgetListIt associations_it(associations);
    AssociationWidget* association;
    while ( (association=associations_it.current()) != 0 ) {
        ++associations_it;
        association->saveToXMI(domDoc, associationWidgetsTag);
    }

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    setEncodedData(domDoc.toString().utf8(), 0);

    data->setType("image/PNG", 1);
    long l_size = (pngImage.convertToImage()).numBytes();
    QByteArray clipdata;
    clipdata.resize(l_size);
    QDataStream clipstream(clipdata, IO_WriteOnly);
    clipstream << pngImage;
    setEncodedData(clipdata, 1);
}

void UMLDrag::setUMLDataClip5(UMLObjectList& objects) {
    setSubType("clip5", 0);

    QDomDocument domDoc;
    QDomElement xmiclip = domDoc.createElement("xmiclip");
    domDoc.appendChild(xmiclip);
    QDomElement objectsTag = domDoc.createElement("umlobjects");
    xmiclip.appendChild(objectsTag);

    UMLObjectListIt object_it(objects);
    UMLObject* obj = 0;
    while ( (obj=object_it.current()) != 0 ) {
        ++object_it;
        obj->saveToXMI(domDoc, objectsTag);
    }

    QDomElement itemsTag = domDoc.createElement("umllistviewitems");
    xmiclip.appendChild(itemsTag);

    setEncodedData(domDoc.toString().utf8(), 0);
}

bool UMLDrag::decodeClip1(const QMimeSource* mimeSource, UMLObjectList& objects) {
    UMLDoc* doc = UMLApp::app()->getDocument();
    if ( !mimeSource->provides("application/x-uml-clip1") ) {
        return false;
    }
    QByteArray payload = mimeSource->encodedData("application/x-uml-clip1");
    if ( !payload.size() ) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        kWarning() << "Can't set content:" << error << " Line:" << line << endl;
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

    UMLListView *listView = UMLApp::app()->getListView();

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
            kWarning() << "UMLDrag::decodeClip1: Given wrong type of umlobject to create: "
            << type << endl;
            return false;
        }
        pObject->setInPaste( true );
        if( !pObject->loadFromXMI( element ) ) {
            kWarning() << "UMLDrag::decodeClip1: failed to load object of type "
            << type << " from XMI" << endl;
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
            Uml::Object_Type type = pObject->getBaseType();
            QString newName = Model_Utils::uniqObjectName(type, pObject->getUMLPackage(),
                                                                pObject->getName());
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

bool UMLDrag::decodeClip2(const QMimeSource* mimeSource, UMLObjectList& objects,
                          UMLListViewItemList& umlListViewItems, UMLViewList& diagrams) {

    if ( !mimeSource->provides("application/x-uml-clip2") ) {
        return false;
    }
    QByteArray payload = mimeSource->encodedData("application/x-uml-clip2");
    if ( !payload.size() ) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        kWarning() << "Can't set content:" << error << " Line:" << line << endl;
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
                kWarning() << "Given wrong type of umlobject to create:" << type << endl;
                return false;
            }
            if( !pObject->loadFromXMI(element) ) {
                kWarning() << "failed to load object from XMI" << endl;
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
        kWarning() << "no diagrams in XMI clip" << endl;
        return false;
    }
    UMLListView *listView = UMLApp::app()->getListView();
    while ( !diagramElement.isNull() ) {
        QString type = diagramElement.attribute("type", "0");
        Uml::Diagram_Type dt = (Uml::Diagram_Type)type.toInt();
        UMLListViewItem *parent = listView->findFolderForDiagram(dt);
        if (parent == NULL)
            return false;
        UMLObject *po = parent->getUMLObject();
        if (po == NULL || po->getBaseType() != Uml::ot_Folder) {
            kError() << "UMLDrag::decodeClip2: bad parent for view" << endl;
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
        kWarning() << "no listitems in XMI clip" << endl;
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

bool UMLDrag::getClip3TypeAndID(const QMimeSource* mimeSource,
                                LvTypeAndID_List& typeAndIdList)
{
    if ( !mimeSource->provides("application/x-uml-clip3") ) {
        return false;
    }
    QByteArray payload = mimeSource->encodedData("application/x-uml-clip3");
    if ( !payload.size() ) {
        return false;
    }
    QTextStream clipdata(payload, IO_ReadOnly);
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        kWarning() << "getClip3Type: Can't set content:" << error << " Line:" << line << endl;
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
        kWarning() << "getClip3Type: no listitems in XMI clip" << endl;
        return false;
    }
    while ( !listItemElement.isNull() ) {
        QString typeStr = listItemElement.attribute( "type", "-1" );
        if (typeStr == "-1") {
            kDebug() << "getClip3Type: bad type" << endl;
            return false;
        }
        QString idStr = listItemElement.attribute( "id", "-1" );
        if (idStr == "-1") {
            kDebug() << "getClip3Type: bad id" << endl;
            return false;
        }
        LvTypeAndID * pData = new LvTypeAndID;
        pData->type = (Uml::ListView_Type)(typeStr.toInt());
        pData->id = STR2ID(idStr);
        typeAndIdList.append(pData);
        listItems = listItems.nextSibling();
        listItemElement = listItems.toElement();
    }
    return true;
}

bool UMLDrag::decodeClip3(const QMimeSource* mimeSource, UMLListViewItemList& umlListViewItems,
                            const UMLListView* parentListView){
    if ( !mimeSource->provides("application/x-uml-clip3") ) {
        return false;
    }
    QByteArray payload = mimeSource->encodedData("application/x-uml-clip3");
    if ( !payload.size() ) {
        return false;
    }
    QTextStream clipdata(payload, IO_ReadOnly);
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        kWarning() << "Can't set content:" << error << " Line:" << line << endl;
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
        kWarning() << "no listitems in XMI clip" << endl;
        return false;
    }
    while ( !listItemElement.isNull() ) {
        // Get the ListView_Type beforehand so that we can construct an
        // UMLListViewItem instance.
        QString type = listItemElement.attribute( "type", "-1" );
        if (type == "-1") {
            kDebug() << "Pech gehabt" << endl;
            continue;
        }
        Uml::ListView_Type t = (Uml::ListView_Type)(type.toInt());
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

bool UMLDrag::decodeClip4(const QMimeSource* mimeSource, UMLObjectList& objects,
                          UMLWidgetList& widgets,
                          AssociationWidgetList& associations, Uml::Diagram_Type & dType) {
    if ( !mimeSource->provides("application/x-uml-clip4") ) {
        return false;
    }
    QByteArray payload = mimeSource->encodedData("application/x-uml-clip4");
    if ( !payload.size() ) {
        return false;
    }

    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        kWarning() << "Can't set content:" << error << " Line:" << line << endl;
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

    dType = (Uml::Diagram_Type)(root.attribute("diagramtype", "0").toInt());

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
            kWarning() << "Given wrong type of umlobject to create: " << type << endl;
            return false;
        }

        if ( !pObject->loadFromXMI( element ) ) {
            kWarning() << "failed to load object from XMI" << endl;
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
        kWarning() << "no widgets in XMI clip" << endl;
        return false;
    }

    UMLView *view = UMLApp::app()->getCurrentView();
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
        AssociationWidget* associationWidget = new AssociationWidget(view);
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

bool UMLDrag::decodeClip5(const QMimeSource* mimeSource, UMLObjectList& objects,
                          UMLClassifier* newParent) {
    if ( !mimeSource->provides("application/x-uml-clip5") ) {
        return false;
    }
    QByteArray payload = mimeSource->encodedData("application/x-uml-clip5");
    if ( !payload.size() ) {
        return false;
    }
    QString xmiClip = QString::fromUtf8(payload);

    QString error;
    int line;
    QDomDocument domDoc;
    if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
        kWarning() << "Can't set content:" << error << " Line:" << line << endl;
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
            kWarning() << "Given wrong type of umlobject to create:" << type << endl;
            return false;
        }
        if( !pObject->loadFromXMI( element ) ) {
            kWarning() << "failed to load object from XMI" << endl;
            return false;
        }
        pObject->resolveRef();
        objects.append(pObject);
        objectElement = objectElement.nextSibling();
        element = objectElement.toElement();
    }

    return true;
}

int UMLDrag::getCodingType(const QMimeSource* mimeSource) {
    int result = 0;
    if (mimeSource->provides("application/x-uml-clip1") ) {
        result = 1;
    }
    if (mimeSource->provides("application/x-uml-clip2") ) {
        result = 2;
    }
    if (mimeSource->provides("application/x-uml-clip3") ) {
        result = 3;
    }
    if (mimeSource->provides("application/x-uml-clip4") ) {
        result = 4;
    }
    if (mimeSource->provides("application/x-uml-clip5") ) {
        result = 5;
    }

    return result;
}

#include "umldrag.moc"
