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

#ifndef ENDOFLIFEWIDGET_H
#define ENDOFLIFEWIDGET_H

#include "umlwidget.h"
#include "worktoolbar.h"

#define ENDOFLIFE_MARGIN 5
#define ENDOFLIFE_WIDTH 30
#define ENDOFLIFE_HEIGHT 10

class ObjectWidget;
class UMLOperation;

/**
 * This class is the graphical version of a UML End of life  A EndOfLifeWidget is created
 * by a @ref UMLView.  An EndOfLifeWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The EndOfLifeWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 * 
 * A EndOfLifeWidgetWidget is used to show that an object in sequence diagram is dead
 *
 * @short  A graphical version of a UML EndOfLifeWidget (new in UML 2.0).
 * @author Florence Mattler <florence.mattler@libertysurf.fr>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class EndOfLifeWidget : public UMLWidget {
    Q_OBJECT

public:
   
    /**
     * Creates a End Of Life widget.
     *
     * @param view              The parent of the widget.
     * @param a			The role A widget for this precondition.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    EndOfLifeWidget( UMLView * view, ObjectWidget* a, Uml::IDType id = Uml::id_None );


    /**
     *  destructor
     */
    virtual ~EndOfLifeWidget();

    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Overrides the standard paint event.
     */
    void draw(QPainter & p, int offsetX, int offsetY);


    /**
     * Calculate the geometry of the widget.
     */
    void calculateWidget();

    /**
     * Activates a EndOflifeWidget.  Connects it m_pOw[] pointer
     * to UMLObject
     */
    void activate(IDChangeLog * Log = 0);

    /**
     * Calculates the size of the widget by calling
     */
     void calculateDimensions();


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
     * Saves the widget to the <endoflifewidget> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the <endoflifewidget> XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

	ObjectWidget * m_pOw[1];

protected:
    /**
     * Overrides method from UMLWidget
     */
    QSize calculateSize();

   
public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
    void slotMenuSelection(int sel);
    void slotWidgetMoved(Uml::IDType id);

private:
    
    int m_nY;
};

#endif
