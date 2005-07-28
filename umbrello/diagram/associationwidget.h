 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef UMBRELLO_ASSOCIATIONWIDGET_H
#define UMBRELLO_ASSOCIATIONWIDGET_H

#include "path.h"
//Added by qt3to4:
#include <Q3PointArray>
#include <Q3PopupMenu>


namespace Umbrello{

class Diagram;
class DiagramWidget;


/** A path that links two diagramwidgets together */
class AssociationWidget : public Path
{
Q_OBJECT
public:
	/** Constructor
	* @param  diagram The diagram this path is in
	* @param  id The unique ID of the diagram element
	*/
	AssociationWidget( Diagram *diagram, uint id, DiagramWidget *start, DiagramWidget *end );
	
	/** Destructor */
	virtual ~AssociationWidget();
	
	/** Move the complete association a certain distance in the diagram, relative to
	* its current position. Reimplemented from Path - the first and end point (points attached)
	* to the widgets dont move, all other points/segments move by the desired distance
	* @param dx Distance to move in the x axis
	* @param dy Distance to move in the y axis
	*/
	virtual void moveBy( int dx, int dy);
	
	virtual void moveHotSpotBy( int h, int dx, int dy );
	
	/** set tha path points - reimplemented for internal reasons.
	  * the start and end points of the path will be set to the closest
	  * hotspots of the start/end widgets to the first/last points in the array, respectively
	  */
	virtual void setPathPoints( const Q3PointArray& );
	
	/** Populate a context menu with items / actions for this element
	*  @param menu The popup menu to insert the actions in
	*/
	virtual void fillContextMenu(Q3PopupMenu &menu);
	
	inline DiagramWidget* startWidget() const;
	inline DiagramWidget* endWidget() const;
		
public slots:
	virtual void widgetMoved( );
	virtual void setFixedSpots( );
	virtual void setAutoAdjust( );
protected:
	DiagramWidget *m_startWidget;
	DiagramWidget *m_endWidget;
	int m_startSpot;
	int m_endSpot;
	bool m_autoAdjust;
};

//inline functions
DiagramWidget* AssociationWidget::startWidget() const
{ return m_startWidget; }
DiagramWidget* AssociationWidget::endWidget() const
{ return m_endWidget; }

} //end of namespace Umbrello

#endif  // UMBRELLO_ASSOCIATIONWIDGET_H
