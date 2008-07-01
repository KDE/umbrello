/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef WIDGET_FACTORY_H
#define WIDGET_FACTORY_H

#include <qstring.h>

// forward declarations
class UMLView;
class UMLObject;
class NewUMLRectWidget;
class UMLScene;
/**
 * Widget factory methods.
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Widget_Factory {

    /**
     * Create a NewUMLRectWidget in the given view and representing the given document object.
     */
    NewUMLRectWidget *createWidget(UMLScene *scene, UMLObject *docObj);

    /**
     * Create a NewUMLRectWidget according to the given XMI tag.
     */
    NewUMLRectWidget* makeWidgetFromXMI(const QString& tag,
                                 const QString& idStr, UMLScene *scene);

}   // end namespace Widget_Factory

#endif
