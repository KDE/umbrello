/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
Uml::ID::Type IDChangeLog::findNewID(Uml::ID::Type OldID)
{
    for (uint i = 0; i < m_LogArray.size(); i++) {
        if ((m_LogArray.point(i))->y() ==  OldID) {
            return  (m_LogArray.point(i))->x();
        }
    }

    return Uml::ID::None;
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

void IDChangeLog::addIDChange(Uml::ID::Type OldID, Uml::ID::Type NewID)
{
    uint pos = 0;
    if (!findIDChange(OldID, NewID, pos)) {
        pos = m_LogArray.size();
        m_LogArray.setPoint(pos, NewID, OldID);
    } else {
        m_LogArray.setPoint(pos, NewID, OldID);
    }
}

Uml::ID::Type IDChangeLog::findOldID(Uml::ID::Type NewID)
{
    uint count = m_LogArray.size();
    for (uint i = 0; i < count; i++) {
        if ((m_LogArray.point(i))->x() ==  NewID) {
            return (m_LogArray.point(i))->y();
        }
    }

    return Uml::ID::None;
}

bool IDChangeLog::findIDChange(Uml::ID::Type OldID, Uml::ID::Type NewID, uint& pos)
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

void IDChangeLog::removeChangeByNewID(Uml::ID::Type OldID)
{
    uint count = m_LogArray.size();
    for (uint i = 0; i < count; i++) {
        if ((m_LogArray.point(i))->y() ==  OldID) {
            m_LogArray.setPoint(i, Uml::ID::None, OldID);
        }
    }
}
