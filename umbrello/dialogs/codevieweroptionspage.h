/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002                                                    *
 *   Luis De la Parra <luis@delaparra.org>                                 *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef CODEVIEWEROPTIONSPAGE_H
#define CODEVIEWEROPTIONSPAGE_H

#include <QtGui/QWidget>
#include "ui_codevieweroptionsbase.h"
#include "codeviewerstate.h"

/**
 * @author Brian Thomas
 */
class CodeViewerOptionsPage : public QWidget, private Ui::CodeViewerOptionsBase
{
    Q_OBJECT
public:
    CodeViewerOptionsPage (Settings::CodeViewerState options, QWidget *parent, const char *name = 0);
    ~CodeViewerOptionsPage();

    Settings::CodeViewerState getOptions();
    void apply();

private:
    Settings::CodeViewerState m_options;

protected slots:

signals:
    void applyClicked();

};

#endif
