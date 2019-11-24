/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2018                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ENTITYWIDGET_H
#define ENTITYWIDGET_H

#include "umlwidget.h"

class UMLScene;

#define ENTITY_MARGIN 5

/**
 * Defines a graphical version of the entity.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of an entity.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class EntityWidget : public UMLWidget
{
    Q_OBJECT
public:
    explicit EntityWidget(UMLScene *scene, UMLObject* o);
    virtual ~EntityWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    virtual bool loadFromXMI1(QDomElement &qElement);
    virtual void saveToXMI1(QDomDocument& qDoc, QDomElement& qElement);

    void setShowAttributeSignature(bool flag);
    bool showAttributeSignature() const;

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    bool m_showAttributeSignatures;

    QSizeF minimumSize() const;
    QSizeF calculateSize(bool withExtensions=true) const;
};

#endif
