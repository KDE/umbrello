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

#include "diagramelement.h"
#include <qptrlist.h>

class UMLObject;

namespace Umbrello{
class Diagram;
class AssociationWidget;

/** Base class for all widgets that represent some kind of UML Object */
class UMLWidget : public DiagramElement
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
	/** Return the width of the widget */
	inline uint width() const;
	/** Return the height of the widget*/
	inline uint height() const;
	
	/** By default all UMLWidgets' default action is to call editProperties.
	* Subclasses can override  this behavior*/
	virtual void execDefaultAction();
	
	/** Return the points bounding the widget. This is used to calculate the bounding rectanle*/
	virtual QPointArray areaPoints() const;
	
//<FIXME FIXME FIXME>
	/** Register an AssociationWidget, so that the widget can calculate the optimal "connecting point"*/
	virtual void registerAssociation(AssociationWidget *a);
	/** */
	virtual QPoint getConnectingPoint( AssociationWidget *a, const QPoint &from);
//</FIXME>

public slots:
	/** Inform the widget that the UML Object it is representing has been modified, so that the widget
	* can update itself */
	virtual void umlObjectModified();
	
protected:
	/** (Re)implement this function to draw the widget */
	virtual void drawShape(QPainter &) = 0;
	
	
	UMLObject *m_umlObject;
	uint m_width, m_height;
	
	QPtrList<AssociationWidget> m_associations;

};

//inline functions
UMLObject* UMLWidget::umlObject() const { return m_umlObject;}
uint UMLWidget::width() const  { return m_width;  }
uint UMLWidget::height() const { return m_height; }

} //end of namespace Umbrello

#endif  // UMBRELLO_UMLWIDGET_H


