/*
    SPDX-License-Identifier: GPL-2.0-or-later

    copyright (C) 2002-2020
    Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    void saveToXMI1(QXmlStreamWriter& writer);
    bool loadFromXMI1(QDomElement& qElement);

protected:
    QSizeF minimumSize() const;

};

#endif
