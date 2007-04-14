/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef __CMD_CREATE_UMLOBJECT__
#define __CMD_CREATE_UMLOBJECT__

#include <QUndoCommand>

#include "umlobject.h"

namespace Uml
{
    class cmdCreateUMLObject : public QUndoCommand
    {
        public:
            cmdCreateUMLObject(UMLObject* o);
            ~cmdCreateUMLObject();

            void redo();
            void undo();

        private:
            UMLObject*  m_obj;
    };
}

#endif
