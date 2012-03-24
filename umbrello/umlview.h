/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLVIEW_H
#define UMLVIEW_H

#include "umlscene.h"

/**
 * UMLView instances represent diagrams.
 * The UMLApp instance manages a QWidgetStack of UMLView instances.
 * The visible diagram is at the top of stack.
 * The UMLView class inherits from QCanvasView and it owns the
 * objects displayed on its related QCanvas (see m_WidgetList.)
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLView : public Q3CanvasView
{
    Q_OBJECT
public:
    UMLView(UMLFolder *parentFolder);
    virtual ~UMLView();

    /**
     * Hack for reducing the difference
     * between the new QGraphicsScreen port.
     */
    UMLScene* umlScene() {
        return m_scene;
    }

    /**
    * Returns the zoom of the diagram.
    */
    int zoom() const {
        return m_nZoom;
    }

    void setZoom(int zoom);

    int currentZoom();

public slots:
    void zoomIn();
    void zoomOut();

protected:
    virtual void closeEvent(QCloseEvent * e);

    UMLScene *m_scene;

    /**
     * The zoom level in percent, default 100
     */
    int m_nZoom;
};

#endif // UMLVIEW_H
