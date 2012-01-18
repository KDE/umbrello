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
 * @author Oliver Kellogg  <okellogg@users.sourceforge.net>
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ForkJoinWidget : public BoxWidget
{
public:
    explicit ForkJoinWidget(UMLScene * scene, bool drawVertical = false, Uml::IDType id = Uml::id_None);
    virtual ~ForkJoinWidget();

    bool getDrawVertical() const;
    void setDrawVertical(bool to);

    virtual void paint(QPainter & p, int offsetX, int offsetY);

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    UMLSceneSize minimumSize();

    void drawSelected(QPainter * p, int offsetX, int offsetY);

    void constrain(int& width, int& height);

private:
    bool m_drawVertical;   ///< whether to draw the plate horizontally or vertically
};

#endif
