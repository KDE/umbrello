
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
#include <kdebug.h>
#include <klocale.h>

#include <qlayout.h>
#include <qtabwidget.h>

#include "codeviewerdialog.h"
#include "../codedocument.h"
#include "../classifiercodedocument.h"

#include "codeeditor.h"

CodeViewerDialog::CodeViewerDialog ( QWidget* parent, CodeDocument * doc, SettingsDlg::CodeViewerState state, 
                                     const char* name, bool modal, WFlags fl )
    : CodeViewerDialogBase ( parent, name, modal, fl )

{
	m_state = state;

	initGUI(name);

	addCodeDocument(doc);

}

/*
 *  Destroys the object and frees any allocated resources
 */
CodeViewerDialog::~CodeViewerDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void CodeViewerDialog::initGUI ( const char * name) {

	if ( !name )
		setName( "CodeViewerDialog" );

	setFont( getState().font );

	// set some viewability parameters
	int margin = fontMetrics().height();
	int width = fontMetrics().maxWidth() * getState().width;
	int height = fontMetrics().lineSpacing() * getState().height;

	CodeViewerDialogBaseLayout->setMargin(margin);

	resize( QSize(width, height).expandedTo(minimumSizeHint()) );

	// make connections to codetextedit

/*
	connect(codeTextEdit,SIGNAL(clicked(int,int)),this,SLOT(clicked(int,int)));
	connect(codeTextEdit,SIGNAL(mouseMoved(QMouseEvent*)),this,SLOT(mouseMoved(QMouseEvent*)));
	connect(codeTextEdit,SIGNAL(doubleClicked(int,int)),this,SLOT(doubleClicked(int,int)));
	connect(codeTextEdit,SIGNAL(newLinePressed()),this,SLOT(newLinePressed()));
	connect(codeTextEdit,SIGNAL(backspacePressed()),this,SLOT(backspacePressed()));
	connect(codeTextEdit,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(cursorPositionChanged(int,int)));
*/
}

/*
 *  Adds a code document to the tabbed output
 */
void CodeViewerDialog::addCodeDocument( CodeDocument * doc)
{
	CodeEditor * page = new CodeEditor ( this, "_codedocumenteditor_", doc );
	QString fname = doc->getFileName();
	QString ext = doc->getFileExtension();
	m_tabWidget->insertTab(page, (fname + (ext.isEmpty()? "" : ext)));

    	connect( m_highlightCheckBox, SIGNAL( stateChanged(int) ), page, SLOT( changeHighlighting(int) ) );
}

SettingsDlg::CodeViewerState CodeViewerDialog::getState() {
	return m_state;
}

bool CodeViewerDialog::close ( bool alsoDelete )
{

	// remember widget size for next time

	getState().height = height() / fontMetrics().lineSpacing();
	getState().width = width() / fontMetrics().maxWidth();

	// capture last code block, if it exists 
/*
	if(m_lastTextBlockToBeEdited)
		updateMethodBlockBody (m_lastTextBlockToBeEdited);
*/

	return CodeViewerDialogBase::close(alsoDelete);

}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CodeViewerDialog::languageChange()
{
    setCaption( tr2i18n( "Code Viewer" ) );
}

#include "codeviewerdialog.moc"
