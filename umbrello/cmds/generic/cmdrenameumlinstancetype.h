/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2017                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_RENAME_UMLINSTANCE_TYPE_H
#define CMD_RENAME_UMLINSTANCE_TYPE_H

#include <QPointer>
#include <QUndoCommand>

#include <KLocalizedString>

class UMLInstance;
class UMLClassifier;

namespace Uml
{
    class CmdRenameUMLInstanceType : public QUndoCommand
    {
    public:
        CmdRenameUMLInstanceType(UMLInstance* o, UMLClassifier *type);
        ~CmdRenameUMLInstanceType();

        void redo();
        void undo();

    private:
        QPointer<UMLInstance>  m_obj;
        QPointer<UMLClassifier> m_type;
        QPointer<UMLClassifier> m_oldType;
    };
}

#endif
