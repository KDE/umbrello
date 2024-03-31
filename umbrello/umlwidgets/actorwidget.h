/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ACTORWIDGET_H
#define ACTORWIDGET_H

#include "umlwidget.h"

#define A_WIDTH 20
#define A_HEIGHT 40
#define A_MARGIN 5

class UMLActor;

/**
 * This class is the graphical version of a UML Actor.
 * An ActorWidget is created by a @ref UMLView.  An ActorWidget belongs to only
 * one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to is destroyed, the
 * ActorWidget will be automatically deleted.
 *
 * If the UMLActor class that this ActorWidget is displaying is deleted, the
 * @ref UMLView will make sure that this instance is also deleted.
 *
 * The ActorWidget class inherits from the @ref UMLWidget class
 * which adds most of the functionality to this class.
 *
 * @short A graphical version of a UML Actor.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLWidget
 * @see UMLView
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ActorWidget : public UMLWidget
{
public:
    ActorWidget(UMLScene * scene, UMLActor *o);
    virtual ~ActorWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    virtual void saveToXMI(QXmlStreamWriter& writer);

protected:
    QSizeF minimumSize() const;
};

#endif
