/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_CREATE_COMPONENTDIAG_H
#define CMD_CREATE_COMPONENTDIAG_H

#include <QtGui/QUndoCommand>

class UMLDoc;
class UMLView;

namespace Uml
{
    class CmdCreateComponentDiag : public QUndoCommand
    {
        public:
            explicit CmdCreateComponentDiag(UMLDoc* doc, const QString& name = "");
            ~CmdCreateComponentDiag();

            void redo();
            void undo();

        private:
            UMLDoc*     m_pUMLDoc;
            UMLView*    m_pUMLView;
            QString     m_Name;
    };
}

#endif
