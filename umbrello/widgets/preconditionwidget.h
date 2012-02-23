/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PRECONDITIONWIDGET_H
#define PRECONDITIONWIDGET_H

#include "umlwidget.h"
#include "worktoolbar.h"

#define PRECONDITION_MARGIN 5
#define PRECONDITION_WIDTH 30
#define PRECONDITION_HEIGHT 10

class ObjectWidget;

/**
 * This class is the graphical version of a UML Precondition.  A PreconditionWidget is created
 * by a @ref UMLView.  An PreconditionWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The PreconditionWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML Precondition (new in UML 2.0).
 * @author Florence Mattler <florence.mattler@libertysurf.fr>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PreconditionWidget : public UMLWidget {
    Q_OBJECT

public:

    /**
     * Creates a Precondition widget.
     *
     * @param scene              The parent of the widget.
     * @param a                 The role A widget for this precondition.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    PreconditionWidget( UMLScene * scene, ObjectWidget* a, Uml::IDType id = Uml::id_None );


    /**
     *  destructor
     */
    virtual ~PreconditionWidget();

    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Overrides the standard paint event.
     */
    void paint(QPainter & p, int offsetX, int offsetY);


    /**
     * Calculate the geometry of the widget.
     */
    void calculateWidget();

    /**
     * Activates a PreconditionWidget.  Connects the WidgetMoved signal from
     * its m_pOw pointer so that PreconditionWidget can adjust to the move of
     * the object widget.
     */
    bool activate(IDChangeLog * Log = 0);

    /**
     * Calculates the size of the widget
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
     * Saves the widget to the "preconditionwidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the "preconditionwidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    UMLSceneSize minimumSize();


public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
    void slotMenuSelection(QAction* action);
    void slotWidgetMoved(Uml::IDType id);

private:
    ObjectWidget * m_pOw;
    int m_nY;
};

#endif
