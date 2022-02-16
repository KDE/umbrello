/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
