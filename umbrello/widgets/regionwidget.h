/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
***************************************************************************/

#ifndef REGIONWIDGET_H
#define REGIONWIDGET_H

#include "umlwidget.h"

/**
 * Represents a rectangular region on Activity diagram.
 */
class RegionWidget: public UMLWidget
{
    Q_OBJECT
public:
    explicit RegionWidget(UMLScene* scene, Uml::ID::Type id = Uml::ID::None);
    virtual ~RegionWidget();

    void draw(QPainter& p, int offsetX, int offsetY);

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    bool loadFromXMI(QDomElement& qElement);

protected:
    UMLSceneSize minimumSize();

};

#endif
