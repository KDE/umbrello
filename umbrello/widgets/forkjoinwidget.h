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

//qt includes
#include <QtGui/QPainter>

// fwd decl.
class UMLScene;

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

    /**
     * Set whether to draw the plate vertically.
     */
    void setDrawVertical(bool to);
    /**
     * Get whether to draw the plate vertically.
     */
    bool getDrawVertical() const;

    void slotMenuSelection(QAction* action);

    void paint(QPainter & p, int offsetX, int offsetY);

    bool loadFromXMI(QDomElement & qElement);
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    void drawSelected(QPainter * p, int offsetX, int offsetY);

    QSize calculateSize();

    void constrain(int& width, int& height);

private:
    void init();

    bool m_drawVertical;   ///< whether to draw the plate horizontally or vertically
};

#endif
