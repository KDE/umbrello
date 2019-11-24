/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef PINWIDGET_H
#define PINWIDGET_H

#include "pinportbase.h"

/**
 * This class is the graphical version of a UML Pin. A pinWidget is created
 * by a @ref UMLView.  An pinWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The pinWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML pin.
 * @author Hassan KOUCH <hkouch@hotmail.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class PinWidget : public PinPortBase
{
    Q_OBJECT
public:

    PinWidget(UMLScene* scene, UMLWidget* owner, Uml::ID::Type id = Uml::ID::None);
    virtual ~PinWidget();

    // int getMinY();

public slots:
    void slotMenuSelection(QAction* action);

// private:
    // int m_nY;
};

#endif
