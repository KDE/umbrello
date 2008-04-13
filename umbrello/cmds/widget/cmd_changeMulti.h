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

 /* Created By Bouchikhi Mohamed-Amine */

#ifndef CMD_CHANGEMULTI_H
#define CMD_CHANGEMULTI_H


#include <QtGui/QUndoCommand>

#include "umlwidget.h"
#include "umlrole.h"

namespace Uml
{
    class cmdChangeMulti: public QUndoCommand
    {
        /* Object selected */
        UMLRole *UMLr;

        /* Old multiplicity */
        QString oldMulti;

        /* New multiplicity */
        QString newMulti;


    public:
        cmdChangeMulti(UMLRole *role,const QString &multi);
        void redo();
        void undo();
    };

}

#endif /*CMD_CHANGEMULTI_H*/

