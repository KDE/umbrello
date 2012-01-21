/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef FLOATINGDASHLINEWIDGET_H
#define FLOATINGDASHLINEWIDGET_H

#include "umlwidget.h"
#define FLOATING_DASH_LINE_MARGIN 25
#define FLOATING_DASH_LINE_TEXT_MARGIN 5

/* how many pixels a user could click around a point */
#define POINT_DELTA 5


/**
 * This class is used to draw dash lines for UML combined fragments. A FloatingDashLineWidget
 * belongs to one @ref CombinedFragmentWidget instance.
 *
 * The FloatingDashLineWidget class inherits from the @ref UMLWidget class.
 *
 * @short  A dash line for UML combined fragments.
 * @author Thomas GALLINARI <tg8187@yahoo.fr>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class FloatingDashLineWidget : public UMLWidget {
    Q_OBJECT
public:
    /**
     * Creates a floating dash line.
     * @param scene              The parent of the widget
     * @param id                The ID to assign (-1 will prompt a new ID)
     */
    explicit FloatingDashLineWidget(UMLScene * scene, Uml::IDType id = Uml::id_None);

    /**
     * destructor
     */
    ~FloatingDashLineWidget();

    /**
     * Overrides the standard paint event.
     */
    void paint(QPainter & p, int offsetX, int offsetY);

    void slotMenuSelection(QAction* action);
     /**
     * Returns true if the given point is near the floatingdashline
     */
     bool onLine(const QPoint & point);

    /**
     * Sets m_text
     */
    void setText(const QString& text);

    /**
     * Overrides the setY method
     */
    void setY(int y);

    /**
     * Sets m_yMin
     */
    void setYMin(int yMin);

    /**
     * Sets m_yMax
     */
    void setYMax(int yMax);

    /**
     * Returns m_yMin
     */
    int getYMin();

    /**
     * Returns the difference between the y-coordinate of the dash line and m_yMin
     */
    int getDiffY();

     /**
     * Creates the "floatingdashline" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the "floatingdashline" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

private:
    /**
     * Text associated to the dash line
     */
    QString m_text;

    /**
     * Value added to the y-coordinate of the combined fragment
     * to obtain the y-coordinate of the dash line
     */
    int m_y;

    /**
     * Minimum value of the Y-coordinate of the dash line
     * (= y-coordinate of the combined fragment)
     */
    int m_yMin;

    /**
     * Maximum value of the Y-coordinate of the dash line
     * (= y-coordinate of the combined fragment + height of the combined fragment)
     */
    int m_yMax;
};

#endif
