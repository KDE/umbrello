/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLVIEW_H
#define UMLVIEW_H

#include <QGraphicsView>

class QHideEvent;
class QCloseEvent;
class QShowEvent;
class UMLScene;
class UMLFolder;
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
class UMLView : public QGraphicsView
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    UMLView(UMLFolder *f );

    /**
     * Destructor
     */
    virtual ~UMLView();

    /**
     * Returns the zoom of the diagram.
     */
    int getZoom() const {
        return m_nZoom;
    }

    /**
     * Sets the zoom of the diagram.
     */
    void setZoom(int zoom);

    /**
     * return the current zoom factor
     */
    int currentZoom();

    /**
     * Overrides the standard operation.
     */
    void hideEvent(QHideEvent *he);

    /**
     * Overrides the standard operation.
     */
    void showEvent(QShowEvent *se);

    UMLScene* umlScene() const;

protected:

    /**
     * The zoom level in percent, default 100
     */
    int m_nZoom;

    /**
     * Override standard method.
     */
    void closeEvent ( QCloseEvent * e );

public slots:

    void zoomIn();
    void zoomOut();

    /**
     * Overrides standard method from QWidget to resize canvas when
     * it's shown.
     */
    void show();
};

#endif // UMLVIEW_H
