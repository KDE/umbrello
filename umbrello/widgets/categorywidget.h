/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CATEGORYWIDGET_H
#define CATEGORYWIDGET_H

#include "umlwidget.h"

class UMLCategory;

/**
 * This class is the graphical version of a UMLCategory.  The
 * CategoryWidget class inherits from the @ref UMLWidget class
 * which adds most of the functionality to this class.
 *
 * @short  A graphical version of a UMLCategory.
 * @author Sharan Rao
 * @author Gopala Krishna
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class CategoryWidget : public UMLWidget
{
    Q_OBJECT
public:
    CategoryWidget(UMLCategory *o);
    virtual ~CategoryWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    // For loading , UMLWidget::loadFromXMI() is used.
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    virtual void updateGeometry();

public Q_SLOTS:
    void slotMenuSelection(QAction* action);

};

#endif
