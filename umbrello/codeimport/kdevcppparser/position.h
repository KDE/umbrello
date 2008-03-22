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

struct Position {
  Position() : m_line(0), m_column(0) {}
  Position( int line, int column) : m_line( line), m_column( column) {}

  bool operator==( Position const& p) const
  {return ((m_line == p.m_line) && (m_column == p.m_column));}

  bool operator<( Position const& p) const {
    return ((m_line < p.m_line)
	    || ((m_line == p.m_line) && (m_column < p.m_column)));
  }

  bool operator>=( Position const& p) const {return ! (*this < p);}

  int m_line, m_column;
};

#endif
