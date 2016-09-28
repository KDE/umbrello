/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMDSETTXT_H
#define CMDSETTXT_H

#include <QUndoCommand>

class FloatingTextWidget;

namespace Uml
{
    class CmdSetTxt : public QUndoCommand
    {
        public:
            CmdSetTxt(FloatingTextWidget* ftw, const QString& txt);
            ~CmdSetTxt();

            void redo();
            void undo();

        private:
            FloatingTextWidget* m_ftw;
            QString m_newstring;
            QString m_oldstring;
    };
}

#endif
