/***************************************************************************
                               diagramelement.h
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

#ifndef DIAGRAMELEMENT_H
#define DIAGRAMELEMENT_H

#include <qobject.h>
#include <qcanvas.h>
#include <qpoint.h>

#include "diagram.h"

class QPopupMenu;

namespace Umbrello{
class Diagram;

/** @short Base class for all elements shown in a Diagram */
class DiagramElement :  public QObject, public QCanvasPolygonalItem
{
	Q_OBJECT

public:
	/** Constructor
	* @param diagram The diagram this element should go in
	* @param id      Unique ID number
	*/
	DiagramElement( Diagram *diagram, int id );
	/** Destructor */
	virtual ~DiagramElement();
	
	/** Return the ID of this element */
	int getID() const;
	
	/** Move the element to an absolute position in the diagram.
	* @param x X coordinate to move to 
	* @param y Y coordinate to move to
	*/
	virtual void moveAbs( int x, int y );
	
	/** @see moveAbs( int x, int y ) */
	virtual void moveAbs( const QPoint & );
	
	/** Move the element a certain distance in the diagram, relative to
	* its current position.
	* @param dx Distance to move in the x axis
	* @param dy Distance to move in the y axis
	*/
	virtual void moveBy( int dx, int dy);
	
	virtual void moveHotSpotBy( int h, int dx, int dy );
	
	
	int isHotSpot( const QPoint& ) const ;
	int closestHotSpot( const QPoint& ) const;
	QPoint hotSpotPosition( int i ) const;
	void showHotSpots( int highlight = -1 );
	void hideHotSpots( );
	inline bool isShowHotSpots( ) const;
	int hotSpotSize( ) const;
	
	/** Populate a context menu with items / actions for this element
	*  @param menu The popup menu to insert the actions in
	*/
	virtual void fillContextMenu(QPopupMenu &menu);

	/** Sets the selected status of the element*/
	virtual void setSelected(bool);
	
	/** Execute the default action for this element. This action is normaly one
	* of the actions from fillContextMenu, or maybe nothing.
	* You can call this method when the user doubleclicks on the element, or when he presses
	* Enter while the element is selected (for example )
	*/
	virtual void execDefaultAction();
	
	/** Return a reference to the diagram in which the element is */
	Diagram* diagram() const;
	
public slots:
	/** Move the element one level up in the element stack */
	virtual void raise( );
	/** Move the element to the forground */
	virtual void moveToForeground();
	/** Move the element one level down in the widget stack */
	virtual void lower( );
	/** Move the element to the background*/
	virtual void moveToBackground();
	
	/** Show a dialog to edit the properties of the element */
	virtual void editProperties();
	
signals:
	/** Signals that the element has been moved */
	void moved();
	/** Signals that the element has been selected*/
	void selected(bool);
	
protected:
	virtual void drawShape(QPainter& ) = 0;
	virtual void drawHotSpots(QPainter&);
	//called whenever the hotspots need to be recalculated. You can override this
	// to place the hotspots wherever you want on your widget. just append the points
	// relative to your widget's (0,0) to the list - (the points are the center of the
	//hotspot )
	virtual void createHotSpots( );
	
	void setHotSpotSize( int s );
		
	int  m_id;
	bool m_useOwnPen;
	bool m_useOwnBrush;
	mutable QPtrList<QPoint> m_hotSpots;
private:
	int m_highlightHotSpot;
	int m_hotSpotSize;
	bool m_showHotSpots;
	

};

bool DiagramElement::isShowHotSpots( ) const {return m_showHotSpots;}

} // end of namespace Umbrello

#endif  //  DIAGRAMELEMENT_H
