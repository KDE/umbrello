 /***************************************************************************
                               unknownumlwidget.h
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef UMBRELLO_UNKNOWNUMLWIDGET_H
#define UMBRELLO_UNKNOWNUMLWIDGET_H

#include "diagramwidget.h"
#include <qptrlist.h>

class UMLObject;

namespace Umbrello{
class Diagram;

/** Widget that represents a UML::Element of unknown type -
  * You will normally never see a widget of this kind, unless the "real" widgets are not fully
  * implemented
  */
class UnknownUMLWidget : public DiagramWidget
{
Q_OBJECT
public:
	/** Constructor
	* @param diagram The diagram this widget should go in
	* @param id The unique ID of the widget
	* @object   The UML Object the widget represents
	*/
	UnknownUMLWidget( Diagram *diagram, uint id, UMLObject *object);
	/** Destructor */
	virtual ~UnknownUMLWidget();
	
public slots:
	/** Inform the widget that the UML Object it is representing has been modified, so that the widget
	* can update itself */
	virtual void umlObjectModified();
	
	/** Show a dialog to edit the properties of the element */
	virtual void editProperties();
	
protected:
	virtual void drawShape(QPainter &);
	void calculateSize();
	
	QString m_name;
	UMLObject *m_umlObject;
};


} //end of namespace Umbrello

#endif  // UMBRELLO_UNKNOWNUMLWIDGET_H


