/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003                                                    *
 *   Brian Thomas <brian.thomas@gsfc.nasa.gov>                             *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codeviewerdialog.h"

// local includes
#include "codedocument.h"
#include "classifiercodedocument.h"
#include "codeeditor.h"
#include "debug_utils.h"
#include "uml.h"

// qt/kde includes
#include <klocale.h>
#include <QtCore/QString>
#include <QtGui/QTabWidget>

CodeViewerDialog::CodeViewerDialog ( QWidget* parent, CodeDocument * doc,
                                     Settings::CodeViewerState state,
                                     const char* name, bool modal, Qt::WFlags fl )
        : KDialog (parent, fl), m_state(state)
{
    setObjectName(name);
    uDebug() << "setObjectName(" << name << ")";
    setModal(modal);
    setButtons(KDialog::Cancel);
    setupUi(mainWidget());
    setInitialSize(QSize(630, 730));
    initGUI(name);
    addCodeDocument(doc);
    connect(this, SIGNAL(cancelClicked()), mainWidget(), SLOT(close()));
}

CodeViewerDialog::~CodeViewerDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void CodeViewerDialog::initGUI ( const char * name)
{
    if ( !name ) {
        setObjectName( "CodeViewerDialog" );
    }

    setFont( state().font );

    ui_highlightCheckBox->setChecked( state().blocksAreHighlighted );
    ui_showHiddenCodeCB->setChecked ( state().showHiddenBlocks );

    int margin = fontMetrics().height();
    CodeViewerDialogBase::gridLayout->setMargin(margin);
}

/**
 * Adds a code document to the tabbed output.
 */
void CodeViewerDialog::addCodeDocument(CodeDocument * doc)
{
    CodeEditor * page = new CodeEditor(this, doc);
    QString name = doc->getFileName();
    QString ext = doc->getFileExtension();
    uDebug() << "name=" << name << " / ext=" << ext;
    ui_tabWidget->addTab(page, (name + (ext.isEmpty() ? "" : ext)));

    connect( ui_highlightCheckBox, SIGNAL(stateChanged(int)), page, SLOT(changeHighlighting(int)));
    connect( ui_showHiddenCodeCB, SIGNAL(stateChanged(int)), page, SLOT(changeShowHidden(int)));
}

/**
 * Return the code viewer state.
 */
Settings::CodeViewerState CodeViewerDialog::state()
{
    return m_state;
}

bool CodeViewerDialog::close()
{
    // remember widget size for next time
    m_state.height = height() / fontMetrics().lineSpacing();
    m_state.width = width() / fontMetrics().maxWidth();
    // remember block highlighting
    m_state.blocksAreHighlighted = ui_highlightCheckBox->isChecked();
    // remember block show status
    m_state.showHiddenBlocks = ui_showHiddenCodeCB->isChecked();
    // run superclass close now
    return KDialog::close();
}

/**
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CodeViewerDialog::languageChange()
{
    Uml::ProgrammingLanguage pl = UMLApp::app()->activeLanguage();
    setWindowTitle( tr2i18n( "Code Viewer - " ) + pl.toString() );
}

#include "codeviewerdialog.moc"
