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
#ifndef _Position_H_
#define _Position_H_

#include <boost/version.hpp>
#include <boost/spirit.hpp>

#if BOOST_VERSION == 103301

typedef boost::spirit::file_position Position;

#elif BOOST_VERSION == 103401
# include <QChar>

typedef boost::spirit::file_position_base<std::basic_string<QChar> > Position;

#else
# error Unknown version of boost lib
#endif

inline bool operator<( Position const& p1, Position const& p2) {
  assert( p1.file == p2.file);
  return( (p1.line < p2.line)
	  || ( (p1.line == p2.line) && (p1.column < p2.column)));
}

inline bool operator>=( Position const& p1, Position const& p2)
{return !(p1 < p2);}

#endif
