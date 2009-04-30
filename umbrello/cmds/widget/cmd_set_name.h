/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_SETNAME_H
#define CMD_SETNAME_H

#include <QtGui/QUndoCommand>

class UMLObject;

namespace Uml
{
    class CmdSetName : public QUndoCommand
    {
        public:
            CmdSetName(UMLObject* obj, const QString& name);
            ~CmdSetName();

            void redo();
            void undo();

        private:
            QString m_oldname;
            UMLObject* m_umlObject;
            QString m_name;
    };
}

#endif
