/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                  *
 ***************************************************************************/

#ifndef UMLVIEW_H
#define UMLVIEW_H

#include <QGraphicsView>

class QCloseEvent;
class QHideEvent;
class QShowEvent;
class UMLFolder;
class UMLScene;

/**
 * UMLView instances represent diagrams.
 * The UMLApp instance manages a QWidgetStack of UMLView instances.
 * The visible diagram is at the top of stack.
 * The UMLView class inherits from QGraphicsView and it owns the
 * objects displayed on its related QGraphicsScene (see m_WidgetList.)
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit UMLView(UMLFolder *parentFolder);
    virtual ~UMLView();

    UMLScene* umlScene() const;

    int zoom() const ;
    void setZoom(int zoom);
    int currentZoom();

    bool showPropDialog();

public slots:
    void zoomIn();
    void zoomOut();
    void show();

protected:
    void dragEnterEvent(QDragEnterEvent* mouseEvent);
    void dropEvent(QDropEvent* mouseEvent);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void showEvent(QShowEvent *se);
    virtual void hideEvent(QHideEvent *he);
    virtual void closeEvent(QCloseEvent* ce);

    void setCenter(const QPointF& centerPoint);
    QPointF center();

    QPointF m_currentCenterPoint;  ///< holds the current centerpoint for the view, used for panning and zooming
    QPoint  m_lastPanPoint;        ///< from panning the view
    int m_nZoom;        ///< zoom level in percent, default 100
};

#endif // UMLVIEW_H
