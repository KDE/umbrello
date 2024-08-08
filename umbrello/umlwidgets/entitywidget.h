/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    virtual bool loadFromXMI(QDomElement &qElement);
    virtual void saveToXMI(QXmlStreamWriter& writer);

    void setShowAttributeSignature(bool flag);
    bool showAttributeSignature() const;

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    bool m_showAttributeSignatures{false};

    QSizeF minimumSize() const;
    QSizeF calculateSize(bool withExtensions=true) const;
};

#endif
