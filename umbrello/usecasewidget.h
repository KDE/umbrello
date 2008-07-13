/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef USECASEWIDGET_H
#define USECASEWIDGET_H

#include "umlwidget.h"

class UMLUseCase;
class TextItemGroup;

/**
 * This class is the graphical version of a UMLUseCase.  A UseCaseWidget is created
 * by a @ref UMLScene.  An UseCaseWidget belongs to only one @ref UMLScene instance.
 * When the @ref UMLScene instance that this class belongs to, it will be automatically deleted.
 *
 * If the @ref UseCase class that this UseCaseWidget is displaying is deleted, the @ref UMLScene will
 * make sure that this instance is also deleted.
 *
 * The UseCaseWidget class inherits from the @ref NewUMLRectWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UMLUseCase.
 * @author Paul Hensgen <phensgen@techie.com>
 * @author Gopala Krishna (port using TextItem)
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UseCaseWidget : public NewUMLRectWidget
{
public:

    UseCaseWidget(UMLUseCase *o);
    virtual ~UseCaseWidget();

    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    // For loading we can use the loadFromXMI() inherited from
    // NewUMLRectWidget.
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    QSizeF sizeHint(Qt::SizeHint which);

protected:
    void updateGeometry();
    void sizeHasChanged(const QSizeF& oldSize);

private:
    enum {
        NameItemIndex,
        TextItemCount
    };
    static const qreal Margin;

    TextItemGroup *m_textItemGroup;
    QSizeF m_minimumSize;
};

#endif
