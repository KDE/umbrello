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

#ifndef CODEVIEWEROPTIONSPAGE_H
#define CODEVIEWEROPTIONSPAGE_H

#include <qwidget.h>
#include "codevieweroptionsbase.h"
#include "settingsdlg.h"

/**
 * @author Brian Thomas
 */

class CodeViewerOptionsPage : public CodeViewerOptionsBase  {
	Q_OBJECT
public:
	CodeViewerOptionsPage (SettingsDlg::CodeViewerState options, QWidget *parent, const char *name=0);
	~CodeViewerOptionsPage();

	SettingsDlg::CodeViewerState getOptions();
	void apply();

protected:

private:

	SettingsDlg::CodeViewerState m_options;
	void init ( SettingsDlg::CodeViewerState options);

protected slots:

signals:
	void applyClicked();

};

#endif
