/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_CREATE_ENTITYRELATIONDIAG_H
#define CMD_CREATE_ENTITYRELATIONDIAG_H

#include <QtGui/QUndoCommand>

class UMLDoc;
class UMLView;

namespace Uml
{
    class CmdCreateEntityRelationDiag : public QUndoCommand
    {
        public:
            explicit CmdCreateEntityRelationDiag(UMLDoc* doc, const QString& name = "");
            ~CmdCreateEntityRelationDiag();

            void redo();
            void undo();

        private:
            UMLDoc*     m_pUMLDoc;
            UMLView*    m_pUMLView;
            QString     m_Name;
    };
}

#endif
