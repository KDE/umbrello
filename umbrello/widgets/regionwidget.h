/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
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
public:
    explicit RegionWidget(Uml::IDType id = Uml::id_None );
    virtual ~RegionWidget();

    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    bool loadFromXMI( QDomElement & qElement );
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

private:
    static const QSizeF MinimumSize;
};
#endif
