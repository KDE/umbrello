/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDCREATEWIDGET_H
#define CMDCREATEWIDGET_H

#include "cmdbasewidgetcommand.h"

#include <QDomElement>

class AssociationWidget;
class UMLWidget;

namespace Uml
{
    class CmdCreateWidget : public CmdBaseWidgetCommand
    {
    public:
        explicit CmdCreateWidget(UMLWidget *widget);
        CmdCreateWidget(AssociationWidget *widget);
        ~CmdCreateWidget();

        void redo();
        void undo();

    private:
        QDomElement  m_element;
    };
}

#endif
