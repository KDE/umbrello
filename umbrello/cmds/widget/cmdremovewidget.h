/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMDREMOVEWIDGET_H
#define CMDREMOVEWIDGET_H

#include "cmdbasewidgetcommand.h"

#include <QDomElement>

class AssociationWidget;
class UMLWidget;

namespace Uml
{
    class CmdRemoveWidget : public CmdBaseWidgetCommand
    {
    public:
        explicit CmdRemoveWidget(UMLWidget* widget);
        CmdRemoveWidget(AssociationWidget *widget);
        ~CmdRemoveWidget();

        void redo();
        void undo();

    private:
        QDomElement m_element;
        QList<QDomElement> m_children;
    };
}

#endif
