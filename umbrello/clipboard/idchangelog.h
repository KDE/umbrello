/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
#include <q3valuevector.h>

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

    void addIDChange(Uml::IDType OldID, Uml::IDType NewID);

    /**
     * Appends another IDChangeLog to this instance of IDChangeLog and
     * returns a reference to itself.
     */
    IDChangeLog& operator+=(const IDChangeLog& Other);

    Uml::IDType findNewID(Uml::IDType OldID);

    Uml::IDType findOldID(Uml::IDType NewID);

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
        Point()
        {}
        Point(const Uml::IDType &x, const Uml::IDType &y)
                : m_x(x), m_y(y)
        {}
        virtual ~Point() {}
        void setX(const Uml::IDType &x) { m_x = x; }
        Uml::IDType x() const { return m_x; }
        void setY(const Uml::IDType &y) { m_y = y; }
        Uml::IDType y() const { return m_y; }
    private:
        Uml::IDType m_x, m_y;
    };
class PointArray : Q3ValueVector<Point> {
    public:
        void  setPoint(uint i, const Uml::IDType &x, const Uml::IDType &y) {
            Point point(x, y);
            Q3ValueVector<Point>::at(i) = point;
        }
        const Point& point( uint i ) const { return Q3ValueVector<Point>::at(i); }
        uint   size() const          { return Q3ValueVector<Point>::size(); }
        bool   resize( uint size )   { Q3ValueVector<Point>::resize(size); return true; }
    };
    PointArray m_LogArray;

    bool findIDChange(Uml::IDType OldID, Uml::IDType NewID, uint& pos);
};

#endif
