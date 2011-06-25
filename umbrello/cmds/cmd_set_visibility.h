/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_SETVISIBILITY_H
#define CMD_SETVISIBILITY_H

#include <QtGui/QUndoCommand>

#include "basictypes.h"

class UMLObject;

namespace Uml
{
    class CmdSetVisibility : public QUndoCommand
    {
        public:
            CmdSetVisibility(UMLObject* obj, Uml::Visibility  visibility);
            ~CmdSetVisibility();

            void redo();
            void undo();

        private:
            Uml::Visibility  m_oldVisibility;
            Uml::Visibility  m_visibility;
            UMLObject*       m_umlObject;
    };
}

#endif
