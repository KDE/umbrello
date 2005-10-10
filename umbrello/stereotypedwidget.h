/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 *  copyright (C) 2003-2004                                                *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

#ifndef STEREOTYPEDWIDGET_H
#define STEREOTYPEDWIDGET_H

#include "umlwidget.h"

/**
 * Intermediate widget parent which adds stereotype to UMLWidget
 *
 * @short UMLWidget with stereotype
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class StereotypedWidget : public UMLWidget {
public:

    /**
     * Constructs a StereotypedWidget.
     *
     * @param view              The parent of this StereotypedWidget.
     * @param o         The UMLObject this will be representing.
     */
    StereotypedWidget(UMLView * view, UMLObject *o);

    /**
     * destructor
     */
    virtual ~StereotypedWidget();

    /**
     * Returns the status of whether to show StereoType.
     *
     * @return  True if stereotype is shown.
     */
    bool getShowStereotype() const;

    /**
     * Set the status of whether to show StereoType.
     *
     * @param _show             True if stereotype shall be shown.
     */
    virtual void setShowStereotype(bool _status);

    /**
     * Overrides standard method.
    virtual void draw(QPainter& p, int offsetX, int offsetY);
     */

    /**
     * Saves to the <packagewidget> XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Loads from a <packagewidget> XMI element.
     */
    bool loadFromXMI(QDomElement& qElement);

protected:
    // Data loaded/saved

    /**
     * Should it show the <<stereotype>> of the widget
     */
    bool m_bShowStereotype;

};

#endif
