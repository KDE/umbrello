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
#include "assocrules.h"
#include "associationspacemanager.h"
#include "attribute.h"
#include "classifier.h"
#include "floatingtextwidget.h"
#include "newlinepath.h"
#include "objectwidget.h"
#include "operation.h"
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
        // 1) FloatingTextWidgets are deleted by ~QGraphicsItem as
        //    AssociationWidget is their parent.
        // 2) umlWidget is not owned by either AssociationWidget nor
        //    WidgetRole and hence not deleted either.
    }


    AssociationWidget::AssociationWidget(UMLWidget *widgetA, Uml::Association_Type type,
                                         UMLWidget *widgetB, UMLObject *umlObj) :
        WidgetBase(umlObj)
    {
        m_associationLine = new New::AssociationLine(this);
        m_nameWidget = 0;

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

        m_associationLine->calculateInitialEndPoints(widgetA, widgetB);

        setWidgetForRole(widgetA, Uml::A);
        setWidgetForRole(widgetB, Uml::B);

        Q_ASSERT(widgetA->umlScene() == widgetB->umlScene());

        if (isCollaboration()) {
            UMLScene *scene = widgetA->umlScene();
            int collabID = scene->generateCollaborationId();
            setName('m' + QString::number(collabID));
        }
        setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
    }

    AssociationWidget::~AssociationWidget()
    {
        delete m_associationLine;
    }

    void AssociationWidget::lwSetFont(QFont font)
    {
        WidgetBase::setFont(font);
    }

    UMLClassifier *AssociationWidget::getOperationOwner()
    {
        Uml::Role_Type role = isCollaboration() ? Uml::B : Uml::A;
        UMLObject *o = widgetForRole(role)->umlObject();
        if (!o) return 0;
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
        if (!c) {
            uError() << "widgetForRole(" << role << ") is not a classifier";
        }
        return c;
    }


    UMLOperation *AssociationWidget::getOperation()
    {
        return dynamic_cast<UMLOperation*>(umlObject());
    }

    void AssociationWidget::setOperation(UMLOperation *op)
    {
        setUMLObject(op);
    }

    QString AssociationWidget::getCustomOpText()
    {
        return name();
    }

    void AssociationWidget::setCustomOpText(const QString &opText)
    {
        setName(opText);
    }


    void AssociationWidget::resetTextPositions()
    {
        if (m_widgetRole[Uml::A].multiplicityWidget) {
            setTextPosition( Uml::tr_MultiA );
        }
        if (m_widgetRole[Uml::B].multiplicityWidget) {
            setTextPosition( Uml::tr_MultiB );
        }
        if (m_widgetRole[Uml::A].changeabilityWidget) {
            setTextPosition( Uml::tr_ChangeA );
        }
        if (m_widgetRole[Uml::B].changeabilityWidget) {
            setTextPosition( Uml::tr_ChangeB );
        }
        if (m_nameWidget) {
            setTextPosition( Uml::tr_Name );
        }
        if (m_widgetRole[Uml::A].roleWidget) {
            setTextPosition( Uml::tr_RoleAName );
        }
        if (m_widgetRole[Uml::B].roleWidget) {
            setTextPosition( Uml::tr_RoleBName );
        }
    }

    void AssociationWidget::setMessageText(FloatingTextWidget *ft)
    {
        QString msg;
        if (isCollaboration()) {
            if (umlObject()) {
                msg = multiplicity(Uml::A) + QLatin1String(": ") +
                    LinkWidget::getOperationText(umlScene());
            } else {
                msg = multiplicity(Uml::A) + QLatin1String(": ") +
                    name();
            }
        } else {
            msg = name();
        }
        ft->setText(msg);
    }

    void AssociationWidget::setText(FloatingTextWidget *ft, const QString &text)
    {
        switch (ft->textRole()) {
            case Uml::tr_Name:
                setName(text);
                break;
            case Uml::tr_RoleAName:
                setRoleName(text, Uml::A);
                break;
            case Uml::tr_RoleBName:
                setRoleName(text, Uml::B);
                break;
            case Uml::tr_MultiA:
                setMultiplicity(text, Uml::A);
                break;
            case Uml::tr_MultiB:
                setMultiplicity(text, Uml::B);
                break;
            default:
                break;
        }
    }


    bool AssociationWidget::showDialog()
    {
        //TODO: Port the following after New::AssociationWidget replaces AssociationWidget.
        /*
        AssocPropDlg dlg(static_cast<QWidget*>(m_pView), this );
        if (! dlg.exec())
            return false;
        QString name = getName();
        QString doc = getDoc();
        QString roleADoc = getRoleDoc(A), roleBDoc = getRoleDoc(B);
        QString rnA = getRoleName(A), rnB = getRoleName(B);
        QString ma = getMulti(A), mb = getMulti(B);
        Uml::Visibility vA = getVisibility(A), vB = getVisibility(B);
        Uml::Changeability_Type cA = getChangeability(A), cB = getChangeability(B);
        //rules built into these functions to stop updating incorrect values
        setName(name);
        setRoleName(rnA, A);
        setRoleName(rnB, B);
        setDoc(doc);
        setRoleDoc(roleADoc, A);
        setRoleDoc(roleBDoc, B);
        setMulti(ma, A);
        setMulti(mb, B);
        setVisibility(vA, A);
        setVisibility(vB, B);
        setChangeability(cA, A);
        setChangeability(cB, B);
        m_pView -> showDocumentation( this, true );
        return true;
        */
        return false;
    }


    UMLClassifier* AssociationWidget::getSeqNumAndOp(QString& seqNum, QString& op)
    {
        seqNum = multiplicity(Uml::A);
        op = name();
        return dynamic_cast<UMLClassifier*>(widgetForRole(Uml::B)->umlObject());
    }

    void AssociationWidget::setSeqNumAndOp(const QString &seqNum, const QString &op)
    {
        if (!op.isEmpty()) {
            setName(op);
        }
        setMultiplicity(seqNum, Uml::A);
    }


    void AssociationWidget::constrainTextPos(qreal &textX, qreal &textY, qreal textWidth, qreal textHeight,
            Uml::Text_Role tr)
    {
        //TODO: Implement this
        Q_UNUSED(textX);
        Q_UNUSED(textY);
        Q_UNUSED(textWidth);
        Q_UNUSED(textHeight);
        Q_UNUSED(tr);
    }


    void AssociationWidget::calculateNameTextSegment()
    {
        //TODO: Implement this
    }


    UMLAssociation* AssociationWidget::association() const
    {
        if (!umlObject()) return 0;
        Q_ASSERT(umlObject()->getBaseType() == Uml::ot_Association);
        return static_cast<UMLAssociation*>(umlObject());
    }

    UMLAttribute* AssociationWidget::attribute() const
    {
        if (!umlObject()) return 0;
        Uml::Object_Type ot = umlObject()->getBaseType();
        if (ot != Uml::ot_Attribute && ot != Uml::ot_EntityAttribute) {
            return 0;
        }
        return static_cast<UMLAttribute*>(umlObject());
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

    QString AssociationWidget::multiplicity(Uml::Role_Type role) const
    {
        FloatingTextWidget *wid = m_widgetRole[role].multiplicityWidget;
        if (wid) {
            return wid->text();
        }
        return QString();
    }

    void AssociationWidget::setMultiplicity(const QString& text, Uml::Role_Type role)
    {
        // If uml association object exists, then set its multiplicity which
        // will eventually signal this particular widget of text change and
        // this widget will react to that change.
        if (association()) {
            association()->setMulti(text, role);
        } else {
            Uml::Text_Role tr = (role == Uml::A ? Uml::tr_MultiA : Uml::tr_MultiB);
            setFloatingText(tr, text, m_widgetRole[role].multiplicityWidget);
        }
    }

    Uml::Visibility AssociationWidget::visibility(Uml::Role_Type role) const
    {
        if (association()) {
            return association()->getVisibility(role);
        }
        if (attribute()) {
            return attribute()->getVisibility();
        }
        return m_widgetRole[role].visibility;
    }

    void AssociationWidget::setVisibility(Uml::Visibility value, Uml::Role_Type role)
    {
        if (value == visibility(role))
            return;
        if (umlObject()) {
            // update our model object
            const Uml::Object_Type ot = umlObject()->getBaseType();
            if (ot == Uml::ot_Association)
                association()->setVisibility(value, role);
            else if (ot == Uml::ot_Attribute)
                attribute()->setVisibility(value);
        }
        m_widgetRole[role].visibility = value;
        // update role pre-text attribute as appropriate
        if (m_widgetRole[role].roleWidget) {
            QString scopeString = value.toString(true);
            m_widgetRole[role].roleWidget->setPreText(scopeString);
        }
    }

    Uml::Changeability_Type AssociationWidget::changeability(Uml::Role_Type role) const
    {
        if (!association()) {
            return m_widgetRole[role].changeability;
        }
        return association()->getChangeability(role);
    }

    void AssociationWidget::setChangeability(Uml::Changeability_Type c, Uml::Role_Type role)
    {
        if (c == changeability(role)) {
            return;
        }
        QString changeString = UMLAssociation::toString(c);
        if (association()) {
            association()->setChangeability(c, role);
        }
        m_widgetRole[role].changeability = c;
        // update our string representation
        setChangeWidget(changeString, role);
    }

    void AssociationWidget::setChangeWidget(const QString &strChangeWidget, Uml::Role_Type role)
    {
        bool newLabel = false;
        Uml::Text_Role tr = (role == Uml::A ? Uml::tr_ChangeA : Uml::tr_ChangeB);

        FloatingTextWidget *&chgWidget = m_widgetRole[role].changeabilityWidget;
        if(!chgWidget) {
            // Don't construct the FloatingTextWidget if the string is empty.
            if (strChangeWidget.isEmpty())
                return;

            newLabel = true;
            chgWidget = new FloatingTextWidget(tr);
            chgWidget->setText(strChangeWidget);
            chgWidget->setLink(this);
            chgWidget->setPreText("{"); // all types have this
            chgWidget->setPostText("}"); // all types have this
        } else {
            if (chgWidget->text().isEmpty()) {
                newLabel = true;
            }
            chgWidget->setText(strChangeWidget);
        }

        if (newLabel) {
            setTextPosition( tr );
        }

        chgWidget->setVisible(FloatingTextWidget::isTextValid(chgWidget->text()));
    }

    void AssociationWidget::setRoleName (const QString &strRole, Uml::Role_Type role)
    {
        //if the association is not supposed to have a Role FloatingTextWidget
        if (!AssocRules::allowRole(associationType()))  {
            return;
        }

        Uml::Text_Role tr = (role == Uml::A ? Uml::tr_RoleAName : Uml::tr_RoleBName);
        setFloatingText(tr, strRole, m_widgetRole[role].roleWidget);
        if (m_widgetRole[role].roleWidget) {
            Uml::Visibility vis = visibility(role);
            if (FloatingTextWidget::isTextValid(m_widgetRole[role].roleWidget->text())) {
                m_widgetRole[role].roleWidget->setPreText(vis.toString(true));
                //m_role[role].m_pRole->show();
            } else {
                m_widgetRole[role].roleWidget->setPreText("");
                //m_role[role].m_pRole->hide();
            }
        }

        // set attribute of UMLAssociation associated with this associationwidget
        if (association()) {
            association()->setRoleName(strRole, role);
        }
    }

    Uml::Role_Type AssociationWidget::roleForWidget(UMLWidget *widget) const
    {
        if (m_widgetRole[Uml::A].umlWidget == widget) {
            return Uml::A;
        } else if (m_widgetRole[Uml::B].umlWidget == widget) {
            return Uml::B;
        } else {
            Q_ASSERT_X(false, "roleForWidget()", "widget passed isn't pariticpant of association");
        }
        return Uml::A; // unreachable.
    }

    UMLWidget* AssociationWidget::widgetForRole(Uml::Role_Type role) const
    {
        return m_widgetRole[role].umlWidget;
    }

    void AssociationWidget::setWidgetForRole(UMLWidget *widget, Uml::Role_Type role)
    {
        m_widgetRole[role].umlWidget = widget;
        widget->associationSpaceManager()->add(this);
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

    void AssociationWidget::setFloatingText(Uml::Text_Role tr, const QString& text, FloatingTextWidget* &ft)
    {
        if (! FloatingTextWidget::isTextValid(text)) {
            if (ft) {
                // Remove preexisting FloatingTextWidget
                delete ft;
                ft = 0;
            }
            return;
        }

        if (!ft) {
            ft = new FloatingTextWidget(tr);
            ft->setText(text);
            ft->setLink(this);
            setTextPosition(tr);
        } else {
            bool newLabel = ft->text().isEmpty();
            ft->setText(text);
            if (newLabel)
                setTextPosition(tr);
        }
    }

    void AssociationWidget::setTextPosition(Uml::Text_Role tr)
    {
        Q_UNUSED(tr);
        // TODO: Implement this stub
    }


#include "newassociationwidget.moc"
}
