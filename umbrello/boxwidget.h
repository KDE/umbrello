/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef BOXWIDGET_H
#define BOXWIDGET_H

//app includes
#include "newumlrectwidget.h"

//qt includes
#include <QtGui/QPainter>

class BoxWidget : public NewUMLRectWidget
{
public:

    explicit BoxWidget(Uml::IDType id = Uml::id_None);
    virtual ~BoxWidget();

    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

private:
    QSizeF m_minimumSize;
};

#endif
