/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef USECASEWIDGET_H
#define USECASEWIDGET_H

#include "umlwidget.h"

class UMLUseCase;


/**
 * This class is the graphical version of a UMLUseCase. The
 * UseCaseWidget class inherits from the @ref UMLWidget class
 * which adds most of the functionality to this class.
 *
 * @short  A graphical version of a UMLUseCase.
 * @author Paul Hensgen <phensgen@techie.com>
 * @author Gopala Krishna
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UseCaseWidget : public UMLWidget
{
public:
    UseCaseWidget(UMLUseCase *o);
    virtual ~UseCaseWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    // For loading we can use the loadFromXMI() inherited from
    // UMLWidget.
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

private:
    enum {
        GroupIndex
    };
    enum {
        NameItemIndex,
        TextItemCount
    };
};

#endif
