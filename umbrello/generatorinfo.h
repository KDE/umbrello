/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GENERATORINFO_H
#define GENERATORINFO_H

#include <qstring.h>
#include <qdict.h>

/**
 * This is really a leftover from the time when the code generators
 * resided in a shared library. The QDict<GeneratorInfo> should
 * probably be changed to a QStringList as the `object' member is
 * no longer needed.
 */
class GeneratorInfo {
public:
	QString language;
	QString object;
};

/**
 * Type for the dictionary of available languages and their generator names.
 */
typedef QDict<GeneratorInfo> GeneratorDict;
typedef QDictIterator<GeneratorInfo> GeneratorDictIt;

#endif
