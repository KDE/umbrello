/***************************************************************************
                          configcodegenerators.h  -  description
                             -------------------
    begin                : Tue Jun 18 2002
    copyright            : (C) 2002 by Luis De la Parra
    email                : luis@delaparra.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGCODEGENERATORS_H
#define CONFIGCODEGENERATORS_H

#include "../codegenerator.h"
#include "configgeneratorsbase.h"
#include <qdict.h>

/**
 * @author Luis De la Parra
 */

class GeneratorInfo;
typedef QDict<GeneratorInfo> GeneratorInfoDict;

class KURL;

class ConfigCodeGenerators : public ConfigCodeGenBase {
	Q_OBJECT
public:
	ConfigCodeGenerators(GeneratorInfoDict dict, QWidget *parent);
	virtual ~ConfigCodeGenerators();

public slots:
	void addLanguage();
	void removeLanguage();
	virtual GeneratorInfoDict configDictionary();

};

#endif
