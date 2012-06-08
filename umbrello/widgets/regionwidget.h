/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
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
    explicit RegionWidget(UMLScene* scene, Uml::IDType id = Uml::id_None);
    virtual ~RegionWidget();

    void paint(QPainter& p, int offsetX, int offsetY);

    virtual void setName(const QString& strName);

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    bool loadFromXMI(QDomElement& qElement);

protected:
    UMLSceneSize minimumSize();

};

#endif
