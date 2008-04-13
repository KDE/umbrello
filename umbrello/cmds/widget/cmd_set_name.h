/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2008                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef CMD_SETNAME_H
#define CMD_SETNAME_H

#include <QtGui/QUndoCommand>

#include "umlwidget.h"

namespace Uml
{
    class cmdSetName : public QUndoCommand
    {
        public:
            /*Constructor */
            cmdSetName(UMLObject * _UMLObj, const QString& _name);
            /*Destructor */
            ~cmdSetName();
            /*Redo method */
            void redo();
            /*Undo method */
            void undo();

        private:

            QString oldname;
            UMLObject * UMLObj;
            QString name;
    };
}

#endif
