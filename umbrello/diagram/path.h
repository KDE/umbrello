 /***************************************************************************
                               path.h
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
 
#ifndef UMBRELLO_PATH_H
#define UMBRELLO_PATH_H

#include "diagramelement.h"
#include "pathsegment.h"
#include <qlist.h>

class QPointArray;

namespace Umbrello{

class Diagram;
class PathSegment;

/** A path consisting of one or more path segments */
class Path : public DiagramElement
{
Q_OBJECT
public:
	enum PathStyle {Orthogonal, Direct};
	/** Constructor
	* @param  diagram The diagram this path is in
	* @param  id The unique ID of the diagram element
	*/
	Path( Diagram *diagram, uint id);
	
	/** Destructor */
	virtual ~Path();
	
	/** Make all segments of the path have a consistent visible state*/
	virtual void setVisible(bool);
	
	/** Set the points for the path. */
	virtual void setPathPoints( const QPointArray& );
	
	/** Move the complete path to an absolute position in the diagram.
	* Reimplemented from DiagramElement : the absolute position refers to
	* the first point in the path (start of first segment) all other segments
	* are moved accordingly
	* @param x X coordinate to move to 
	* @param y Y coordinate to move to
	*/
	virtual void moveAbs( int x, int y );
	
	/** Move the path
	 * @see moveAbs( int x, int y ) */
	virtual void moveAbs( const QPoint & );
	
	/** Move the complete path a certain distance in the diagram, relative to
	* its current position. Reimplemented from DiagramElement
	* @param dx Distance to move in the x axis
	* @param dy Distance to move in the y axis
	*/
	virtual void moveBy( int dx, int dy);
	
	virtual void moveHotSpotBy( int h, int dx, int dy );
	
	/** Populate a context menu with items / actions for this element
	*  @param menu The popup menu to insert the actions in
	*/
	virtual void fillContextMenu(QPopupMenu &menu);
	
	/** Execute teh default action for Paths: create/delete
	 * hotspots in the path
	 */
	virtual void execDefaultAction();
	virtual void toggleHotSpot( const QPoint& );
	
	virtual QPointArray areaPoints() const;

		
public slots:
	void setDirectStyle();
	void setOrthogonalStyle();

protected:
	virtual void drawShape(QPainter& );
	virtual void drawHotSpots(QPainter&);
	//called whenever the hotspots need to be recalculated. You can override this
	// to place the hotspots wherever you want on your widget. just append the points
	// relative to your widget's (0,0) to the list - (the points are the center of the
	//hotspot )
	virtual void createHotSpots(  );
	
	
	QPtrList<PathSegment> m_segments;
	PathStyle m_style;
};

} //end of namespace Umbrello

#endif  // UMBRELLO_PATH_H
