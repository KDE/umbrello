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
#include "../umlviewlist.h"
#include "../umlwidgetlist.h"
#include "../umlnamespace.h"

/**
 *	This class provides encoding and decoding for the uml data
 *	that will be use in a drag_n_drop operation or in a copy paste
 *	operation
 *
 *	@author Gustavo Madrigal
 */

class UMLListView;
class UMLDoc;

class UMLDragPrivate;


class Q_EXPORT UMLDrag : public QDragObject  {
	Q_OBJECT
	UMLDragPrivate *d;
public:

	/**
	*		For use when the user selects only UML Objects
	*		from the ListView but no diagrams to be
	*		copied, Mime type = "application/x-uml-clip1
	*/
	UMLDrag(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
		 QWidget * dragSource = 0, const char * name = 0 );

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
	UMLDrag(UMLListViewItemDataList& UMLListViewItems, QWidget * dragSource = 0,
		 const char * name = 0 );

	/*
	 *	For use when the user selects UMLObjects from a
	 *	Diagram. The Selected widegets and the relationships *
	 *	between only selected widgets will be copied and also
	 *	its respective ListView Items, Mime type =
	 *	"application/x-uml-clip4
	 */
	UMLDrag(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
		 UMLWidgetDataList& WidgetDatas, AssociationWidgetDataList& AssociationDatas,
		 QPixmap& PngImage, Uml::Diagram_Type dType, QWidget * dragSource = 0,
		 const char * name = 0 );

	/**
	 *	For use when the user selects only Operations and/or
	 *	Attributes from the ListView, Mime type =
	 *	"application/x-uml-clip5
	 */
	UMLDrag(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
		 int, QWidget * dragSource = 0, const char * name = 0 );

	/**
	 * 	Constructor
	 */
	UMLDrag( QWidget * dragSource = 0, const char * name = 0 );

	/**
	 * 	Deconstructor
	 */
	~UMLDrag();

	/**
	 *	For use when the user selects only UMLObjects from the
	 *	ListView but no diagrams to be copied
	 */
	void setUMLData(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems);

	/**
	 *	For use when the user selects UML Object and Diagrams
	 *	from the ListView to be copied
	 */
	void setUMLData(UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
			 UMLViewDataList& Diagrams);

	/**
	 *		For use when the user selects only empty
	 *		folders from the ListView to be copied.
	 */
	void setUMLData(UMLListViewItemDataList& UMLListViewItems);

	/**
	 *	For use when the user selects UML Objects from a
	 *	Diagram. The Selected widegets and the relationships
	 *	between only selected widgets will be copied and also
	 *	its respective ListView Items
	 */
	void setUMLData( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
			 UMLWidgetDataList& WidgetDatas,
			 AssociationWidgetDataList& AssociationDatas, QPixmap& PngImage,
			 Uml::Diagram_Type dType );

	/**
	 *	For use when the user selects only Attirbutes and/or
	 *	Operation from the ListView
	 */
	void setUMLData( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems, int );

	/**
	 * 		Returns whether it can decode the given mimesource
	 */
	static bool canDecode( const QMimeSource* e );

	/**
	 *
	 */
	virtual void setSubType( const QCString & sub, int index);

	/**
	 *
	 */
	virtual void setEncodedData( const QByteArray &, int index );

	/**
	 *
	 */
	const char * format(int i) const;

	/**
	 *
	 */
	virtual QByteArray encodedData(const char*) const;

	/**
	 *	For use when the user selects only UML Objects
	 *	from the ListView but no diagrams to be
	 *	copied, decodes Mime type =
	 *	"application/x-uml-clip1
	 */
	static bool decode(const QMimeSource* e, UMLObjectList& Objects,
			   UMLListViewItemDataList& UMLListViewItems, UMLDoc* Doc);

	/**
	 *	For use when the user selects UML Object and Diagrams
	 *	from the ListView to be copied, decodes Mime type =
	 *	"application/x-uml-clip2
	 */
	static bool decode(const QMimeSource* e, UMLObjectList& Objects,
			   UMLListViewItemDataList& UMLListViewItems,
	                   UMLViewDataList& Diagrams, UMLDoc* Doc);

	/**
	 *		For use when the user selects only empty
	 *		folders from the ListView to be copied,
	 *		decodes Mime * type = "application/x-uml-clip3
	 */
	static bool decode( const QMimeSource* e, UMLListViewItemDataList& UMLListViewItems);

	/**
	 *		For use when the user selects UML Objects from
	 *		a Diagram. The Selected widegets and the
	 *		relationships between only * selected widgets
	 *		will be copied and also its respective
	 *		ListView Items, * decodes Mime type =
	 *		"application/x-uml-clip4
	 */
	static bool decode(const QMimeSource* e, UMLObjectList& Objects,
			   UMLListViewItemDataList& UMLListViewItems,
	                   UMLWidgetDataList& WidgetDatas,
			   AssociationWidgetDataList& AssociationDatas,
			   Uml::Diagram_Type & dType, UMLDoc* Doc );

	/**
	 *	For use when the user selects only Attributes and/or
	 *	Operations from the ListView * copied, decodes Mime
	 *	type = "application/x-uml-clip5
	 */
	static bool decode(const QMimeSource* e, UMLObjectList& Objects,
			   UMLListViewItemDataList& UMLListViewItems, UMLDoc* Doc, int);

	/**
	 *
	 */
	static int getCodingType(const QMimeSource* e);
};

#endif
