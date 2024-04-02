/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef BOXWIDGET_H
#define BOXWIDGET_H

//app includes
#include "umlwidget.h"

/**
 * Displays a rectangular box.
 * These widgets are diagram specific.  They will still need a unique id
 * from the @ref UMLDoc class for deletion and other purposes.
 *
 * @short Displays a box.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class BoxWidget : public UMLWidget
{
public:
    explicit BoxWidget(UMLScene * scene, Uml::ID::Type id = Uml::ID::None, WidgetType type = WidgetBase::wt_Box);
    virtual ~BoxWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual bool showPropertiesDialog();

protected:
    void toForeground();
};

#endif
