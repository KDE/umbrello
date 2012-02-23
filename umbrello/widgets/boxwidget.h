/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef BOXWIDGET_H
#define BOXWIDGET_H

//app includes
#include "umlwidget.h"

/**
 * Displays a rectangular box.
 * These widgets are diagram specific.  They will still need a unique id
 * from the @ref UMLDoc class for deletion and other purposes.
 *
 * @short Displays a box.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class BoxWidget : public UMLWidget
{
public:
    explicit BoxWidget(UMLScene * scene, Uml::IDType id = Uml::id_None, WidgetType type = WidgetBase::wt_Box);
    virtual ~BoxWidget();

    virtual void paint(QPainter & p, int offsetX, int offsetY);
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

};

#endif
