/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kdebug.h>
#include <qdom.h>

#include "umldrag.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../umlview.h"
#include "../umlobject.h"
#include "../umlwidget.h"
#include "../umllistviewitem.h"
#include "../associationwidget.h"

class UMLDragPrivate {
public:
	static const int nfmt = 4;
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


UMLDrag::UMLDrag(UMLObjectList& objects, UMLListViewItemList& umlListViewItems,
		 QWidget* dragSource /*= 0*/, const char* name /*= 0*/ ): QDragObject(dragSource, name) {
	data = new UMLDragPrivate;
	setUMLDataClip1(objects, umlListViewItems);
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

UMLDrag::UMLDrag(UMLObjectList& objects, UMLListViewItemList& umlListViewItems,
		 UMLWidgetList& widgets, AssociationWidgetList& associationDatas,
		 QPixmap& pngImage, Uml::Diagram_Type dType, QWidget * dragSource /*= 0*/,
		 const char * name /*= 0*/ ): QDragObject(dragSource, name) {
	data = new UMLDragPrivate;
	setUMLDataClip4(objects, umlListViewItems, widgets, associationDatas, pngImage, dType);
}

UMLDrag::UMLDrag(UMLObjectList& objects, UMLListViewItemList& umlListViewItems, int,
		 QWidget* /*dragSource = 0*/, const char* /*name = 0*/ ) {
	data = new UMLDragPrivate;
	setUMLDataClip5(objects, umlListViewItems);
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

void UMLDrag::setUMLDataClip1(UMLObjectList& objects, UMLListViewItemList& umlListViewItems) {
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

	UMLListViewItemListIt item_it2(umlListViewItems);
	UMLListViewItem* item = 0;
	while ( (item=item_it2.current()) != 0 ) {
		++item_it2;
		item->saveToXMI(domDoc, itemsTag);
	}
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

void UMLDrag::setUMLDataClip4(UMLObjectList& objects, UMLListViewItemList& umlListViewItems,
			      UMLWidgetList& widgets, AssociationWidgetList& associations,
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

	UMLListViewItemListIt item_it2(umlListViewItems);
	UMLListViewItem* item = 0;
	while ( (item=item_it2.current()) != 0 ) {
		++item_it2;
		item->saveToXMI(domDoc, itemsTag);
	}
	setEncodedData(domDoc.toString().utf8(), 0);

	data->setType("image/PNG", 1);
	long l_size = (pngImage.convertToImage()).numBytes();
	char* data = new char[l_size];
	QByteArray clipdata;
	clipdata.setRawData(data, l_size);
	QDataStream clipstream(clipdata, IO_WriteOnly);
	clipstream << pngImage;
	setEncodedData(clipdata, 1);
}

void UMLDrag::setUMLDataClip5(UMLObjectList& objects, UMLListViewItemList& umlListViewItems) {
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

	UMLListViewItemListIt item_it2(umlListViewItems);
	UMLListViewItem* item = 0;
	while ( (item=item_it2.current()) != 0 ) {
		++item_it2;
		item->saveToXMI(domDoc, itemsTag);
	}
	setEncodedData(domDoc.toString().utf8(), 0);
}

bool UMLDrag::canDecode(const QMimeSource* mimeSource) {
	const char* f;
	for (int i=0; (f=mimeSource->format(i)); i++) {
		if ( 0==qstrnicmp(f,"application/x-uml-clip", 22) ) {
			//FIXME need to test for clip1, clip2, clip3, clip4 or clip5
			//(the only valid clip types)
			return true;
		}
	}
	return false;
}

bool UMLDrag::decodeClip1(const QMimeSource* mimeSource, UMLObjectList& objects,
			  UMLListViewItemList& umlListViewItems, UMLDoc* doc) {
	if ( !mimeSource->provides("application/x-uml-clip1") ) {
		return false;
	}
	QByteArray payload = mimeSource->encodedData("application/x-uml-clip1");
	if ( !payload.size() ) {
		return false;
	}
	QString xmiClip(payload);

	QString error;
	int line;
	QDomDocument domDoc;
	if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
		kdWarning() << "Can't set content:" << error << " Line:" << line << endl;
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
			pObject = doc->makeNewUMLObject(type);

			if( !pObject ) {
				kdWarning() << "Given wrong type of umlobject to create:" << type << endl;
				return false;
			}
			if( !pObject->loadFromXMI( element ) ) {
				kdWarning() << "failed to load object from XMI" << endl;
				return false;
			}
			objects.append(pObject);
		}
		objectElement = objectElement.nextSibling();
		element = objectElement.toElement();
	}

	//listviewitems
	QDomNode listItemNode = objectsNode.nextSibling();
	QDomNode listItems = listItemNode.firstChild();
	QDomElement listItemElement = listItems.toElement();
	if ( listItemElement.isNull() ) {
		kdWarning() << "no listitems in XMI clip" << endl;
		return false;
	}
	while ( !listItemElement.isNull() ) {
		UMLListViewItem* itemData = new UMLListViewItem( doc->getListView() );
		itemData->loadFromXMI(listItemElement);
		umlListViewItems.append(itemData);
		listItems = listItems.nextSibling();
		listItemElement = listItems.toElement();
	}
	return true;
}

bool UMLDrag::decodeClip2(const QMimeSource* mimeSource, UMLObjectList& objects,
			  UMLListViewItemList& umlListViewItems, UMLViewList& diagrams, UMLDoc* doc) {
	if ( !mimeSource->provides("application/x-uml-clip2") ) {
		return false;
	}
	QByteArray payload = mimeSource->encodedData("application/x-uml-clip2");
	if ( !payload.size() ) {
		return false;
	}
	QString xmiClip(payload);

	QString error;
	int line;
	QDomDocument domDoc;
	if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
		kdWarning() << "Can't set content:" << error << " Line:" << line << endl;
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
			pObject = doc->makeNewUMLObject(type);

			if( !pObject ) {
				kdWarning() << "Given wrong type of umlobject to create:" << type << endl;
				return false;
			}
			if( !pObject->loadFromXMI(element) ) {
				kdWarning() << "failed to load object from XMI" << endl;
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
		kdWarning() << "no diagrams in XMI clip" << endl;
		return false;
	}
	while ( !diagramElement.isNull() ) {
		UMLView* view = new UMLView(UMLApp::app()->getMainViewWidget(), doc);
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
		kdWarning() << "no listitems in XMI clip" << endl;
		return false;
	}
	while ( !listItemElement.isNull() ) {
		UMLListViewItem* itemData = new UMLListViewItem( doc->getListView() );
		itemData->loadFromXMI(listItemElement);
		umlListViewItems.append(itemData);
		listItems = listItems.nextSibling();
		listItemElement = listItems.toElement();
	}
	return true;
}

bool UMLDrag::decodeClip3(const QMimeSource* mimeSource, UMLListViewItemList& umlListViewItems) {
	if ( !mimeSource->provides("application/x-uml-clip3") ) {
		return false;
	}
	QByteArray payload = mimeSource->encodedData("application/x-uml-clip3");
	if ( !payload.size() ) {
		return false;
	}
	QTextStream clipdata(payload, IO_ReadOnly);
	QString xmiClip(payload);

	QString error;
	int line;
	QDomDocument domDoc;
	if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
		kdWarning() << "Can't set content:" << error << " Line:" << line << endl;
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
		kdWarning() << "no listitems in XMI clip" << endl;
		return false;
	}
	while ( !listItemElement.isNull() ) {
		UMLListViewItem* itemData = new UMLListViewItem(NULL);
		itemData->loadFromXMI(listItemElement);
		umlListViewItems.append(itemData);
		listItems = listItems.nextSibling();
		listItemElement = listItems.toElement();
	}
	return true;
}

bool UMLDrag::decodeClip4(const QMimeSource* mimeSource, UMLObjectList& objects,
			  UMLListViewItemList& umlListViewItems, UMLWidgetList& widgets,
			  AssociationWidgetList& associations, Uml::Diagram_Type & dType,
			  UMLDoc* doc) {
	if ( !mimeSource->provides("application/x-uml-clip4") ) {
		return false;
	}
	QByteArray payload = mimeSource->encodedData("application/x-uml-clip4");
	if ( !payload.size() ) {
		return false;
	}
	QString xmiClip(payload);

	QString error;
	int line;
	QDomDocument domDoc;
	if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
		kdWarning() << "Can't set content:" << error << " Line:" << line << endl;
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
		pObject = doc->makeNewUMLObject(type);

		if ( !pObject ) {
			kdWarning() << "Given wrong type of umlobject to create: " << type << endl;
			return false;
		}
		if ( !pObject->loadFromXMI( element ) ) {
			kdWarning() << "failed to load object from XMI" << endl;
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
		kdWarning() << "no widgets in XMI clip" << endl;
		return false;
	}
	while ( !widgetElement.isNull() ) {
		UMLWidget* widget = doc->getCurrentView()->loadWidgetFromXMI(widgetElement);
		widgets.append(widget);
		widgetNode = widgetNode.nextSibling();
		widgetElement = widgetNode.toElement();
	}

	//AssociationWidgets
	QDomNode associationWidgetsNode = widgetsNode.nextSibling();
	QDomNode associationWidgetNode = associationWidgetsNode.firstChild();
	QDomElement associationWidgetElement = associationWidgetNode.toElement();
	while ( !associationWidgetElement.isNull() ) {
		AssociationWidget* associationWidget = new AssociationWidget(doc->getCurrentView());
		associationWidget->loadFromXMI(associationWidgetElement);
		associations.append(associationWidget);
		associationWidgetNode = associationWidgetNode.nextSibling();
		associationWidgetElement = associationWidgetNode.toElement();
	}

	//listviewitems
	QDomNode listItemNode = associationWidgetsNode.nextSibling();
	QDomNode listItems = listItemNode.firstChild();
	QDomElement listItemElement = listItems.toElement();
	while ( !listItemElement.isNull() ) {
		UMLListViewItem* itemData = new UMLListViewItem( doc->getListView() );
		itemData->loadFromXMI(listItemElement);
		umlListViewItems.append(itemData);
		listItems = listItems.nextSibling();
		listItemElement = listItems.toElement();
	}
	return true;
}

//FIXME this needs fixing
bool UMLDrag::decodeClip5(const QMimeSource* mimeSource, UMLObjectList& objects,
			  UMLListViewItemList& umlListViewItems, UMLDoc* doc) {
kdDebug()<<"DECODE CLIP 5 CALLED"<<endl;
	if ( !mimeSource->provides("application/x-uml-clip5") ) {
		return false;
	}
	QByteArray payload = mimeSource->encodedData("application/x-uml-clip5");
	if ( !payload.size() ) {
		return false;
	}
	QString xmiClip(payload);

	QString error;
	int line;
	QDomDocument domDoc;
	if( !domDoc.setContent(xmiClip, false, &error, &line) ) {
		kdWarning() << "Can't set content:" << error << " Line:" << line << endl;
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
	UMLObject* pObject = 0;
/*
	while ( !element.isNull() ) {
		pObject = 0;
		QString type = element.tagName();
		pObject = doc->makeNewClassifierObject(type, element);

		if( !pObject ) {
			kdWarning() << "Given wrong type of umlobject to create:" << type << endl;
			return false;
		}
		if( !pObject->loadFromXMI( element ) ) {
			kdWarning() << "failed to load object from XMI" << endl;
			return false;
		}
		objects.append(pObject);
		objectElement = objectElement.nextSibling();
		element = objectElement.toElement();
	}
*/

	//listviewitems
	QDomNode listItemNode = objectsNode.nextSibling();
	QDomNode listItems = listItemNode.firstChild();
	QDomElement listItemElement = listItems.toElement();
	if ( listItemElement.isNull() ) {
		kdWarning() << "no listitems in XMI clip" << endl;
		return false;
	}
	while ( !listItemElement.isNull() ) {
		UMLListViewItem* itemData = new UMLListViewItem( doc->getListView() );
		itemData->loadFromXMI(listItemElement);
		umlListViewItems.append(itemData);
		listItems = listItems.nextSibling();
		listItemElement = listItems.toElement();
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
