
/***************************************************************************
                          codeeditor.h  -  description
                             -------------------
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Brian Thomas
    email                : brian.thomas@gsfc.nasa.gov
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <qstring.h>
#include <qtextedit.h>
#include "settingsdlg.h"

class CodeEditor : public QTextEdit
{
    Q_OBJECT
public:

    CodeEditor ( const QString & text, const QString & context = QString::null, QWidget * parent = 0, const char * name = 0 );
    CodeEditor ( QWidget* parent, const char* name = 0);
    ~CodeEditor ();

protected:

	void contentsMouseMoveEvent ( QMouseEvent * e );
	void keyPressEvent ( QKeyEvent * e );

public slots:

	void insertParagraph ( const QString & text, int para );
	void removeParagraph ( int para );

private:

	void init();

signals:

	void mouseMoved ( QMouseEvent * e);
	void newLinePressed ();
	void backspacePressed ();

};

#endif // CODEEDITOR_H
