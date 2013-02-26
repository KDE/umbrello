/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2013                                               *
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
 *
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ForkJoinWidget : public BoxWidget
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
public:
    explicit ForkJoinWidget(UMLScene * scene, Qt::Orientation ori = Qt::Horizontal, Uml::ID::Type id = Uml::ID::None);
    virtual ~ForkJoinWidget();

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation ori);

    virtual void draw(QPainter & p, int offsetX, int offsetY);

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    UMLSceneSize minimumSize();

    void drawSelected(QPainter * p, int offsetX, int offsetY);

    void constrain(UMLSceneValue& width, UMLSceneValue& height);

private:
    Qt::Orientation m_orientation;   ///< whether to draw the plate horizontally or vertically
};

#endif
