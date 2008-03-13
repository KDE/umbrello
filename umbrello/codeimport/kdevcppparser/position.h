/* umbrello/codeimport/kdevcppparser/driver.h */
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
