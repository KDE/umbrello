/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLDRAG_H
#define UMLDRAG_H


#include <qdragobject.h>
#include <qobject.h>

#include "../umllistviewitemdatalist.h"
#include "../associationwidgetdatalist.h"
#include "../umlobjectlist.h"
#include "../umlviewdatalist.h"
#include "../umlwidgetdatalist.h"
#include "../umlnamespace.h"

/**
 *	This class provides encoding and decoding for the uml data
 *	that will be use in a drag and drop operation or in a copy or paste
 *	operation.
 *
 *	@author Gustavo Madrigal, Jonathan Riddell (XMI conversion)
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLListView;
class UMLDoc;

class UMLDragPrivate;

class Q_EXPORT UMLDrag : public QDragObject  {
	Q_OBJECT
	UMLDragPrivate* data;
public:

	/**
	 *	For use when the user selects only UML Objects from
	 *	the ListView but no diagrams to be copied, Mime type =
	 *	"application/x-uml-clip1
	 */
	UMLDrag(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
		 QWidget* dragSource = 0, const char* name = 0 );

	/**
	 *	For use when the user selects UML Object and Diagrams
	 *	from the ListView to be copied, Mime type =
	 *	"application/x-uml-clip2
	 */
	UMLDrag(UMLObjectList &Objects, UMLListViewItemDataList& UMLListViewItems,
		 UMLViewDataList& Diagrams, QWidget * dragSource = 0, const char * name = 0 );

	/**
	 *	For use when the user selects only empty folders from
	 *	the ListView to be copied, Mime type =
	 *	"application/x-uml-clip3
	 */
	UMLDrag(UMLListViewItemDataList& UMLListViewItems, QWidget* dragSource = 0,
		const char* name = 0 );

	/*
	 *	For use when the user selects UMLObjects from a
	 *	Diagram. The Selected widegets and the relationships *
	 *	between only selected widgets will be copied and also
	 *	its respective ListView Items, Mime type =
	 *	"application/x-uml-clip4
	 */
	UMLDrag(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
		UMLWidgetDataList& WidgetDatas, AssociationWidgetDataList& AssociationDatas,
		QPixmap& PngImage, Uml::Diagram_Type dType, QWidget* dragSource = 0,
		const char* name = 0 );

	/**
	 *	For use when the user selects only Operations and/or
	 *	Attributes from the ListView, Mime type =
	 *	"application/x-uml-clip5
	 */
	UMLDrag(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
		int, QWidget* dragSource = 0, const char* name = 0);

	/**
	 * 	Constructor
	 */
	UMLDrag(QWidget* dragSource = 0, const char* name = 0);

	/**
	 * 	Deconstructor
	 */
	~UMLDrag();

	/**
	 *	For use when the user selects only UMLObjects from the
	 *	ListView but no diagrams to be copied
	 */
	void setUMLDataClip1(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems);

	/**
	 *	For use when the user selects UML Object and Diagrams
	 *	from the ListView to be copied
	 */
	void setUMLDataClip2(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
			     UMLViewDataList& Diagrams);

	/**
	 *		For use when the user selects only empty
	 *		folders from the ListView to be copied.
	 */
	void setUMLDataClip3(UMLListViewItemDataList& UMLListViewItems);

	/**
	 *	For use when the user selects UML Objects from a
	 *	Diagram. The Selected widegets and the relationships
	 *	between only selected widgets will be copied and also
	 *	its respective ListView Items
	 */
	void setUMLDataClip4(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
			     UMLWidgetDataList& WidgetDatas,
			     AssociationWidgetDataList& AssociationDatas, QPixmap& PngImage,
			     Uml::Diagram_Type dType);

	/**
	 *	For use when the user selects only Attirbutes and/or
	 *	Operation from the ListView
	 */
	void setUMLDataClip5(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems);

	/**
	 * 	Returns whether it can decode the given mimesource
	 */
	static bool canDecode(const QMimeSource* mimeSource);

	/**
	 *	Sets the type of the clip to "application/x-uml-" + sub
	 *	sub should be clip[1-5]
	 */
	virtual void setSubType(const QCString& sub, int index);

	/**
	 *	Sets the data in the clip
	 */
	virtual void setEncodedData(const QByteArray&, int index);

	/**
	 *	Returns the type set by setSubType
	 */
	const char* format(int index) const;

	/**
	 * Returns the encoded data of the given type
	 *
	 * @param dataName the name of the data type to return
	 */
	virtual QByteArray encodedData(const char* dataName) const;

	/**
	 *	For use when the user selects only UML Objects
	 *	from the ListView but no diagrams to be
	 *	copied, decodes Mime type =
	 *	"application/x-uml-clip1
	 */
	static bool decodeClip1(const QMimeSource* mimeSource, UMLObjectList& objects,
				UMLListViewItemDataList& umlListViewItems, UMLDoc* doc);

	/**
	 *	For use when the user selects UML Object and Diagrams
	 *	from the ListView to be copied, decodes Mime type =
	 *	"application/x-uml-clip2
	 */
	static bool decodeClip2(const QMimeSource* mimeSource, UMLObjectList& objects,
				UMLListViewItemDataList& umlListViewItems,
				UMLViewDataList& diagrams, UMLDoc* doc);

	/**
	 *	For use when the user selects UMLObjects from
	 *	the ListView to be copied, decodes Mime * type =
	 *	"application/x-uml-clip3
	 */
	static bool decodeClip3(const QMimeSource* mimeSource, UMLListViewItemDataList& umlListViewItems);

	/**
	 *	For use when the user selects UML Objects from a
	 *	Diagram. The Selected widegets and the relationships
	 *	between only * selected widgets will be copied and
	 *	also its respective ListView Items, * decodes Mime
	 *	type = "application/x-uml-clip4
	 */
	static bool decodeClip4(const QMimeSource* mimeSource, UMLObjectList& objects,
				UMLListViewItemDataList& umlListViewItems,
				UMLWidgetDataList& widgetDatas,
				AssociationWidgetDataList& associationDatas,
				Uml::Diagram_Type & dType, UMLDoc* doc);

	/**
	 *	For use when the user selects only Attributes and/or
	 *	Operations from the ListView * copied, decodes Mime
	 *	type = "application/x-uml-clip5
	 */
	static bool decodeClip5(const QMimeSource* mimeSource, UMLObjectList& objects,
				UMLListViewItemDataList& umlListViewItems, UMLDoc* doc);

	/**
	 *	Converts application/x-uml-clip[1-5] clip type to an integer
	 */
	static int getCodingType(const QMimeSource* mimeSource);

};

#endif
