/***************************************************************************
 *                                                                         *
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

#define PIN_MARGIN 5
#define PIN_WIDTH 1
#define PIN_HEIGHT 1

//class ActivityWidget;
class UMLOperation;

/**
 * This class is the graphical version of a UML Object Flow.  A ObjectFlowWidget is created
 * by a @ref UMLView.  An ObjectFlowWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The ObjectFlowWidget class inherits from the @ref UMLWidget class which adds most of the functionality
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
     * Creates a Object Flow widget.
     *
     * @param view              The parent of the widget.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
     PinWidget( UMLView * view, ActivityWidget* a, Uml::IDType id = Uml::id_None );

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
    void draw(QPainter & p, int offsetX, int offsetY);
/*
     * Activates a PreconditionWidget.  Connects it m_pOw[] pointer
     * to UMLObject
     */


//    void activate(IDChangeLog * Log = 0);

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
     * Saves the widget to the <objectflowwidget> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the <objectflowwidget> XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    //  QSize calculateSize();

    /**
     * Type of ObjectFlow.
     */
  // ObjectFlow m_ObjectFlowType;

//public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
   // void slotMenuSelection(int sel);

private:
    ActivityWidget * m_pOw[1];
   int m_nY;
};

#endif
