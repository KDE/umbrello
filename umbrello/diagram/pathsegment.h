/***************************************************************************
                                  pathsegment.h
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
 
#ifndef UMBRELLO_PATH_SEGMENT_H
#define UMBRELLO_PATH_SEGMENT_H

#include <qcanvas.h>


namespace Umbrello{

class Path;

/** Represents a segment within a path*/
class PathSegment : public QCanvasLine
{
friend class Path;
public:
	/** Destructor */
	virtual ~PathSegment();
	inline Path* path() const;
	virtual void moveBy( double dx, double dy);
	virtual QPointArray areaPoints() const;
	
protected: 
	/** Constructor
	* @param path The path this segment is part of
	*/
	PathSegment( Path *path );
	
	virtual void drawShape(QPainter& );
private:
	Path *m_path;
};

//inline functions
Path* PathSegment::path() const { return m_path;}

} //end of namespace Umbrello

#endif // UMBRELLO_PATH_SEGMENT_H
