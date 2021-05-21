/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef INTERFACEWIDGET_H
#define INTERFACEWIDGET_H

#include "classifierwidget.h"

/**
 * Provides an interface widget
 *
 * The implementation is currently in @ref ClassifierWidget
 * and should be moved to this class for easier implementation
 *
 * @author Ralf Habacker
 */
class InterfaceWidget : public ClassifierWidget
{
public:
    InterfaceWidget(UMLScene * scene, UMLClassifier * c);
    InterfaceWidget(UMLScene * scene, UMLPackage * p);
};

#endif // INTERFACEWIDGET_H
