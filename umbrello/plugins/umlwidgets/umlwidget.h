 /***************************************************************************
                               umlwidget.h
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
 
#ifndef UMBRELLO_UMLWIDGET_H
#define UMBRELLO_UMLWIDGET_H

#include "../../diagram/diagramwidget.h"
#include <qptrlist.h>

class UMLObject;

namespace Umbrello{
class Diagram;

/** Base class for all widgets that represent some kind of UML Object */
class UMLWidget : public DiagramWidget
{
Q_OBJECT

public:
	/** Constructor
	* @param diagram The diagram this widget should go in
	* @param id The unique ID of the widget
	* @object   The UML Object the widget represents
	*/
	UMLWidget( Diagram *diagram, uint id, UMLObject *object);
	/** Destructor */
	virtual ~UMLWidget();
	/** Return the UML Object the widget represents */
	inline UMLObject* umlObject( ) const;
	
	/** By default all UMLWidgets' default action is to call editProperties.
	* Subclasses can override  this behavior*/
	virtual void execDefaultAction();
	

public slots:
	/** Inform the widget that the UML Object it is representing has been modified, so that the widget
	* can update itself */
	virtual void umlObjectModified();
	
protected:
	/** (Re)implement this function to draw the widget */
	virtual void drawShape(QPainter &) = 0;
	
	
	UMLObject *m_umlObject;
};

//inline functions
UMLObject* UMLWidget::umlObject() const { return m_umlObject;}


} //end of namespace Umbrello

#endif  // UMBRELLO_UMLWIDGET_H


