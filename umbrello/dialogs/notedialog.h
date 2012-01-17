/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef NOTEDIALOG_H
#define NOTEDIALOG_H

// app includes
#include "notewidget.h"

// kde includes
#include <kdialog.h>

class KTextEdit;
class QGroupBox;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NoteDialog : public KDialog
{
   Q_OBJECT

public:

    /**
     *  Constructs an NoteDialog.
     */
    NoteDialog( QWidget * parent, NoteWidget * pNote );

    /**
     *  Standard deconstructor.
     */
    ~NoteDialog();

public slots:
    void slotOk();

private:
    //GUI widgets
    QGroupBox * m_docGB;
    KTextEdit * m_docTE;

    /**
     *  Note widget to show documentation for.
     */
    NoteWidget * m_pNoteWidget;
};

#endif
