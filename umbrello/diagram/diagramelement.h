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
#include <qpoint.h>
#include <kdebug.h>

#include "diagram.h"

class QPopupMenu;

namespace Umbrello{

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
	
	/** Move the element to an absolute position in the diagram
	* @param x X coordinate to move to 
	* @param y Y coordinate to move to
	*/
	virtual void moveAbs( int x, int y );
	
	/** @see moveAbs( int x, int y ) */
	virtual void moveAbs( const QPoint & );
	
	/** Move the element a certain distance in the diagram, relative to
	* its current position
	* @param dx Distance to move in the x axis
	* @param dy Distance to move in the y axis
	*/
	virtual void moveBy( int dx, int dy);
	
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
	void raise( );
	/** Move the element to the forground */
	void moveToForeground();
	/** Move the element one level down in the widget stack */
	void lower( );
	/** Move the element to the background*/
	void moveToBackground();
	
	/** Show a dialog to edit the properties of the element */
	virtual void editProperties();
	
signals:
	/** Signals that the element has been moved */
	void moved();
	
protected:
	virtual void drawShape(QPainter& ) = 0;

	int m_id;
	bool m_useOwnPen;
	bool m_useOwnBrush;
};


} // end of namespace Umbrello

#endif  //  DIAGRAMELEMENT_H
