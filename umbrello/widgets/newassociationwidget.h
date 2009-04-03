/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2008                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef NEWASSOCIATIONWIDGET_H
#define NEWASSOCIATIONWIDGET_H

#include "widgetbase.h"

class FloatingTextWidget;
class UMLAssociation;
class UMLWidget;

namespace New
{
    class AssociationLine;

    enum Region {
        Left,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft
    };

    struct WidgetRole
    {
        FloatingTextWidget *multiplicityWidget;
        FloatingTextWidget *changeabilityWidget;
        FloatingTextWidget *roleWidget;

        UMLWidget *umlWidget;

        New::Region region;

        // The following are used only in case of absence of UMLObject
        Uml::Visibility visibility;
        Uml::Changeability_Type changeability;
        QString roleDocumentation;

        WidgetRole();
        ~WidgetRole();
    };

    class AssociationWidget : public WidgetBase
    {
        Q_OBJECT
    public:
        AssociationWidget(UMLWidget *widgetA, Uml::Association_Type type,
                          UMLWidget *widgetB, UMLObject *obj = 0);
        virtual ~AssociationWidget();

        UMLAssociation* association() const;
        bool isEqual(New::AssociationWidget *other) const;

        QString multiplicity(Uml::Role_Type role) const;
        void setMultiplicity(const QString& text, Uml::Role_Type role);

        Uml::Visibility visibility(Uml::Role_Type role) const;
        void setVisibility(Uml::Visibility v, Uml::Role_Type role);

        Uml::Changeability_Type changeability(Uml::Role_Type role) const;
        void setChangeability(Uml::Changeability_Type c, Uml::Role_Type role);

        UMLWidget* widgetForRole(Uml::Role_Type role) const;
        void setWidgetForRole(UMLWidget *widget, Uml::Role_Type role);

        Uml::Association_Type associationType() const;
        void setAssociationType(Uml::Association_Type type);

        bool isCollaboration() const;

        ////////////////////////////////////////////////
        New::AssociationLine* associationLine() const {
            return m_associationLine;
        }
        ///////////////////////////////////////////////

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem* opt, QWidget*);

    protected:
        virtual void updateGeometry();
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);
        virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    private:
        friend class New::AssociationLine;

        New::AssociationLine *m_associationLine;
        WidgetRole m_widgetRole[2];
    };
}

#endif
