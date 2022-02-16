/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

 /* Created By Bouchikhi Mohamed-Amine */

#ifndef CMDCHANGEMULTIPLICITY_H
#define CMDCHANGEMULTIPLICITY_H

#include <QUndoCommand>

class UMLRole;

namespace Uml
{
    class CmdChangeMultiplicity : public QUndoCommand
    {
    public:
        CmdChangeMultiplicity(UMLRole *role, const QString &multi);
        void redo();
        void undo();

    private:
        UMLRole* m_umlRole;   ///< Object selected
        QString m_oldMulti;   ///< Old multiplicity
        QString m_newMulti;   ///< New multiplicity
    };

}

#endif // CMDCHANGEMULTI_H

