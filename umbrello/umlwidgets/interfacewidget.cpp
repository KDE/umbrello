/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "interfacewidget.h"

#include "debug_utils.h"

DEBUG_REGISTER_DISABLED(InterfaceWidget)


InterfaceWidget::InterfaceWidget(UMLScene *scene, UMLClassifier *c)
  : ClassifierWidget(scene, c)
{

}

InterfaceWidget::InterfaceWidget(UMLScene *scene, UMLPackage *p)
  : ClassifierWidget(scene, p)
{

}
