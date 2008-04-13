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

#ifndef CMD_CHANGEFONTSELECTION_H
#define CMD_CHANGEFONTSELECTION_H

#include <QtGui/QUndoCommand>

#include "umlwidget.h"

namespace Uml
{
    class cmdChangeFontSelection: public QUndoCommand
    {
        public:
            cmdChangeFontSelection(UMLDoc * doc,UMLView *view,QFont fon);
            void undo();
            void redo();

        private:
            UMLDoc *pDoc;
            UMLView *pView;

            QFont newFont;
            QFont oldFont;
    };
}
#endif /*CMD_CHANGEFONTSELECTION_H*/
