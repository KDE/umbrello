/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2009                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef NEWASSOCIATIONWIDGET_H
#define NEWASSOCIATIONWIDGET_H

#include "linkwidget.h"
#include "widgetbase.h"

class FloatingTextWidget;
class UMLAssociation;
class UMLAttribute;
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

    class AssociationWidget : public WidgetBase, public LinkWidget
    {
        Q_OBJECT
    public:
        AssociationWidget(UMLWidget *widgetA, Uml::Association_Type type,
                          UMLWidget *widgetB, UMLObject *obj = 0);
        virtual ~AssociationWidget();

        //---------- LinkWidget Interface methods implemementation from now on.

        virtual void lwSetFont (QFont font);
        virtual UMLClassifier *getOperationOwner();

        virtual UMLOperation *getOperation();
        virtual void setOperation(UMLOperation *op);

        virtual QString getCustomOpText();
        virtual void setCustomOpText(const QString &opText);

        virtual void resetTextPositions();

        virtual void setMessageText(FloatingTextWidget *ft);
        virtual void setText(FloatingTextWidget *ft, const QString &newText);

        virtual bool showDialog();

        virtual UMLClassifier* getSeqNumAndOp(QString& seqNum, QString& op);
        virtual void setSeqNumAndOp(const QString &seqNum, const QString &op);

        virtual void constrainTextPos(qreal &textX, qreal &textY, qreal textWidth, qreal textHeight,
                Uml::Text_Role tr);

        virtual void calculateNameTextSegment();

        //---------- End LinkWidget Interface methods implemementation.
        UMLAssociation* association() const;
        UMLAttribute* attribute() const;

        bool isEqual(New::AssociationWidget *other) const;

        QString multiplicity(Uml::Role_Type role) const;
        void setMultiplicity(const QString& text, Uml::Role_Type role);

        Uml::Visibility visibility(Uml::Role_Type role) const;
        void setVisibility(Uml::Visibility v, Uml::Role_Type role);

        Uml::Changeability_Type changeability(Uml::Role_Type role) const;
        void setChangeability(Uml::Changeability_Type c, Uml::Role_Type role);
        void setChangeWidget(const QString &strChangeWidget, Uml::Role_Type role);

        void setRoleName (const QString &strRole, Uml::Role_Type role);

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
        void setFloatingText(Uml::Text_Role tr, const QString& text,
                FloatingTextWidget* &ft);
        void setTextPosition(Uml::Text_Role tr);

        friend class New::AssociationLine;

        New::AssociationLine *m_associationLine;
        WidgetRole m_widgetRole[2];
        FloatingTextWidget *m_nameWidget;
    };
}

#endif
