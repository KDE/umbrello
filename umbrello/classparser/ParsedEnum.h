/***************************************************************************
                          ParsedContainer.h  -  description
                             -------------------
    begin                : Mon Mar 15 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef _CPARSEDENUM_H_INCLUDED
#define _CPARSEDENUM_H_INCLUDED

#include "ParsedItem.h"
#include <qstringlist.h>

/** Represents a parsed enum.
 *  Since this is a special case of a parsed item, the container
 *  inherits CParsedItem.
 *
 * @author Oliver M. Kellogg  <okellogg@users.sourceforge.net>
 */
class CParsedEnum : public CParsedItem
{
public: // Constructor and destructor

  CParsedEnum() { setItemType(PIT_ENUM); }
  ~CParsedEnum() {}

public: // Public attributes

  /** The literals. */
  QStringList literals;

public: // Implementation of virtual methods

  /** Output this object to stdout */
  virtual void out() {}

  /** Return a string made for persistant storage. 
   * @param str String to store the result in.
   * @return Pointer to str.
   */
  virtual QString asPersistantString( QString &str ) { return str;}

  /** Initialize the object from a persistant string. 
   * @param str String to initialize from.
   * @param startPos Position(0-based) at which to start.
   */
  virtual int fromPersistantString( const char *, int startPos ) { return startPos; }

};

#endif
