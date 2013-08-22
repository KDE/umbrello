/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef NOTEDIALOG_H
#define NOTEDIALOG_H

// kde includes
#include <kdialog.h>

class NoteWidget;
class KTextEdit;
class QGroupBox;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class NoteDialog : public KDialog
{
   Q_OBJECT
public:
    NoteDialog(QWidget * parent, NoteWidget * pNote);
    ~NoteDialog();

public slots:
    void slotOk();

private:
    QGroupBox * m_docGB;  ///< GUI widget
    KTextEdit * m_docTE;  ///< GUI widget
    NoteWidget * m_pNoteWidget;  ///< note widget to show documentation for
};

#endif
