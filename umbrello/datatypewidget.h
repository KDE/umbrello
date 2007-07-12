/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DATATYPEWIDGET_H
#define DATATYPEWIDGET_H

#include "umlwidget.h"

class UMLClassifier;

#define DATATYPE_MARGIN 5

/**
 * Defines a graphical version of the datatype.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of an datatype.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class DatatypeWidget : public UMLWidget {
public:

    /**
     * Constructs an DatatypeWidget.
     *
     * @param view              The parent of this DatatypeWidget.
     * @param d         The UMLClassifier this will be representing.
     */
    DatatypeWidget(UMLView* view, UMLClassifier *d);

    /**
     * Standard deconstructor.
     */
    virtual ~DatatypeWidget();

    /**
     * Overrides standard method.
     */
    void draw(QPainter& p, int offsetX, int offsetY);

    /**
     * Saves to the "datatypewidget" XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Loads from a "datatypewidget" XMI element.
     */
    bool loadFromXMI(QDomElement& qElement);

protected:
    /**
     * Overrides method from UMLWidget.
     */
    QSize calculateSize();

private:
    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * The right mouse button menu.
     */
    ListPopupMenu* m_pMenu;

public slots:
};

#endif
