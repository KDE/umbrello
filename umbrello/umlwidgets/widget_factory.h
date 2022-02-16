/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef WIDGET_FACTORY_H
#define WIDGET_FACTORY_H

#include <QString>

// forward declarations
class UMLObject;
class UMLScene;
class UMLWidget;

/**
 * Widget factory methods.
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
namespace Widget_Factory {

    UMLWidget *createWidget(UMLScene *scene, UMLObject *docObj);

    UMLWidget* makeWidgetFromXMI(const QString& tag,
                                 const QString& idStr, UMLScene *scene);

}   // end namespace Widget_Factory

#endif
