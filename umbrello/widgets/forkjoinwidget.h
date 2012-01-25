/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef FORKJOINWIDGET_H
#define FORKJOINWIDGET_H

//app includes
#include "boxwidget.h"

/**
 * @short Displays a fork/join plate in a state diagram.
 *
 * @author Oliver Kellogg  <okellogg@users.sourceforge.net>
 * @author Gopala Krishna
 *
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ForkJoinWidget : public BoxWidget
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
public:

    explicit ForkJoinWidget(Qt::Orientation o = Qt::Horizontal, Uml::IDType id = Uml::id_None);
    virtual ~ForkJoinWidget();

    /// @return Whether to draw plate vertically or not.
    Qt::Orientation orientation() const {
        return m_orientation;
    }
    void setOrientation(Qt::Orientation o);

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    virtual void updateGeometry();

private:
    /// whether to draw the plate horizontally or vertically
    Qt::Orientation m_orientation;
};

#endif
