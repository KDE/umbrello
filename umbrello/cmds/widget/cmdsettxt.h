/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
