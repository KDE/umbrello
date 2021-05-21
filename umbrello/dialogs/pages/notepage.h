/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
