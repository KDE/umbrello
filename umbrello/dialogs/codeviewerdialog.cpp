/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003-2020 Brian Thomas <brian.thomas@gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codeviewerdialog.h"

// local includes
#include "codedocument.h"
#include "classifiercodedocument.h"
#include "codeeditor.h"
#include "debug_utils.h"
#include "uml.h"

// qt/kde includes
#include <KLocalizedString>
#include <QString>
#include <QTabWidget>
#include <QPushButton>
#include <QDialogButtonBox>

DEBUG_REGISTER(CodeViewerDialog)

CodeViewerDialog::CodeViewerDialog (QWidget* parent, CodeDocument * doc,
                                     Settings::CodeViewerState state)
  : SinglePageDialogBase(parent), m_state(state)
{
    setModal(false);
    setupUi(SinglePageDialogBase::mainWidget());
    initGUI();
    addCodeDocument(doc);
    connect(SinglePageDialogBase::m_buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), mainWidget(), SLOT(close()));
}

CodeViewerDialog::~CodeViewerDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void CodeViewerDialog::initGUI()
{
    setFont(state().font);

    ui_highlightCheckBox->setChecked(state().blocksAreHighlighted);
    ui_showHiddenCodeCB->setChecked (state().showHiddenBlocks);

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
    logDebug2("CodeViewerDialog::addCodeDocument: name=%1 / ext=%2", name, ext);
    ui_tabWidget->addTab(page, (name + (ext.isEmpty() ? QString() : ext)));

    connect(ui_highlightCheckBox, SIGNAL(stateChanged(int)), page, SLOT(changeHighlighting(int)));
    connect(ui_showHiddenCodeCB, SIGNAL(stateChanged(int)), page, SLOT(changeShowHidden(int)));
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
    return QDialog::close();
}

/**
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CodeViewerDialog::languageChange()
{
    Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
    setWindowTitle(i18n("Code Viewer - %1", Uml::ProgrammingLanguage::toString(pl)));
}

