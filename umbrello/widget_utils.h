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

}

#endif
