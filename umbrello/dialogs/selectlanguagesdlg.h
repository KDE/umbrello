/***************************************************************************
                          selectlaguagesdlg.h  -  description
                             -------------------
    begin                : Mon Jul 15 2002
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

#ifndef SELECTLANGUAGESDLG_H
#define SELECTLANGUAGESDLG_H

#include "sellanguagesbase.h"
#include <qdict.h>

class GeneratorInfo;
typedef QDict<GeneratorInfo> GeneratorInfoDict;

/**
 * @author Luis De la Parra
 */

class SelectLanguagesDlg : protected SelectLanguagesBase {
public:
	SelectLanguagesDlg(QWidget *parent=0, const char *name =0);
	~SelectLanguagesDlg();

	GeneratorInfoDict selectedLanguages( );

	void offerLanguages(const QStringList & libraries,
			     const GeneratorInfoDict &gdict = 0);

	int exec() {
		return QDialog::exec();
	}

};

#endif
