/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_RENAME_UMLOBJECT_H
#define CMD_RENAME_UMLOBJECT_H

#include <QtGui/QUndoCommand>

#include "umlobject.h"

namespace Uml
{
    class cmdRenameUMLObject : public QUndoCommand
    {
        public:
            cmdRenameUMLObject(UMLObject* o, const QString& name);
            ~cmdRenameUMLObject();

            void redo();
            void undo();

        private:
            UMLObject*  m_obj;
            QString     m_name;
            QString     m_oldname;
    };
}

#endif
