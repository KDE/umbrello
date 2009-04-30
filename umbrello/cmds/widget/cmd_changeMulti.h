/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

 /* Created By Bouchikhi Mohamed-Amine */

#ifndef CMD_CHANGEMULTI_H
#define CMD_CHANGEMULTI_H

#include <QtGui/QUndoCommand>

class UMLRole;

namespace Uml
{
    class CmdChangeMulti : public QUndoCommand
    {
    public:
        CmdChangeMulti(UMLRole *role, const QString &multi);
        void redo();
        void undo();

    private:
        UMLRole* m_umlRole;   ///< Object selected
        QString m_oldMulti;   ///< Old multiplicity
        QString m_newMulti;   ///< New multiplicity
    };

}

#endif // CMD_CHANGEMULTI_H

