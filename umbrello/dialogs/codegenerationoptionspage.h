/***************************************************************************
                          codegenerationoptionspage.h  -  description
                             -------------------
    begin                : Thu Jul 25 2002
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

#ifndef CODEGENERATIONOPTIONSPAGE_H
#define CODEGENERATIONOPTIONSPAGE_H

#include <qwidget.h>
#include "codegenerationoptionsbase.h"
#include "settingsdlg.h"

/**
 * @author Luis De la Parra
 */

class CodeGenerationOptionsPage : public CodeGenerationOptionsBase  {
	Q_OBJECT
public:
	CodeGenerationOptionsPage(const SettingsDlg::CodeGenState &state, QDict<GeneratorInfo> ldict,
	                          QString activeLanguage, QWidget *parent=0, const char *name=0);
	~CodeGenerationOptionsPage();
	void setDefaults();
	void setState(const SettingsDlg::CodeGenState &state);
	void state(SettingsDlg::CodeGenState &state);
	QString getCodeGenerationLanguage();
protected slots:
	void browseClicked();
};

#endif
