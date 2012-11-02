/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PINWIDGET_H
#define PINWIDGET_H

#include "umlwidget.h"
#include "worktoolbar.h"
#include "activitywidget.h"
#include "floatingtextwidget.h"


/**
 * This class is the graphical version of a UML Pin which is used in
 * Activity diagrams.
 *
 * @author Hassan KOUCH <hkouch@hotmail.com>
 * @author Gopala Krishna
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PinWidget : public UMLWidget
{
    Q_OBJECT
public:
    PinWidget( UMLWidget* a, Uml::IDType id = Uml::id_None );
    virtual ~PinWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w);

    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );
    virtual bool loadFromXMI( QDomElement & qElement );

    void updatePosition(const QPointF& reference);

protected:
    virtual void updateGeometry();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

private Q_SLOTS:
    void setInitialPosition();

private:
    /// This is the fixed size for this widget.
    static const qreal Size;

    /**
     * The owner to which this pin is attached.
     *
     * @todo why not ActivityWidget directly ?
     */
    UMLWidget * m_ownerWidget;

    /// This is used to display text for the Pin
    FloatingTextWidget * m_nameFloatingTextWiget;
};

#endif
