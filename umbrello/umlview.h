/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
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
 * UMLView instances represent views onto diagrams (scenes).
 * The UMLApp instance manages the visibility of UMLView instances.
 * The visible view (and therefore diagram) is at the top of stack.
 * The UMLView class inherits from QGraphicsView.
 * Construction of an UMLView implicitly constructs an UMLScene, i.e.
 * there is a one to one relation between diagram and view.
 * The UMLFolder instances own the UMLView instances.
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

    virtual bool showPropertiesDialog(QWidget *parent = 0);

public slots:
    void zoomIn();
    void zoomOut();
    void show();

protected:
    virtual void wheelEvent(QWheelEvent* event);
    virtual void showEvent(QShowEvent *se);
    virtual void hideEvent(QHideEvent *he);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void resizeEvent(QResizeEvent *event);
};

#endif // UMLVIEW_H
