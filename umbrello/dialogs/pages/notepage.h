/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2018                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef NOTEPAGE_H
#define NOTEPAGE_H

#include "dialogpagebase.h"

class DocumentationWidget;
class NoteWidget;

/**
 * @author Ralf Habacker
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class NotePage : public DialogPageBase
{
   Q_OBJECT
public:
    NotePage(QWidget *parent, NoteWidget *note);
    ~NotePage();

    bool apply();

protected:
    DocumentationWidget *m_docWidget; ///< widget holding the documentation
    NoteWidget *m_noteWidget;  ///< note widget to show documentation for
};

#endif
