/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef IDCHANGELOG_H
#define IDCHANGELOG_H

#include "basictypes.h"

#include <QtCore/QVector>

/**
 * This class contains all the ID translations done for each
 * UMLObject pasted. It contains for each old id its new
 * assigned id.
 *
 * @author Gustavo Madrigal
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class IDChangeLog 
{
public:

    IDChangeLog();
    IDChangeLog(const IDChangeLog& Other);

    ~IDChangeLog();

    IDChangeLog& operator=(const IDChangeLog& Other);

    bool operator==(const IDChangeLog& Other) const;

    void addIDChange(Uml::IDType OldID, Uml::IDType NewID);

    IDChangeLog& operator+=(const IDChangeLog& Other);

    Uml::IDType findNewID(Uml::IDType OldID);
    Uml::IDType findOldID(Uml::IDType NewID);

    void removeChangeByNewID( Uml::IDType OldID);

    enum SpecialIDs
    {
        NullID = -1000  ///< An impossible id value.
    };

private:

    /**
     * Each change is a Point (x=newID, y=oldID)
     */
    class Point
    {
    public:
        Point() {}
        Point(const Uml::IDType &x, const Uml::IDType &y) : m_x(x), m_y(y) {}
        virtual ~Point() {}
        void setX(const Uml::IDType &x) { m_x = x; }
        Uml::IDType x() const { return m_x; }
        void setY(const Uml::IDType &y) { m_y = y; }
        Uml::IDType y() const { return m_y; }
    private:
        Uml::IDType m_x, m_y;
    };

    class PointArray : QVector<Point*>
    {
    public:
        void  setPoint(uint i, const Uml::IDType &x, const Uml::IDType &y) {
            QVector<Point*>::insert(i, new Point(x, y));
        }
        Point* point( uint i ) const { return QVector<Point*>::at(i); }
        uint   size() const          { return QVector<Point*>::size(); }
        bool   resize( uint size )   { QVector<Point*>::resize(size); return true; }
    };

    PointArray m_LogArray;

    bool findIDChange(Uml::IDType OldID, Uml::IDType NewID, uint& pos);
};

#endif
