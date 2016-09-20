/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

 /* Created By Krzywda Stanislas and Bouchikhi Mohamed-Amine ;) */

#ifndef CMDCHANGEFILLCOLOR_H
#define CMDCHANGEFILLCOLOR_H

#include "cmdbasewidgetcommand.h"

#include <QColor>

class UMLWidget;

namespace Uml
{
    class CmdChangeFillColor : public CmdBaseWidgetCommand
    {
        public:
            CmdChangeFillColor(UMLWidget* w, const QColor& col);
            ~CmdChangeFillColor();

            void redo();
            void undo();

        private:
            QColor        m_oldColor;  ///< old color
            QColor        m_color;     ///< new color

    };
}

#endif /*CMDCHANGEFILLCOLOR_H*/
