/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef IDCHANGELOG_H
#define IDCHANGELOG_H

#include "basictypes.h"

#include <QVector>

/**
 * This class contains all the ID translations done for each
 * UMLObject pasted. It contains for each old id its new
 * assigned id.
 *
 * @author Gustavo Madrigal
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class IDChangeLog 
{
public:

    IDChangeLog();
    IDChangeLog(const IDChangeLog& Other);

    ~IDChangeLog();

    IDChangeLog& operator=(const IDChangeLog& Other);

    bool operator==(const IDChangeLog& Other) const;

    void addIDChange(Uml::ID::Type OldID, Uml::ID::Type NewID);

    IDChangeLog& operator+=(const IDChangeLog& Other);

    Uml::ID::Type findNewID(Uml::ID::Type OldID);
    Uml::ID::Type findOldID(Uml::ID::Type NewID);

    void removeChangeByNewID(Uml::ID::Type OldID);

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
        Point(const Uml::ID::Type &x, const Uml::ID::Type &y) : m_x(x), m_y(y) {}
        virtual ~Point() {}
        void setX(const Uml::ID::Type &x) { m_x = x; }
        Uml::ID::Type x() const { return m_x; }
        void setY(const Uml::ID::Type &y) { m_y = y; }
        Uml::ID::Type y() const { return m_y; }
    private:
        Uml::ID::Type m_x, m_y;
    };

    class PointArray : QVector<Point*>
    {
    public:
        void  setPoint(uint i, const Uml::ID::Type &x, const Uml::ID::Type &y) {
            QVector<Point*>::insert(i, new Point(x, y));
        }
        Point* point(uint i) const { return QVector<Point*>::at(i); }
        uint   size() const        { return QVector<Point*>::size(); }
        bool   resize(uint size)   { QVector<Point*>::resize(size); return true; }
    };

    PointArray m_LogArray;

    bool findIDChange(Uml::ID::Type OldID, Uml::ID::Type NewID, uint& pos);
};

#endif
