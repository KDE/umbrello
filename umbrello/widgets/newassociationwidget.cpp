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

#include "newassociationwidget.h"

#include "association.h"
#include "floatingtextwidget.h"
#include "newlinepath.h"
#include "objectwidget.h"
#include "umlscene.h"
#include "umlwidget.h"

namespace New
{
    WidgetRole::WidgetRole()
    {
        multiplicityWidget = changeabilityWidget = roleWidget = 0;
        umlWidget = 0;
        region = New::Left;
        visibility = Uml::Visibility::Public;
        changeability = Uml::chg_Changeable;
    }

    WidgetRole::~WidgetRole()
    {
        // for now nothing, as ownership is with New::AssociationWidget
    }


    AssociationWidget::AssociationWidget(UMLWidget *widgetA, Uml::Association_Type type,
                                         UMLWidget *widgetB, UMLObject *umlObj) : WidgetBase(umlObj)
    {
        m_associationLine = new New::AssociationLine(this);

// Disabled for testing purpose
#if 0
        if (!umlObj && UMLAssociation::assocTypeHasUMLRepresentation(type)) {
            UMLObject *objectA = widgetA->umlObject();
            UMLObject *objectB = widgetB->umlObject();

            if (objectA && objectB) {
                // TODO: Check for already existing association of
                //       same type between same two widgets. (better
                //       to check before creation rather than here.)

                setUMLObject(new UMLAssociation(type, objectA, objectB));
            }
        }

        setWidgetForRole(widgetA, Uml::A);
        setWidgetForRole(widgetB, Uml::B);

        Q_ASSERT(widgetA->umlScene() == widgetB->umlScene());

        if (isCollaboration()) {
            UMLScene *scene = widgetA->umlScene();
            int collabID = scene->generateCollaborationId();
            setName('m' + QString::number(collabID));
        }
#endif
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
    }

    AssociationWidget::~AssociationWidget()
    {
        delete m_associationLine;
    }

    UMLAssociation* AssociationWidget::association() const
    {
        if (!umlObject()) return 0;
        Q_ASSERT(umlObject()->getBaseType() == Uml::ot_Association);
        return static_cast<UMLAssociation*>(umlObject());
    }

    bool AssociationWidget::isEqual(New::AssociationWidget *other) const
    {
        if( this == other )
            return true;

        // if no model representation exists, then the widgets are not equal
        if ( !association() || !other->association() )
            return false;
        else if (association() != other->association()) {
            return false;
        }

        if (associationType() != other->associationType())
            return false;

        UMLWidget *aWid = widgetForRole(Uml::A);
        UMLWidget *bWid = widgetForRole(Uml::B);

        UMLWidget *oaWid = other->widgetForRole(Uml::A);
        UMLWidget *obWid = other->widgetForRole(Uml::B);

        if (aWid->id() != oaWid->id() || bWid->id() != obWid->id())
            return false;

        if (aWid->baseType() == Uml::wt_Object && oaWid->baseType() == Uml::wt_Object) {
            ObjectWidget *a = static_cast<ObjectWidget*>(aWid);
            ObjectWidget *oa = static_cast<ObjectWidget*>(oaWid);
            if (a->localID() != oa->localID()) {
                return false;
            }
        }

        if (bWid->baseType() == Uml::wt_Object && obWid->baseType() == Uml::wt_Object) {
            ObjectWidget *b = static_cast<ObjectWidget*>(bWid);
            ObjectWidget *ob = static_cast<ObjectWidget*>(obWid);
            if (b->localID() != ob->localID()) {
                return false;
            }
        }

        // Two objects in a collaboration can have multiple messages between each other.
        // Here we depend on the messages having names, and the names must be different.
        // That is the reason why collaboration messages have strange initial names like
        // "m29997" or similar.
        return (name() == other->name());

    }

    UMLWidget* AssociationWidget::widgetForRole(Uml::Role_Type role) const
    {
        return m_widgetRole[role].umlWidget;
    }

    void AssociationWidget::setWidgetForRole(UMLWidget *widget, Uml::Role_Type role)
    {
        m_widgetRole[role].umlWidget = widget;

    }

    Uml::Association_Type AssociationWidget::associationType() const
    {
        return static_cast<UMLAssociation*>(umlObject())->getAssocType();
    }

    void AssociationWidget::setAssociationType(Uml::Association_Type type)
    {
        static_cast<UMLAssociation*>(umlObject())->setAssocType(type);
    }

    bool AssociationWidget::isCollaboration() const
    {
        Uml::Association_Type at = associationType();
        return (at == Uml::at_Coll_Message || at == Uml::at_Coll_Message_Self);
    }

    QRectF AssociationWidget::boundingRect() const
    {
        return m_associationLine->boundingRect();
    }

    QPainterPath AssociationWidget::shape() const
    {
        return m_associationLine->shape();
    }

    void AssociationWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem* opt, QWidget *)
    {
        m_associationLine->paint(painter, opt);
    }

    void AssociationWidget::updateGeometry()
    {
        prepareGeometryChange();
        m_associationLine->calculateBoundingRect();
    }

    QVariant AssociationWidget::itemChange(GraphicsItemChange change, const QVariant& value)
    {
        if (change == ItemSelectedHasChanged) {
            setAcceptHoverEvents(isSelected());
        }
        return WidgetBase::itemChange(change, value);
    }

    QVariant AssociationWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
    {
        if (change == LineWidthHasChanged || change == LineColorHasChanged) {
            m_associationLine->updatePenSettings();
            return QVariant();
        }
        return WidgetBase::attributeChange(change, oldValue);
    }

    void AssociationWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        setSelected(true);
        m_associationLine->mousePressEvent(event);
    }

    void AssociationWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        m_associationLine->mouseMoveEvent(event);
    }

    void AssociationWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        m_associationLine->mouseReleaseEvent(event);
    }

    void AssociationWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
        m_associationLine->mouseDoubleClickEvent(event);
    }

    void AssociationWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        m_associationLine->hoverEnterEvent(event);
    }

    void AssociationWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
    {
        m_associationLine->hoverMoveEvent(event);
    }

    void AssociationWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        m_associationLine->hoverLeaveEvent(event);
    }

#include "newassociationwidget.moc"
}
