/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PINWIDGET_H
#define PINWIDGET_H

#include "umlwidget.h"
#include "worktoolbar.h"
#include "activitywidget.h"
#include "floatingtextwidget.h"

#define PIN_MARGIN 5
#define PIN_WIDTH 1
#define PIN_HEIGHT 1

//class ActivityWidget;

/**
 * This class is the graphical version of a UML Pin.  A pinWidget is created
 * by a @ref UMLView.  An pinWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The pinWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML pin.
 * @author Hassan KOUCH <hkouch@hotmail.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PinWidget : public UMLWidget {
    Q_OBJECT

public:

    /**
     * Creates a Pin widget.
     *
     * @param scene              The parent of the widget.
     * @param a                 The widget to which this pin is attached.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
     PinWidget( UMLScene * scene, UMLWidget* a, Uml::IDType id = Uml::id_None );

    /**
     *  destructor
     */
    virtual ~PinWidget();

    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Overrides the standard paint event.
     */
    void paint(QPainter & p, int offsetX, int offsetY);


    /**
     * Sets the name of the pin.
     */
    virtual void setName(const QString &strName);

    /**
     * Returns the minimum height this widget should be set at on
     * a sequence diagrams.  Takes into account the widget positions
     * it is related to.
     */
    int getMinY();

    /**
     * Returns the maximum height this widget should be set at on
     * a sequence diagrams.  Takes into account the widget positions
     * it is related to.
     */
    int getMaxY();

    /**
     * Saves the widget to the "pinwidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the "pinwidget" XMI element.
     */
     bool loadFromXMI( QDomElement & qElement );

    /**
     * Overrides mouseMoveEvent.
     */
    void mouseMoveEvent(QMouseEvent *me);

public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
    void slotMenuSelection(QAction* action);


private:
    UMLWidget * m_pOw;

    /**
     * This is a pointer to the Floating Text widget which displays the
     * name of the signal widget.
     */
    FloatingTextWidget * m_pName;

    /**
     * Save the value of the widget to know how to move the floatingtext
     */
    int m_oldX;
    int m_oldY;

    int m_nY;
};

#endif
