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

#include <Q3CanvasView>

class AssociationWidget;
class ToolBarState;
class ToolBarStateFactory;
class UMLFolder;
class UMLObject;
class UMLScene;
class UMLWidget;

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

    UMLScene* umlScene() const;

    int zoom() const ;
    void setZoom(int zoom);
    int currentZoom();

    void showDocumentation( UMLObject * object, bool overwrite );
    void showDocumentation( UMLWidget * widget, bool overwrite );
    void showDocumentation( AssociationWidget * widget, bool overwrite );
    void updateDocumentation( bool clear );

    /**
     * Reset the toolbar.
     */
    void resetToolbar() {
        emit sigResetToolBar();
    }

signals:
    void sigResetToolBar();

public slots:
    void zoomIn();
    void zoomOut();
    void slotToolBarChanged(int c);

protected:
    virtual void closeEvent(QCloseEvent* ce);

    void contentsMouseReleaseEvent(QMouseEvent* mouseEvent);
    void contentsMouseMoveEvent(QMouseEvent* mouseEvent);
    void contentsMouseDoubleClickEvent(QMouseEvent* mouseEvent);
    void contentsMousePressEvent(QMouseEvent* mouseEvent);
    void hideEvent(QHideEvent *he);
    void showEvent(QShowEvent *se);

    /**
     * Pointer to scene
     */
    UMLScene *m_scene;

    /**
     * The zoom level in percent, default 100
     */
    int m_nZoom;

    ToolBarStateFactory* m_pToolBarStateFactory;
    ToolBarState* m_pToolBarState;

    /**
     * set to true when a child has used the showDocumentation method,
     * thus when one clicks on a child widget.
     * Reset to false when clicking in an empty region of the view.
     */
    bool m_bChildDisplayedDoc;
};

#endif // UMLVIEW_H
