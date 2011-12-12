/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef WIDGET_UTILS_H
#define WIDGET_UTILS_H

#include <QtCore/QPoint>

#include "basictypes.h"
#include "umlwidgetlist.h"
#include "messagewidgetlist.h"
#include "umlscene.h"

/**
 * General purpose widget utilities.
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Widget_Utils {

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
UMLSceneRectangle *decoratePoint(const QPoint& p);

}

#endif
