/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
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
    DatatypeWidget(UMLScene *scene, UMLClassifier *d);
    virtual ~DatatypeWidget();

    void paint(QPainter& p, int offsetX, int offsetY);

    bool loadFromXMI(QDomElement& qElement);
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

public Q_SLOTS:

protected:
    UMLSceneSize minimumSize();

private:
    /**
     * The right mouse button menu.
     */
    ListPopupMenu* m_pMenu;

};

#endif
