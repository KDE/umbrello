
/***************************************************************************
                          codeviewerdialog.cpp  -  description
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
#include <iostream.h>

#include <qcursor.h>
#include "codeeditor.h"


CodeEditor::CodeEditor ( const QString & text, const QString & context, QWidget * parent, const char * name )
    : QTextEdit ( text, context, parent, name) 
{
	init();
};

CodeEditor::CodeEditor ( QWidget* parent, const char* name)
    : QTextEdit ( parent, name )
{
	init();
};

/*
 *  Destroys the object and frees any allocated resources
 */
CodeEditor::~CodeEditor() { };

void CodeEditor::contentsMouseMoveEvent ( QMouseEvent * e ) {
	emit mouseMoved(e);
}

void CodeEditor::keyPressEvent ( QKeyEvent * e ) {

	cerr<<"KEY PRESS EVENT:["<<e->text().latin1()<<"] ascii CODE:"<<e->ascii()<<endl;

	if((e->ascii() == 8) ) // || (e->ascii() == 127))
		emit backspacePressed();

	// Q: can the MAC or WIN/DOS sequences occur? 
	if((e->ascii() == 10) || (e->ascii() == 13) || (e->text() == "\r\n")) 
		emit newLinePressed();

	QTextEdit::keyPressEvent(e);
}

void CodeEditor::insertParagraph ( const QString & text, int para ) {

	cerr<<"INSERT PARAGRAPH CALLED FOR :"<<para<<endl;
	QTextEdit::insertParagraph(text,para);

}

void CodeEditor::removeParagraph ( int para ) {

	cerr<<"REMOVE PARAGRAPH CALLED FOR :"<<para<<endl;

	QTextEdit::removeParagraph(para);
}

void CodeEditor::init () {
	setReadOnly (true);
}

