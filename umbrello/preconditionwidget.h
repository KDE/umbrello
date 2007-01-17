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

#ifndef PRECONDITIONWIDGET_H
#define PRECONDITIONWIDGET_H

#include "umlwidget.h"
#include "worktoolbar.h"

#define PRECONDITION_MARGIN 5
#define PRECONDITION_WIDTH 30
#define PRECONDITION_HEIGHT 10

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
     * @param view              The parent of the widget.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    PreconditionWidget( UMLView * view, Uml::IDType id = Uml::id_None );


    /**
     *  destructor
     */
    virtual ~PreconditionWidget();

    /**
     * Overrides the standard paint event.
     */
    void draw(QPainter & p, int offsetX, int offsetY);


    /**
     * Show a properties dialog for an PreconditionWidget.
     *
     * @return  True if we modified the precondition.
     */
    bool showProperties();


    /**
     * Saves the widget to the <preconditionwidget> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the <preconditionwidget> XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

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
};

#endif
