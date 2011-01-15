/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003                                                    *
 *   Brian Thomas <brian.thomas@gsfc.nasa.gov>                             *
 *   copyright (C) 2004-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEVIEWERDIALOG_H
#define CODEVIEWERDIALOG_H

#include "codeviewerstate.h"
#include "ui_codeviewerdialogbase.h"

class CodeDocument;


/**
 * This class is sooo ugly I don't know where to begin. For now, its a prototype
 * that works, and thats all we need. In the future, a re-write is mandated to
 * bring a bit of beauty to this beast. -b.t.
 */
class CodeViewerDialog : public KDialog, private Ui::CodeViewerDialogBase
{
    Q_OBJECT
public:

    CodeViewerDialog ( QWidget* parent, CodeDocument * doc, Settings::CodeViewerState state,
                       const char* name = 0, bool modal = false, Qt::WFlags fl = 0 );

    ~CodeViewerDialog ();

    Settings::CodeViewerState state( );

    void addCodeDocument( CodeDocument * doc);

protected:

    bool close ();

private:

    friend class CodeEditor;

    Settings::CodeViewerState m_state;

    void initGUI ( const char * name );

public slots:

protected slots:

    virtual void languageChange();

};


#endif // CODEVIEWERDIALOG_H
