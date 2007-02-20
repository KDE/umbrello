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

#ifndef EXPANSION_REGIONWIDGET_H
#define EXPANSION_REGIONWIDGET_H
#include <qpainter.h>
#include <qstringlist.h>
#include "umlwidget.h"
#include "worktoolbar.h"
#include "floatingtextwidget.h"

#define EXPANSION_REGION_MARGIN 5
#define EXPANSION_REGION_WIDTH 90
#define EXPANSION_REGION_HEIGHT 45

class ExpansionRegionWidget: public UMLWidget {
    Q_OBJECT
public:

    /**
     * Creates a ExpansionRegion widget.
     *
     * @param view              The parent of the widget.
     * @param EXPANSION_REGIONType        The type of ExpansionRegion.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    ExpansionRegionWidget( UMLView * view, Uml::IDType id = Uml::id_None );

    /**
     * destructor
     */
    virtual ~ExpansionRegionWidget();

    /**
     * Overrides the standard paint event.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

    /**
     * Sets the name of the EXPANSION_REGION.
     */
    virtual void setName(const QString &strName);

    /**
     * Returns the name of the ExpansionRegion.
     */
    virtual QString getName() const;

    /**
     * Show a properties dialog for a ExpansionRegion.
     *
     * @return  True if we modified the EXPANSION_REGION.
     */
    bool showProperties();

    /**
     * Creates the <EXPANSION_REGIONwidget> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads a <EXPANSION_REGIONwidget> XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    QSize calculateSize();

};

#endif
