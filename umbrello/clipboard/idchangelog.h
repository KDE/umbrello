/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IDCHANGELOG_H
#define IDCHANGELOG_H


/**
 * This class contains all the ID translations done for each
 * UMLObject pasted. It contains for each old id its new
 * assigned id.
 *
 * @author Gustavo Madrigal
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

#include <qstring.h>
#include <qmemarray.h>

#include "../umlnamespace.h"

class IDChangeLog {
public:
	/**
	 * Constructor.
	 */
	IDChangeLog();

	/**
	 * Copy constructor.
	 */
	IDChangeLog(const IDChangeLog& Other);

	/**
	 * Deconstructor.
	 */
	~IDChangeLog();

	/**
	 * Overloaded '=' operator.
	 */
	IDChangeLog& operator=(const IDChangeLog& Other);

	/**
	 * Overloaded '==' operator.
	 */
	bool operator==(const IDChangeLog& Other);

	/**
	 * Adds a new ID Change to the log.
	 */
	void addIDChange(Uml::IDType OldID, Uml::IDType NewID);

	/**
	 * Appends another IDChangeLog to this instance of IDChangeLog and
	 * returns a reference to itself.
	 */
	IDChangeLog& operator+=(const IDChangeLog& Other);

	/**
	 * Returns the new assigned ID of the object that had OldID as its
	 * previous id.
	 */
	Uml::IDType findNewID(Uml::IDType OldID);

	/**
	 * Returns the old ID of an UMLobject given its new one.
	 */
	Uml::IDType findOldID(Uml::IDType NewID);

	/**
	 * Removes a change giving an New ID.
	 */
	void removeChangeByNewID( Uml::IDType OldID);

	enum SpecialIDs
	{
	    NullID = -1000 ///< An impossible id value.
	};

private:
	/**
	 * Each change is a Point (x=newID, y=oldID)
	 */
	class Point {
	public:
		Point(Uml::IDType x, Uml::IDType y) { m_x = x; m_y = y; }
		virtual ~Point() {}
		void   setX( Uml::IDType x ) { m_x = x; }
		Uml::IDType x() { return m_x; }
		void   setY( Uml::IDType y ) { m_y = y; }
		Uml::IDType y() { return m_y; }
	private:
		Uml::IDType m_x, m_y;
	};
	class PointArray : QMemArray<Point> {
	public:
		void  setPoint( uint i, Uml::IDType x, Uml::IDType y ) {
			QMemArray<Point>::at(i) = Point(x, y);
		}
		Point& point( uint i ) const { return QMemArray<Point>::at(i); }
		uint   size() const          { return QMemArray<Point>::size(); }
		bool   resize( uint size )   { return QMemArray<Point>::resize(size); }
	};
	PointArray m_LogArray;

	/**
	 * Finds a specific change in the log.
	 */
	bool findIDChange(Uml::IDType OldID, Uml::IDType NewID, uint& pos);
};

#endif
