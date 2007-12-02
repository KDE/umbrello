/***************************************************************************
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
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codeviewerdialog.h"

// qt/kde includes
#include <QLayout>
#include <QTabWidget>
#include <QCheckBox>
#include <kdebug.h>
#include <klocale.h>

// local includes
#include "../uml.h"
#include "../codedocument.h"
#include "../classifiercodedocument.h"
#include "codeeditor.h"

CodeViewerDialog::CodeViewerDialog ( QWidget* parent, CodeDocument * doc,
                                     Settings::CodeViewerState state,
                                     const char* name, bool modal, Qt::WFlags fl )
        : QDialog ( parent, fl )
{
    setObjectName(name);
    uDebug() << "setObjectName(" << name << ")" << endl;
    setModal(modal);
    setupUi(this);

    m_state = state;

    initGUI(name);

    addCodeDocument(doc);
}

CodeViewerDialog::~CodeViewerDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void CodeViewerDialog::initGUI ( const char * name)
{
    if ( !name )
        setObjectName( "CodeViewerDialog" );

    setFont( getState().font );

    // set some viewability parameters
    int margin = fontMetrics().height();
    int width = fontMetrics().maxWidth() * getState().width;
    int height = fontMetrics().lineSpacing() * getState().height;

    m_highlightCheckBox->setChecked( getState().blocksAreHighlighted );
    m_showHiddenCodeCB->setChecked ( getState().showHiddenBlocks );

    CodeViewerDialogBase::gridLayout->setMargin(margin);

    resize( QSize(width, height).expandedTo(minimumSizeHint()) );
}

void CodeViewerDialog::addCodeDocument( CodeDocument * doc)
{
    CodeEditor * page = new CodeEditor ( this, "_codedocumenteditor_", doc );
    QString name = doc->getFileName();
    QString ext = doc->getFileExtension();
    uDebug() << "name=" << name << " / ext=" << ext << endl;
    m_tabWidget->addTab(page, (name + (ext.isEmpty() ? "" : ext)));

    connect( m_highlightCheckBox, SIGNAL( stateChanged(int) ), page, SLOT( changeHighlighting(int) ) );
    connect( m_showHiddenCodeCB, SIGNAL( stateChanged(int) ), page, SLOT( changeShowHidden(int) ) );
}

Settings::CodeViewerState CodeViewerDialog::getState()
{
    return m_state;
}

bool CodeViewerDialog::close ( bool /*alsoDelete*/ )
{
    // remember widget size for next time
    m_state.height = height() / fontMetrics().lineSpacing();
    m_state.width = width() / fontMetrics().maxWidth();

    // remember block highlighting
    m_state.blocksAreHighlighted = m_highlightCheckBox->isChecked();

    // remember block show status
    m_state.showHiddenBlocks = m_showHiddenCodeCB->isChecked();

    // run superclass close now
    return QDialog::close();
}

void CodeViewerDialog::languageChange()
{
    Uml::Programming_Language pl = UMLApp::app()->getActiveLanguage();

    setWindowTitle( tr2i18n( "Code Viewer - " ) + pl );
}

#include "codeviewerdialog.moc"
