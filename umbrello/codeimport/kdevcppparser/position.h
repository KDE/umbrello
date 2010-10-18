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

#ifdef _MSC_VER //Q_CC_MSVC isn't defined here
/*
workaround for the following msvc error 
...\Microsoft Visual Studio 8\VC\INCLUDE\xstring(2044) : error C2620: 
    member 'std::basic_string<_Elem>::_Bxty::_Buf ' of union 'std::basic_string<_Elem>::_Bxty' 
    has user-defined constructor or non-trivial default constructor with [  _Elem=QChar]
...\Microsoft Visual Studio 8\VC\INCLUDE\xstring(2046) : see reference to class 
    template instantiation 'std::basic_string<_Elem>::_Bxty' being compiled with [  _Elem=QChar]
..\umbrello\umbrello\codeimport\kdevcppparser\position.h(49) : see reference to class 
    template instantiation 'std::basic_string<_Elem>' being compiled with [  _Elem=QChar]
*/
#define union struct 
#include <xstring>
#undef union 
#endif

#include <limits.h>
#include <boost/version.hpp>
#include <boost/spirit/include/classic.hpp>
#include <QString>
#include <QChar>

typedef boost::spirit::classic::file_position_base<std::basic_string<QChar> > Position;
typedef std::basic_string<QChar> PositionFilename;

inline PositionFilename QString2PositionFilename( QString const& p)
{
    return p.data();
}

inline bool operator<( Position const& p1, Position const& p2)
{
    assert( p1.file == p2.file);
    return( (p1.line < p2.line) || ( (p1.line == p2.line) && (p1.column < p2.column)));
}

inline bool operator>=( Position const& p1, Position const& p2)
{
    return !(p1 < p2);
}

#endif
