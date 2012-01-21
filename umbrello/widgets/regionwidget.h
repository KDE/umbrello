/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
***************************************************************************/

#ifndef REGIONWIDGET_H
#define REGIONWIDGET_H
#include <qpainter.h>
#include <qstringlist.h>
#include "umlwidget.h"
#include "worktoolbar.h"
#include "floatingtextwidget.h"

#define REGION_MARGIN 5
#define REGION_WIDTH 90
#define REGION_HEIGHT 45

class RegionWidget: public UMLWidget {
    Q_OBJECT
public:

    /**
     * Creates a Region widget.
     *
     * @param scene              The parent of the widget.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    explicit RegionWidget( UMLScene * scene, Uml::IDType id = Uml::id_None );

    /**
     * destructor
     */
    virtual ~RegionWidget();

    /**
     * Overrides the standard paint event.
     */
    void paint(QPainter & p, int offsetX, int offsetY);

    /**
     * Sets the name of the REGION.
     */
    virtual void setName(const QString &strName);

    /**
     * Returns the name of the Region.
     */
    virtual QString getName() const;


    /**
     * Creates the "REGIONwidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads a "REGIONwidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    UMLSceneSize minimumSize();

};

#endif
