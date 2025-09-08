/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "interfacewidget.h"

#include "umlclassifier.h"
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
