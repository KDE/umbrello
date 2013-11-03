/*
    Copyright 2008  Jean Vittor  <jean.vittor@free.fr>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy 
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef POSITION_H
#define POSITION_H

#include <limits.h>
#include <boost/version.hpp>
#include <boost/spirit/include/classic.hpp>
#include <QString>
#include <QChar>
#include <QDebug>

typedef QString PositionFilename;
typedef boost::spirit::classic::file_position_base<PositionFilename> PositionType;

class Position : public PositionType
{
public:
    Position()
    {
    }

    explicit Position(const QString& fileName) : PositionType(fileName)
    {
    }

    Position(const Position &p) : PositionType(p)
    {
    }

    explicit Position(const PositionType &p) : PositionType(p)
    {
    }

    bool operator<(Position const& p) const
    {
        assert( file == p.file);
        return( (line < p.line) || ( (line == p.line) && (column < p.column)));
    }

    bool operator>=(Position const& p) const
    {
        return !(*this < p);
    }
};

inline QDebug operator<<(QDebug out, Position const &p)
{
    out << "Position("
        << "file" << p.file
        << "line" << p.line
        << "column" << p.column
        << ")";
    return out;
}

typedef boost::spirit::classic::position_iterator<QChar const*, PositionType> CharIteratorType;

class CharIterator : public CharIteratorType
{
public:
    CharIterator()
    { 
    }

    CharIterator(const QChar *a, const QChar *b, Position p) : CharIteratorType(a,b,p)
    {
    }

    CharIterator(const PositionType &p) : CharIteratorType(0, 0, p)
    {
    }
/*
    CharIterator(PositionType p) : CharIteratorType(0, 0, p)
    {
    }
*/
    const Position &get_position() const
    {
        return static_cast<const Position&>(CharIteratorType::get_position());
    }

    void set_position(Position const& p)
    {
        CharIteratorType::set_position(p);
    }
};


#endif
