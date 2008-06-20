/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef WIDGET_UTILS_H
#define WIDGET_UTILS_H

#include <QtCore/QPoint>
#include <QtXml/QDomDocument>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <QtGui/QFont>

#include "umlnamespace.h"
#include "umlwidgetlist.h"
#include "messagewidgetlist.h"

#define RESIZE_HANDLE_SIZE 6

// forward declarations
class QGraphicsRectItem;

/**
 * General purpose widget utilities.
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Widget_Utils
{

    /**
     * Find the widget identified by the given ID in the given widget
     * or message list.
     *
     * @param id            The unique ID to find.
     * @param widgets       The UMLWidgetList to search in.
     * @param pMessages     Optional pointer to a MessageWidgetList to
     *                      search in.
     */
    UMLWidget* findWidget(Uml::IDType id,
                          const UMLWidgetList& widgets,
                          const MessageWidgetList* pMessages = NULL);

    /**
     * Creates the decoration point.
     * @param p   the base point
     * @return    the decoration point
     */
    QGraphicsRectItem *decoratePoint(const QPointF& p);

    /**
     * Converts a point to a comma separated string i.e "x,y"
     */
    QString pointToString(const QPointF& point);

    /**
     * Converts a comma separated string to point.
     */
    QPointF stringToPoint(const QString& str);

    /**
     * Loads pixmap from xmi.
     *
     * @param qElement The dom element from which pixmap should be
     *                 loaded.
     *
     * @param pixmap The pixmap into which the image should be loaded.
     *
     * @return True or false based on success or failure of this method.
     */
    bool loadPixmapFromXMI(const QDomElement &qElement, QPixmap &pixmap);

    /**
     * Saves pixmap informatin into dom element \a qElement.
     *
     * @param qDoc The dom document object.
     *
     * @param qElement The dom element into which the pixmap should be
     *                 saved.
     *
     * @param pixmap The pixmap to be saved.
     */
    void savePixmapToXMI(QDomDocument &qDoc, QDomElement &qElement, const QPixmap& pixmap);

    /**
     * Loads gradient from xmi. The gradient pointer should be null
     * and the new gradient object will be created inside this method.
     * The gradient should later be deleted externally.
     *
     * @param qElement The dom element from which gradient should be
     *                 loaded.
     *
     * @param gradient The pointer to gradient into which the gradient
     *                 should be loaded. (Allocated inside this
     *                 method)
     *
     * @return True or false based on success or failure of this method.
     */
    bool loadGradientFromXMI(const QDomElement &qElement, QGradient *&gradient);

    /**
     * Saves gradient informatin into dom element \a qElement.
     *
     * @param qDoc The dom document object.
     *
     * @param qElement The dom element into which the gradient should be
     *                 saved.
     *
     * @param gradient The gradient to be saved.
     */
    void saveGradientToXMI(QDomDocument &qDoc, QDomElement &qElement, const QGradient *gradient);

    /**
     * Extracts the QBrush properties into brush from the XMI xml
     * element qElement.
     *
     * @param qElement The dom element from which the xmi info should
     *                 be extracted.
     *
     * @param brush The QBrush object into which brush details should
     *              be read into.
     */
    bool loadBrushFromXMI(const QDomElement &qElement, QBrush &brush);

    /**
     * Saves the brush info as xmi into the dom element \a qElement.
     *
     * @param qDoc The QDomDocument object pointing to the xmi document.
     *
     * @param qElement The element into which the pen, brush and font
     *                 info should be saved.
     *
     * @param brush The QBrush whose details should be saved.
     */
    void saveBrushToXMI(QDomDocument &qDoc, QDomElement &qElement,
                        const QBrush& brush);

    /**
     * Utility function to draw the eight resize handles surrounding the
     * rectangle \a rect.
     *
     * @param painter The QPainter initialized with device on which
     *                the resize handles should be painted.
     *
     * @param rect The rectangle surrounding which the resize handles
     *             should be drawn.
     *
     * @note The painter state is saved and restored.
     */
    void drawResizeHandles(QPainter *painter, const QRectF &rect);

    /**
     * Utility function which returns the appropriate resize handle
     * based on the whether \a point exists inside the resize handle
     * corresponding to \a rect.
     *
     * @return One of size resize handles if found or Uml::rh_None if
     *         not found.
     *
     * @param point The point to be tested for closure.
     *
     * @param rect The rectangle with respect to which \a point should
     *             be tested.
     */
    Uml::ResizeHandle resizeHandleForPoint(const QPointF &point, const QRectF& rect);

    /**
     * Utility method to return the appropriate mouse cursor based on
     * \a handle.
     *
     * @param handle The ResizeHandle for which the mouse cursor is required.
     *
     * @return The appropriate QCursor if handle != Uml::rh_None, else
     *         returns QCursor::ArrowCursor.
     */
    QCursor cursorForResizeHandle(const Uml::ResizeHandle handle);

    /**
     * This utility method adjust the \a rect to hold the resize
     * handles.
     *
     * @param rect The reference to a valid rect, which will be
     *             enlarged to hold the resize handles.
     */
    void adjustRectForResizeHandles(QRectF &rect);
}

#endif
