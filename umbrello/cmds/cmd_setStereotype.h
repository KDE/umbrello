/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_SETSTEREOTYPE_H
#define CMD_SETSTEREOTYPE_H

#include <QtGui/QUndoCommand>

class UMLObject;

namespace Uml
{
    class CmdSetStereotype : public QUndoCommand
    {
        public:
            CmdSetStereotype(UMLObject * obj, const QString& stereo);
            ~CmdSetStereotype();

            void redo();
            void undo();

        private:
            QString    m_stereo;
            QString    m_oldStereo;
            UMLObject* m_umlObject;
    };
}

#endif
