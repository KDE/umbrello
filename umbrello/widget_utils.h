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
     * Extracts the QPen properties into pen, QBrush properties into
     * brush and QFont properties into font from the XMI xml element
     * qElement.
     *
     * @param qElement The dom element from which the xmi info should
     *                 be extracted.
     *
     * @param pen The QPen object into which pen details should be
     *            read into.
     *
     * @param brush The QBrush object into which brush details should
     *              be read into.
     *
     * @param font The QFont object into which font details should be
     *             read into.
     * @todo Implement
     */
    void loadPainterInfoFromXMI(const QDomElement &qElement, QPen &pen,
                                QBrush &brush, QFont &font);

    /**
     * Saves the pen, brush and font info as xmi into the dom element
     * \a qElement.
     *
     * @param qDoc The QDomDocument object pointing to the xmi document.
     *
     * @param qElement The element into which the pen, brush and font
     *                 info should be saved.
     *
     * @param pen The QPen whose details should be saved.
     * @param brush The QBrush whose details should be saved.
     * @param font The QFont whose details should be saved.
     * @todo Implement
     */
    void savePainterInfoToXMI(QDomDocument &qDoc, QDomElement &qElement,
                              const QPen &pen, const QBrush &brush,
                              const QFont &font);

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
