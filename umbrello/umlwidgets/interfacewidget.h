/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
