/***************************************************************************
                               diagramwidget.h
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

#ifndef DIAGRAMWIDGET_H
#define DIAGRAMWIDGET_H

#include "diagramelement.h"
#include <q3ptrlist.h>
//Added by qt3to4:
#include <Q3PointArray>


namespace Umbrello{

class AssociationWidget;

/** @short Base class for all widgets shown in a Diagram.
  * DiagramWidgets are elements that can draw themselves, and can
  * have associations with other widgets in form of a path connecting
  * the two of them */
class DiagramWidget :  public DiagramElement
{
	Q_OBJECT

public:
	/** Constructor
	* @param diagram The diagram this widget should go in
	* @param id      Unique ID number
	*/
	DiagramWidget( Diagram *diagram, int id );
	/** Destructor */
	virtual ~DiagramWidget();
	
	/** Return the width of the widget */
	inline uint width() const;
	/** Return the height of the widget*/
	inline uint height() const;
	
	/** Return the points bounding the widget. This is used to calculate the bounding rectanle
	 * The default implementation returns the points given by the widget's position and size
	 * For rectangular shapes this should be ok, but you can reimplement this for fine tuning*/
	virtual Q3PointArray areaPoints() const;
		
protected:
	virtual void drawShape(QPainter& ) = 0;
	virtual void drawHotSpots(QPainter&);
	//called whenever the hotspots need to be recalculated. You can override this
	// to place the hotspots wherever you want on your widget. just append the points
	// relative to your widget's (0,0) to the list - (the points are the center of the
	//hotspot )
	virtual void createHotSpots(  );
	
	void setSize( uint w, uint h );
	
	
private:
	uint m_width, m_height;
};

//inline functions
uint DiagramWidget::width() const  { return m_width;  }
uint DiagramWidget::height() const { return m_height; }


} // end of namespace Umbrello

#endif  //  DIAGRAMWIDGET
