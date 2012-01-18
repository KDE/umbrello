/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "idchangelog.h"

/**
 * Constructor.
 */
IDChangeLog::IDChangeLog()
{
}

/**
 * Copy constructor.
 */
IDChangeLog::IDChangeLog(const IDChangeLog& Other)
{
    m_LogArray = Other.m_LogArray;
}

/**
 * Deconstructor.
 */
IDChangeLog::~IDChangeLog()
{
    for (uint i = 0; i < m_LogArray.size(); i++) {
        delete m_LogArray.point(i);
    }
}

/**
 * Overloaded '=' operator.
 */
IDChangeLog& IDChangeLog::operator=(const IDChangeLog& Other)
{
    m_LogArray = Other.m_LogArray;

    return *this;
}

/**
 * Overloaded '==' operator.
 */
bool IDChangeLog::operator==(const IDChangeLog& Other) const
{
    Q_UNUSED(Other);
    /*It needs to be Implemented*/
    return false;
}

/**
 * Returns the new assigned ID of the object that had OldID as its
 * previous id.
 */
Uml::IDType IDChangeLog::findNewID(Uml::IDType OldID)
{
    for (uint i = 0; i < m_LogArray.size(); i++) {
        if ((m_LogArray.point(i))->y() ==  OldID) {
            return  (m_LogArray.point(i))->x();
        }
    }

    return Uml::id_None;
}

/**
 * Appends another IDChangeLog to this instance of IDChangeLog and
 * returns a reference to itself.
 */
IDChangeLog& IDChangeLog::operator+=(const IDChangeLog& Other)
{
    //m_LogArray.putpoints(m_LogArray.size(), Other.m_LogArray.size(), Other)
    uint count = Other.m_LogArray.size();
    for (uint i = 0; i < count; i++) {
        addIDChange((Other.m_LogArray.point(i))->y(), (Other.m_LogArray.point(i))->x());
    }

    return *this;
}

void IDChangeLog::addIDChange(Uml::IDType OldID, Uml::IDType NewID)
{
    uint pos = 0;
    if (!findIDChange(OldID, NewID, pos)) {
        pos = m_LogArray.size();
        m_LogArray.setPoint(pos, NewID, OldID);
    } else {
        m_LogArray.setPoint(pos, NewID, OldID);
    }
}

Uml::IDType IDChangeLog::findOldID(Uml::IDType NewID)
{
    uint count = m_LogArray.size();
    for (uint i = 0; i < count; i++) {
        if ((m_LogArray.point(i))->x() ==  NewID) {
            return (m_LogArray.point(i))->y();
        }
    }

    return Uml::id_None;
}

bool IDChangeLog::findIDChange(Uml::IDType OldID, Uml::IDType NewID, uint& pos)
{
    uint count = m_LogArray.size();
    for (uint i = 0; i < count; i++) {
        if (((m_LogArray.point(i))->y() ==  OldID) && ((m_LogArray.point(i))->x() ==  NewID)) {
            pos = i;
            return  true;
        }
    }

    return false;
}

void IDChangeLog::removeChangeByNewID(Uml::IDType OldID)
{
    uint count = m_LogArray.size();
    for (uint i = 0; i < count; i++) {
        if ((m_LogArray.point(i))->y() ==  OldID) {
            m_LogArray.setPoint(i, Uml::id_None, OldID);
        }
    }
}
