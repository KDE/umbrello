/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "umlnamespace.h"
#include "qregexp.h"

namespace Uml {

    bool tagEq (QString tag, QString pattern) {
	tag.remove( QRegExp("^\\w+:") );  // remove leading "UML:" or other
	int patSections = pattern.contains( '.' ) + 1;
	QString tagEnd = tag.section( '.', -patSections );
	return (tagEnd.lower() == pattern.lower());
    }

}
