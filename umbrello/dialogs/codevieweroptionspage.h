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
#include "../codeviewerstate.h"

/**
 * @author Brian Thomas
 */

class CodeViewerOptionsPage : public CodeViewerOptionsBase  {
    Q_OBJECT
public:
    CodeViewerOptionsPage (Settings::CodeViewerState options, QWidget *parent, const char *name=0);
    ~CodeViewerOptionsPage();

    Settings::CodeViewerState getOptions();
    void apply();

protected:

private:

    Settings::CodeViewerState m_options;
    void init ( Settings::CodeViewerState options);

protected slots:

signals:
    void applyClicked();

};

#endif
