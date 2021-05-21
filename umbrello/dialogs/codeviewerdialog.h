/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003-2020 Brian Thomas <brian.thomas@gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEVIEWERDIALOG_H
#define CODEVIEWERDIALOG_H

#include "codeviewerstate.h"
#include "ui_codeviewerdialogbase.h"
#include "singlepagedialogbase.h"

class CodeDocument;

/**
 * This class is sooo ugly I don't know where to begin. For now, its a prototype
 * that works, and thats all we need. In the future, a re-write is mandated to
 * bring a bit of beauty to this beast. -b.t.
 */
class CodeViewerDialog : public SinglePageDialogBase, private Ui::CodeViewerDialogBase
{
    Q_OBJECT
public:

    CodeViewerDialog (QWidget* parent, CodeDocument * doc, Settings::CodeViewerState state);

    ~CodeViewerDialog ();

    Settings::CodeViewerState state();

    void addCodeDocument(CodeDocument * doc);

protected:

    bool close ();

private:

    friend class CodeEditor;

    Settings::CodeViewerState m_state;

    void initGUI();

public slots:

protected slots:

    virtual void languageChange();

};


#endif // CODEVIEWERDIALOG_H
