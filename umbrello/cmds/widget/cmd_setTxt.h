/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_SETTXT_H
#define CMD_SETTXT_H

#include <QtGui/QUndoCommand>

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
