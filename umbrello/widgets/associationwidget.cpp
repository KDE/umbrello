/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "associationwidget.h"

// app includes
#include "association.h"
#include "associationline.h"
#include "assocpropdlg.h"
#include "assocrules.h"
#include "associationspacemanager.h"
#include "attribute.h"
#include "classifier.h"
#include "classifierlistitem.h"
#include "classifierwidget.h"
#include "debug_utils.h"
#include "entity.h"
#include "floatingtextwidget.h"
#include "objectwidget.h"
#include "operation.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"
#include "widget_utils.h"

// kde includes
#include <kinputdialog.h>
#include <klocale.h>
#include <kcolordialog.h>

// qt includes
#include <QPointer>
#include <QRegExpValidator>

// system includes
#include <cmath>

/**
 * Default constructor for WidgetRole.
 */
WidgetRole::WidgetRole()
  : multiplicityWidget(0),
    changeabilityWidget(0),
    roleWidget(0),
    umlWidget(0),
    visibility(Uml::Visibility::Public),
    changeability(Uml::Changeability::Changeable),
    roleDocumentation(QString())
{
}

/**
 * Destructor for WidgetRole.
 */
WidgetRole::~WidgetRole()
{
    // 1) FloatingTextWidgets are deleted by ~QGraphicsItem as
    //    AssociationWidget is their parent.
    // 2) umlWidget is not owned by either AssociationWidget nor
    //    WidgetRole and hence not deleted either.
}

/**
 * Initialisation of floating widgets.
 * @param role     the role A or B
 * @param parent   the parent association widget
 */
void WidgetRole::initFloatingWidgets(Uml::Role_Type role, AssociationWidget *parent)
{
    Uml::TextRole textRole = (role == Uml::A ? Uml::TextRole::MultiA : Uml::TextRole::MultiB);
    multiplicityWidget = new FloatingTextWidget(textRole);
    multiplicityWidget->setLink(parent);

    textRole = (role == Uml::A ? Uml::TextRole::ChangeA : Uml::TextRole::ChangeB);
    changeabilityWidget = new FloatingTextWidget(textRole);
    changeabilityWidget->setPreText("{");
    changeabilityWidget->setPostText("}");
    changeabilityWidget->setText("");
    changeabilityWidget->setLink(parent);

    textRole = (role == Uml::A ? Uml::TextRole::RoleAName : Uml::TextRole::RoleBName);
    roleWidget = new FloatingTextWidget(textRole);
    roleWidget->setLink(parent);

    //TODO: Activation of floating text widgets
}

/**
 * Default constructor.
 */
AssociationWidget::AssociationWidget()
  : WidgetBase(WidgetBase::wt_Association)
{
    init();
}

/**
 * Constructor.
 */
AssociationWidget::AssociationWidget(UMLWidget *widgetA, Uml::AssociationType type,
                                     UMLWidget *widgetB, UMLObject *umlObj)
  : WidgetBase(WidgetBase::wt_Association)
{
    init();
    const bool doNotNotifyAsSlot = false;
    if (umlObj) {
        setUMLObject(umlObj, doNotNotifyAsSlot);
    }
    else if (Uml::AssociationType::hasUMLRepresentation(type)) {
        UMLObject *objectA = widgetA->umlObject();
        UMLObject *objectB = widgetB->umlObject();

        if (objectA && objectB) {
             bool swap = false;

             // This is not correct. We could very easily have more than one
             // of the same type of association between the same two objects.
             // Just create the association. This search should have been
             // done BEFORE creation of the widget, if it mattered to the code.
             // But lets leave check in here for the time being so that debugging
             // output is shown, in case there is a collision with code elsewhere.
             UMLAssociation * myAssoc = umlDoc()->findAssociation(type,
                    objectA, objectB, &swap);
             if (myAssoc) {
                 if (type == Uml::AssociationType::Generalization) {
                     DEBUG(DBG_SRC) << "Ignoring second construction of same generalization";
                 } else {
                     DEBUG(DBG_SRC) << "Constructing a similar or exact same assoc " <<
                         "as an already existing assoc (swap=" << swap << ")";
                     // now, just create a new association anyways
                     myAssoc = 0;
                 }
             }

             if (!myAssoc) {
                myAssoc = new UMLAssociation(type, objectA, objectB);
             }

             setUMLObject(myAssoc, doNotNotifyAsSlot);
        }
    }

    setWidgets(widgetA, widgetB);
    setAssociationType(type);

    //TODO: Probably move this calculation to slotInit.
    m_associationLine->calculateInitialEndPoints();
    m_associationLine->reconstructSymbols();

    Q_ASSERT(widgetA->umlScene() == widgetB->umlScene());

    updateNameWidgetRole();
}

/**
 * Destructor.
 */
AssociationWidget::~AssociationWidget()
{
    delete m_associationLine;
}

/**
 * Reimplemented to do more checks and changes while setting a new UMLObject.
 * The old UMLObject's connectivity is removed in @ref umlObjectChanged method, which is
 * invoked by WidgetBase::setUMLObject.
 */
void AssociationWidget::setUMLObject(UMLObject *obj, bool notifyAsSlot)
{
    if (obj == umlObject()) {
        return;
    }
    if (!obj) {
        WidgetBase::setUMLObject(0, notifyAsSlot);
        return;
    }

    const UMLObject::ObjectType ot = obj->baseType();
    switch (ot) {
        case UMLObject::ot_Association:
            {
                UMLAssociation *assoc = static_cast<UMLAssociation*>(obj);
                if (assoc->nrof_parent_widgets < 0) {
                    assoc->nrof_parent_widgets = 0;
                }
                assoc->nrof_parent_widgets++;
            }
            break;
        case UMLObject::ot_Operation:
            // Nothing special to do.
            break;
        case UMLObject::ot_Attribute:
            {
                UMLClassifier *klass = static_cast<UMLClassifier*>(obj->parent());
                connect(klass, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                        this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
                // attributeChanged is emitted along with modified signal. So its not
                // necessary to handle attributeChanged signal.
            }
            break;
        case UMLObject::ot_EntityAttribute:
            {
                UMLEntity *ent = static_cast<UMLEntity*>(obj->parent());
                connect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                        this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
            }
            break;
        case UMLObject::ot_ForeignKeyConstraint:
            {
                UMLEntity *ent = static_cast<UMLEntity*>(obj->parent());
                connect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                        this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
            }
            break;
        default:
            uError() << "cannot associate UMLObject of type " << UMLObject::toString(ot);
            break;
    }

    WidgetBase::setUMLObject(obj, notifyAsSlot);
}

void AssociationWidget::slotClassifierListItemRemoved(UMLClassifierListItem* classifierItem)
{
    DEBUG(DBG_SRC) << "TODO - " << classifierItem->name();
}

void AssociationWidget::lwSetFont(QFont font)
{
    WidgetBase::setFont(font);
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 * @todo Move to LinkWidget.
 */
UMLClassifier *AssociationWidget::operationOwner()
{
    Uml::Role_Type role = (isCollaboration() ? Uml::B : Uml::A);
    UMLObject *o = widgetForRole(role)->umlObject();
    if (!o) {
        return 0;
    }
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
    if (!c) {
        uError() << "widgetForRole(" << role << ") is not a classifier";
    }
    return c;
}

/**
 * Implements operation from LinkWidget.
 * Motivated by FloatingTextWidget.
 */
UMLOperation *AssociationWidget::operation()
{
    return dynamic_cast<UMLOperation*>(umlObject());
}

/**
 * Implements operation from LinkWidget.
 * Motivated by FloatingTextWidget.
 */
void AssociationWidget::setOperation(UMLOperation *op)
{
    setUMLObject(op);
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
QString AssociationWidget::customOpText()
{
    return name();
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
void AssociationWidget::setCustomOpText(const QString &opText)
{
    setName(opText);
}

/**
 * Calls setTextPosition on all the labels.
 * Overrides operation from LinkWidget.
 */
void AssociationWidget::resetTextPositions()
{
    DEBUG(DBG_SRC) << "called";
    if (m_role[Uml::A].multiplicityWidget) {
        setTextPosition( Uml::TextRole::MultiA );
    }
    if (m_role[Uml::B].multiplicityWidget) {
        setTextPosition( Uml::TextRole::MultiB );
    }
    if (m_role[Uml::A].changeabilityWidget) {
        setTextPosition( Uml::TextRole::ChangeA );
    }
    if (m_role[Uml::B].changeabilityWidget) {
        setTextPosition( Uml::TextRole::ChangeB );
    }
    if (m_nameWidget) {
        setTextPosition( Uml::TextRole::Name );
    }
    if (m_role[Uml::A].roleWidget) {
        setTextPosition( Uml::TextRole::RoleAName );
    }
    if (m_role[Uml::B].roleWidget) {
        setTextPosition( Uml::TextRole::RoleBName );
    }
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param ft        The text widget which to update.
 */
void AssociationWidget::setMessageText(FloatingTextWidget *ft)
{
    QString message;
    if (isCollaboration()) {
        if (umlObject()) {
            message = multiplicity(Uml::A) + QLatin1String(": ") +
                          LinkWidget::operationText(umlScene());
        } else {
            message = multiplicity(Uml::A) + QLatin1String(": ") +
                          name();
        }
    } else {
        message = name();
    }
    ft->setText(message);
}

/**
 * Sets the text of the given FloatingTextWidget.
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
void AssociationWidget::setText(FloatingTextWidget *ft, const QString &text)
{
    Uml::TextRole role = ft->textRole();
    switch (role) {
        case Uml::TextRole::Name:
            setName(text);
            break;
        case Uml::TextRole::RoleAName:
            setRoleName(text, Uml::A);
            break;
        case Uml::TextRole::RoleBName:
            setRoleName(text, Uml::B);
            break;
        case Uml::TextRole::MultiA:
            setMultiplicity(text, Uml::A);
            break;
        case Uml::TextRole::MultiB:
            setMultiplicity(text, Uml::B);
            break;
        default:
            uWarning() << "Unhandled TextRole: " << role.toString();
            break;
    }
}

/**
 * Shows the association properties dialog and updates the
 * corresponding texts if its execution is successful.
 */
void AssociationWidget::showPropertiesDialog()
{
    UMLView *view = umlScene() ? umlScene()->activeView() : 0;

    QPointer<AssocPropDlg> dlg = new AssocPropDlg(view, this);
    if (dlg->exec()) {
        //rules built into these functions to stop updating incorrect values
        setName(name());

        setRoleName(roleName(Uml::A), Uml::A);
        setRoleName(roleName(Uml::B), Uml::B);

        setDocumentation(documentation());

        setRoleDocumentation(roleDocumentation(Uml::A), Uml::A);
        setRoleDocumentation(roleDocumentation(Uml::B), Uml::B);

        setMultiplicity(multiplicity(Uml::A), Uml::A);
        setMultiplicity(multiplicity(Uml::B), Uml::B);

        setVisibility(visibility(Uml::A), Uml::A);
        setVisibility(visibility(Uml::B), Uml::B);

        setChangeability(changeability(Uml::A), Uml::A);
        setChangeability(changeability(Uml::B), Uml::B);

        umlScene()->showDocumentation(this, true);
    }
    delete dlg;
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param seqNum    Return this AssociationWidget's sequence number string.
 * @param op        Return this AssociationWidget's operation string.
 */
UMLClassifier* AssociationWidget::seqNumAndOp(QString& seqNum, QString& op)
{
    seqNum = multiplicity(Uml::A);
    op = name();
    return dynamic_cast<UMLClassifier*>(widgetForRole(Uml::B)->umlObject());
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param seqNum    The new sequence number string to set.
 * @param op                The new operation string to set.
 */
void AssociationWidget::setSeqNumAndOp(const QString &seqNum, const QString &op)
{
    if (!op.isEmpty()) {
        setName(op);
    }
    setMultiplicity(seqNum, Uml::A);
}

void AssociationWidget::constrainTextPos(qreal &textX, qreal &textY, qreal textWidth,
        qreal textHeight, Uml::TextRole tr)
{
    const QPointF textCenter = QRectF(textX, textY, textWidth, textHeight).center();
    const uint numSegments = m_associationLine->count()-1;
    const uint lastSegmentIndex = numSegments-1;

    if (numSegments <= 0) {
        uError() << "Called when it had zero segments";
        return;
    }
    QPointF p0, p1;
    switch (tr) {
        case Uml::TextRole::RoleAName:
        case Uml::TextRole::MultiA:
        case Uml::TextRole::ChangeA:
            p0 = m_associationLine->point(0);
            p1 = m_associationLine->point(1);
            // If we are dealing with a single line then tie the
            // role label to the proper half of the line, i.e.
            // the role label must be closer to the "other"
            // role object.
            if (numSegments == 1) {
                p1 = (p0 + p1)/2.0;
            }
            break;

        case Uml::TextRole::RoleBName:
        case Uml::TextRole::MultiB:
        case Uml::TextRole::ChangeB:
            p0 = m_associationLine->point(lastSegmentIndex);
            p1 = m_associationLine->point(lastSegmentIndex+1);
            if (numSegments == 1) {
                p0 = (p0 + p1)/2.0;
            }
            break;

        case Uml::TextRole::Name:
        case Uml::TextRole::Coll_Message:  // CHECK: collab.msg texts seem to be TextRole::Name
        case Uml::TextRole::State:         // CHECK: is this used?
            // Find the linepath segment to which the (textX,textY) is closest
            // and constrain to the corridor of that segment (see farther below)
            {
                qreal minDistSquare = 100000.0;  // utopian initial value
                int lpIndex = 0;
                for (uint i = 0; i < numSegments; ++i) {
                    QPointF mid = (m_associationLine->point(i) +
                            m_associationLine->point(i+1))/2.0;
                    QPointF delta = textCenter - mid;
                    qreal distanceSquare = delta.x() * delta.x() + delta.y() * delta.y();
                    if (distanceSquare < minDistSquare) {
                        minDistSquare = distanceSquare;
                        lpIndex = i;
                    }
                }
                p0 = m_associationLine->point(lpIndex);
                p1 = m_associationLine->point(lpIndex + 1);
            }
            break;

        default:
            uError() << "unexpected TextRole " << tr.toString();
            return;
    }
    // Constraint:
    // The midpoint between p0 and p1 is taken to be the center of a circle
    // with radius D/2 where D is the distance between p0 and p1.
    // The text center needs to be within this circle else it is constrained
    // to the nearest point on the circle.
    const QPointF mid = (p0 + p1) / 2.0;
    const qreal radius = QLineF(p0, mid).length();

    QPointF projected = textCenter;
    QLineF line(mid, projected);
    if (line.length() > radius) {
        line.setLength(radius);
        projected = line.p2();
    }

    QGraphicsEllipseItem* eellipse = new QGraphicsEllipseItem(this);
    QGraphicsLineItem* lline = new QGraphicsLineItem(this);
    QGraphicsScene *s = umlScene();
    if (eellipse->scene() != s && s) {
        s->addItem(eellipse);
        s->addItem(lline);
    }
    eellipse->hide();
    lline->hide();

    QRectF r(0, 0, 2 * radius, 2 * radius);
    r.moveCenter(mid);
    eellipse->setRect(r);
    lline->setLine(QLineF(mid, projected));

    textX = projected.x() - .5 * textWidth;
    textY = projected.y() - .5 * textHeight;
}

/**
 * Calculates the m_nameSegmentIndex value according to the new
 * NameText topleft corner PT.
 * It iterates through all AssociationLine's segments and for each one
 * calculates the sum of PT's distance to the start point + PT's
 * distance to the end point. The segment with the smallest sum will
 * be the RoleTextSegment (if this segment moves then the RoleText
 * will move with it). It sets m_nameSegmentIndex to the start point
 * of the chosen segment.
 *
 * Overrides operation from LinkWidget (i.e. this method is also
 * required by FloatingTextWidget.)
 */
void AssociationWidget::calculateNameTextSegment()
{
    if (!m_nameWidget) return;
    //changed to use the middle of the text
    //i think this will give a better result.
    //never know what sort of lines people come up with
    //and text could be long to give a false reading
    qreal xt = m_nameWidget->x();
    qreal yt = m_nameWidget->y();
    xt += m_nameWidget->width() / 2;
    yt += m_nameWidget->height() / 2;
    uint size = m_associationLine->count();
    //sum of length(PTP1) and length(PTP2)
    qreal total_length = 0;
    qreal smallest_length = 0;
    for(uint i = 0; i < size - 1; ++i) {
        QPointF pi = m_associationLine->point(i);
        QPointF pj = m_associationLine->point(i+1);
        qreal xtiDiff = xt - pi.x();
        qreal xtjDiff = xt - pj.x();
        qreal ytiDiff = yt - pi.y();
        qreal ytjDiff = yt - pj.y();
        total_length =  std::sqrt( double(xtiDiff * xtiDiff + ytiDiff * ytiDiff) )
                        + std::sqrt( double(xtjDiff * xtjDiff + ytjDiff * ytjDiff) );
        //this gives the closest point
        if( total_length < smallest_length || i == 0) {
            smallest_length = total_length;
            m_nameSegmentIndex = i;
        }
    }
}

/**
 * Returns the UMLAssociation representation of this object.
 *
 * @return  Pointer to the UMLAssociation that is represented by
 *          this AsociationWidget.
 */
UMLAssociation* AssociationWidget::association() const
{
    if (!umlObject() || umlObject()->baseType() != UMLObject::ot_Association) {
        return 0;
    }
    return static_cast<UMLAssociation*>(umlObject());
}

/**
 * Returns the UMLAttribute representation of this object.
 *
 * @return  Pointer to the UMLAttribute that is represented by
 *          this AsociationWidget.
 */
UMLAttribute* AssociationWidget::attribute() const
{
    if (!umlObject()) {
        return 0;
    }
    UMLObject::ObjectType ot = umlObject()->baseType();
    if (ot != UMLObject::ot_Attribute && ot != UMLObject::ot_EntityAttribute) {
        return 0;
    }
    return static_cast<UMLAttribute*>(umlObject());
}

bool AssociationWidget::isEqual(AssociationWidget *other) const
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

    if (aWid->baseType() == WidgetBase::wt_Object &&
        oaWid->baseType() == WidgetBase::wt_Object) {
        ObjectWidget *a = static_cast<ObjectWidget*>(aWid);
        ObjectWidget *oa = static_cast<ObjectWidget*>(oaWid);
        if (a->localID() != oa->localID()) {
            return false;
        }
    }

    if (bWid->baseType() == WidgetBase::wt_Object &&
        obWid->baseType() == WidgetBase::wt_Object) {
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

FloatingTextWidget* AssociationWidget::multiplicityWidget(Uml::Role_Type role) const
{
    return m_role[role].multiplicityWidget;
}

QString AssociationWidget::multiplicity(Uml::Role_Type role) const
{
    if (association()) {
        return association()->getMultiplicity(role);
    }
    return m_role[role].multiplicityWidget->text();
}

void AssociationWidget::setMultiplicity(const QString& text, Uml::Role_Type role)
{
    DEBUG(DBG_SRC) << "Called with " << text << "AB"[role] << " role";
    // If uml association object exists, then set its multiplicity which
    // will eventually signal this particular widget of text change and
    // this widget will react to that change.
    if (association()) {
        association()->setMultiplicity(text, role);
    } else {
        Uml::TextRole tr = (role == Uml::A ? Uml::TextRole::MultiA : Uml::TextRole::MultiB);
        setFloatingText(tr, text, m_role[role].multiplicityWidget);
    }
}

Uml::Visibility AssociationWidget::visibility(Uml::Role_Type role) const
{
    if (association()) {
        return association()->getVisibility(role);
    }
    if (attribute()) {
        return attribute()->visibility();
    }
    return m_role[role].visibility;
}

void AssociationWidget::setVisibility(Uml::Visibility value, Uml::Role_Type role)
{
    if (value == visibility(role)) {
        return;
    }

    UMLAttribute *attrib = attribute();
    UMLAssociation *assoc = association();

    m_role[role].visibility = value;
    if (attrib) {
        attrib->setVisibility(value);
    } else if (assoc) {
        assoc->setVisibility(value, role);
    } else {
        QString scopeString = value.toString(true);
        m_role[role].roleWidget->setPreText(scopeString);
    }
}

/**
 * Return the given role's changeability FloatingTextWidget widget.
 */
FloatingTextWidget* AssociationWidget::changeabilityWidget(Uml::Role_Type role) const
{
    return m_role[role].changeabilityWidget;
}

Uml::Changeability AssociationWidget::changeability(Uml::Role_Type role) const
{
    if (!association()) {
        return m_role[role].changeability;
    }
    return association()->changeability(role);
}

void AssociationWidget::setChangeability(Uml::Changeability value, Uml::Role_Type role)
{
    m_role[role].changeability = value;
    if (association()) {
        association()->setChangeability(value, role);
    } else {
        m_role[role].changeabilityWidget->setText(value.toString());
    }
}

FloatingTextWidget* AssociationWidget::nameWidget() const
{
    return m_nameWidget;
}

FloatingTextWidget* AssociationWidget::roleWidget(Uml::Role_Type role) const
{
    return m_role[role].roleWidget;
}

/**
 * Return the given role's FloatingTextWidget widget text.
 *
 * @return  The name set at the FloatingTextWidget.
 */
QString AssociationWidget::roleName(Uml::Role_Type role) const
{
    if (association()) {
        return association()->getRoleName(role);
    }
    return m_role[role].roleWidget->text();
}

/**
 * Sets the text to the FloatingTextWidget that display the Role text of this
 * association.
 * For this function to work properly, the associated widget
 *  should already be set.
 */
void AssociationWidget::setRoleName(const QString &strRole, Uml::Role_Type role)
{
    //if the association is not supposed to have a Role FloatingTextWidget
    if (!AssocRules::allowRole(associationType()))  {
        m_role[role].roleWidget->setText("");
        return;
    }

    if (association()) {
        association()->setRoleName(strRole, role);
        return;
    }

    m_role[role].roleWidget->setText(strRole);
    m_role[role].roleWidget->setPreText(visibility(role).toString(true));
}

/**
 * Set the documentation on the given role.
 */
void AssociationWidget::setRoleDocumentation(const QString& doc, Uml::Role_Type role)
{
    UMLAssociation *umlAssoc = association();
    if (!umlAssoc) {
        m_role[role].roleDocumentation = doc;
    } else {
        umlAssoc->setRoleDoc(doc, role);
    }
}

/**
 * Returns the given role's documentation.
 */
QString AssociationWidget::roleDocumentation(Uml::Role_Type role) const
{
    UMLAssociation *umlAssoc = association();
    if (!umlAssoc) {
        return QString();
    }
    return umlAssoc->getRoleDoc(role);
}

bool AssociationWidget::associates(UMLWidget *a, UMLWidget *b) const
{
    Q_ASSERT(a != 0);
    Q_ASSERT(b != 0);
    return widgetForRole(Uml::A) == a && widgetForRole(Uml::B) == b;
}

bool AssociationWidget::containsWidget(UMLWidget *widget) const
{
    Q_ASSERT(widget != 0);
    return widgetForRole(Uml::A) == widget || widgetForRole(Uml::B) == widget;
}

FloatingTextWidget* AssociationWidget::textWidgetByRole(Uml::TextRole tr) const
{
    switch (tr) {
        case Uml::TextRole::MultiA:
            return m_role[Uml::A].multiplicityWidget;

        case Uml::TextRole::MultiB:
            return m_role[Uml::B].multiplicityWidget;

        case Uml::TextRole::Name:
        case Uml::TextRole::Coll_Message:
            return m_nameWidget;

        case Uml::TextRole::RoleAName:
            return m_role[Uml::A].roleWidget;

        case Uml::TextRole::RoleBName:
            return m_role[Uml::B].roleWidget;

        case Uml::TextRole::ChangeA:
            return m_role[Uml::A].changeabilityWidget;

        case Uml::TextRole::ChangeB:
            return m_role[Uml::B].changeabilityWidget;

        default:
            break;
    }

    return 0;
}

Uml::Role_Type AssociationWidget::roleForWidget(UMLWidget *widget) const
{
    if (m_role[Uml::A].umlWidget == widget) {
        return Uml::A;
    } else if (m_role[Uml::B].umlWidget == widget) {
        return Uml::B;
    } else {
        Q_ASSERT_X(false, "roleForWidget()", "widget passed isn't pariticpant of association");
    }
    return Uml::A; // unreachable.
}

bool AssociationWidget::isSelf() const
{
    return widgetForRole(Uml::A) == widgetForRole(Uml::B);
}

ClassifierWidget* AssociationWidget::associationClass() const
{
    return m_associationClass;
}

void AssociationWidget::setAssociationClass(ClassifierWidget *classifier)
{
    m_associationClass = classifier;
    if (classifier) {
        classifier->setClassAssociationWidget(this);
    }
    //TODO: Add this to the AssociationWidgetList of ClassifierWidget
    m_associationLine->calculateAssociationClassLine();
}

/**
 * Gets the given role widget.
 *
 * @return  Pointer to the role's UMLWidget.
 */
UMLWidget* AssociationWidget::widgetForRole(Uml::Role_Type role) const
{
    return m_role[role].umlWidget;
}

void AssociationWidget::setWidgetForRole(UMLWidget *widget, Uml::Role_Type role)
{
    if (m_role[role].umlWidget && widget != m_role[role].umlWidget) {
        DEBUG(DBG_SRC) << "Was holding" << (widget ? ID2STR(widget->id()) : "null")
            << "in its " << (role == Uml::A ? "A" : "B")  << " role end";
    }
    m_role[role].umlWidget = widget;
}

void AssociationWidget::setWidgets(UMLWidget *widgetA, UMLWidget *widgetB)
{
    setWidgetForRole(widgetA, Uml::A);
    setWidgetForRole(widgetB, Uml::B);
    if (widgetA || widgetB) {
        if (widgetA == widgetB) {
            widgetA->associationSpaceManager()->add(this,
                    RegionPair(Uml::reg_North, Uml::reg_North));
        } else {
            widgetA->associationSpaceManager()->add(this,
                    RegionPair(Uml::reg_North));
            widgetB->associationSpaceManager()->add(this,
                    RegionPair(Uml::reg_Error, Uml::reg_North));
        }
    }
}

Uml::IDType AssociationWidget::widgetIDForRole(Uml::Role_Type role) const
{
    UMLWidget *widget = widgetForRole(role);
    if (widget) {
        if (widget->baseType() == WidgetBase::wt_Object) {
            return static_cast<ObjectWidget*>(widget)->localID();
        }
        return widget->id();
    }

    UMLAssociation *assoc = association();
    if (assoc) {
        return assoc->getObjectId(role);
    }

    uError() << "m_role[role].umlWidget is NULL";
    return Uml::id_None;
}

/**
 * Gets the association's type.
 *
 * @return  This AssociationsWidget's AssociationType.
 */
Uml::AssociationType AssociationWidget::associationType() const
{
    if (!umlObject() || umlObject()->baseType() != UMLObject::ot_Association) {
        return m_associationType;
    }

    return static_cast<UMLAssociation*>(umlObject())->getAssocType();
}

/**
 * Sets the association's type.
 *
 * @param type   The AssociationType to set.
 */
void AssociationWidget::setAssociationType(Uml::AssociationType type)
{
    m_associationType = type;
    if (umlObject() && umlObject()->baseType() == UMLObject::ot_Association) {
        static_cast<UMLAssociation*>(umlObject())->setAssociationType(type);
    }

    WidgetRole &a = m_role[Uml::A];
    WidgetRole &b = m_role[Uml::B];

    if( a.umlWidget && !AssocRules::allowMultiplicity(type, a.umlWidget->baseType()) ) {
        if (a.multiplicityWidget) {
            a.multiplicityWidget->setName("");
        }
        if (b.multiplicityWidget) {
            b.multiplicityWidget->setName("");
        }
    }

    if (!AssocRules::allowRole(type)) {
        if (a.roleWidget) {
            a.roleWidget->setName("");
        }
        if (b.roleWidget) {
            b.roleWidget->setName("");
        }
        setRoleDocumentation("", Uml::A);
        setRoleDocumentation("", Uml::B);
    }
    m_associationLine->reconstructSymbols();
}

bool AssociationWidget::isCollaboration() const
{
    Uml::AssociationType at = associationType();
    return (at == Uml::AssociationType::Coll_Message || at == Uml::AssociationType::Coll_Message_Self);
}

/**
 * Returns a QString Object representing this AssociationWidget.
 */
QString AssociationWidget::toString() const
{
    QString string;
    static const QChar colon(':');

    if (widgetForRole(Uml::A)) {
        string = widgetForRole(Uml::A)->name();
    }
    string.append(colon);

    if (m_role[Uml::A].roleWidget) {
        string += m_role[Uml::A].roleWidget->text();
    }
    string.append(colon);
    string.append(associationType().toStringI18n());
    string.append(colon);

    if (widgetForRole(Uml::B)) {
        string += widgetForRole(Uml::B)->name();
    }

    string.append(colon);
    if (m_role[Uml::B].roleWidget) {
        string += m_role[Uml::B].roleWidget->text();
    }

    return string;
}

AssociationLine* AssociationWidget::associationLine() const
{
    return m_associationLine;
}

bool AssociationWidget::activate()
{
    Q_ASSERT(umlScene());
    setActivatedFlag(false);

    if (isCollaboration() && m_loadData.isEmpty() && !name().startsWith("m")) {

        int collabID = umlScene()->generateCollaborationId();

        setName('m' + QString::number(collabID));
    }

    bool hasUMLRepresentation =
        Uml::AssociationType::hasUMLRepresentation(associationType());
    if (!umlObject() && hasUMLRepresentation) {
        UMLObject *myObj = umlDoc()->findObjectById(id());
        if (!myObj) {
            uError() << "cannot find UMLObject " << ID2STR(id());
            return false;
        } else {
            setUMLObject(myObj);
            setAssociationType(associationType());
        }
    }

    Uml::AssociationType type = associationType();
    if (!widgetForRole(Uml::A)) {
        setWidgetForRole(umlScene()->findWidget(widgetIDForRole(Uml::A)), Uml::A);
    }
    if (!widgetForRole(Uml::B)) {
        setWidgetForRole(umlScene()->findWidget(widgetIDForRole(Uml::B)), Uml::B);
    }

    if (!widgetForRole(Uml::A) || !widgetForRole(Uml::B)) {
        return false;
    }

    // Call this method to ensure addition of this widget in the associationSpaceManager()
    // of both widgetForRole(Uml::A) and widgetForRole(Uml::B).
    setWidgets(widgetForRole(Uml::A), widgetForRole(Uml::B));

    //TODO: Check whether this comment should be removed.
    m_associationLine->calculateEndPoints();

    if (AssocRules::allowRole(type)) {
        for (unsigned r = Uml::A; r <= Uml::B; ++r) {
            WidgetRole& robj = m_role[r];
            if (!robj.roleWidget) {
                continue;
            }
            Uml::Visibility vis = visibility((Uml::Role_Type)r);
            robj.roleWidget->setPreText(vis.toString(true));

            if (umlScene()->type() == Uml::DiagramType::Collaboration) {
                robj.roleWidget->setUMLObject(robj.umlWidget->umlObject());
            }
            robj.roleWidget->activate();

            FloatingTextWidget* multi = robj.multiplicityWidget;
            if (multi &&
                    AssocRules::allowMultiplicity(type, robj.umlWidget->baseType())) {
                multi->activate();
            }

            FloatingTextWidget* change = robj.changeabilityWidget;
            if (change) {
                change->activate();
            }
        }
    }

    if (m_nameWidget) {
        updateNameWidgetRole();
        m_nameWidget->activate();
        calculateNameTextSegment();
    }

    // Prepare the association class line if needed.
    m_associationLine->calculateAssociationClassLine();

    // Set the text positions only if one label has zero position
    //:TODO: Remove this, when improved initialisation is done.
    const QPointF zeroPos = QPointF(0.0, 0.0);
    if ((m_role[Uml::A].multiplicityWidget->pos()  == zeroPos) ||
        (m_role[Uml::B].multiplicityWidget->pos()  == zeroPos) ||
        (m_role[Uml::A].changeabilityWidget->pos() == zeroPos) ||
        (m_role[Uml::B].changeabilityWidget->pos() == zeroPos) ||
        (m_nameWidget->pos()                             == zeroPos) ||
        (m_role[Uml::A].roleWidget->pos()          == zeroPos) ||
        (m_role[Uml::B].roleWidget->pos()          == zeroPos)) {
        resetTextPositions();
    }

    return WidgetBase::activate();
}

/**
 * Returns the bounding rectangle of all segments of the association.
 */
QRectF AssociationWidget::boundingRect() const
{
    return m_associationLine->boundingRect();
}

QPainterPath AssociationWidget::shape() const
{
    return m_associationLine->shape();
}

/**
 *
 */
void AssociationWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    m_associationLine->paint(painter, option, widget);
}

/**
 * Handles the selection from the popup menu.
 */
void AssociationWidget::slotMenuSelection(QAction* action)
{
    QString oldText, newText;
    QFont font;
    QRegExpValidator v(QRegExp(".*"), 0);
    Uml::AssociationType atype = associationType();
    Uml::Role_Type r = Uml::B;
    UMLScene *scene = umlScene();
    UMLView *view = scene ? scene->activeView() : 0;

    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    ListPopupMenu::MenuType sel = menu->getMenuType(action);
    DEBUG(DBG_SRC) << "menu selection = " << ListPopupMenu::toString(sel);

    //if it's a collaboration message we now just use the code in floatingtextwidget
    //this means there's some redundant code below but that's better than duplicated code
    if (isCollaboration() && sel != ListPopupMenu::mt_Delete) {
        //TODO: Verify the working of following line of code.
        m_nameWidget->slotMenuSelection(action);
        return;
    }

    switch(sel) {
    case ListPopupMenu::mt_Properties:
        if(atype == Uml::AssociationType::Seq_Message || atype == Uml::AssociationType::Seq_Message_Self) {
            // show op dlg for seq. diagram here
            // don't worry about here, I don't think it can get here as
            // line is widget on seq. diagram
            // here just in case - remove later after testing
            DEBUG(DBG_SRC) << "mt_Properties: assoctype is " << atype.toString();
        } else {  //standard assoc dialog
            UMLScene *scene = umlScene();
            if (scene) {
                scene->updateDocumentation( false );
            }
            showPropertiesDialog();
        }
        break;

    case ListPopupMenu::mt_Delete:
        /* if (m_pAssocClassLineSel0)
            removeAssocClassLine();
        else */ if (association())
            scene->removeAssocInViewAndDoc(this);
        else
            scene->removeAssoc(this);
        break;

    case ListPopupMenu::mt_Rename_MultiA:
        r = Uml::A;   // fall through
    case ListPopupMenu::mt_Rename_MultiB:
        oldText = multiplicityWidget(r)->text();
        newText = KInputDialog::getText(i18n("Multiplicity"),
                                        i18n("Enter multiplicity:"),
                                        oldText, 0, view, &v);
        if (newText != oldText && FloatingTextWidget::isTextValid(newText)) {
            setMultiplicity(newText, r);
        }
        break;

    case ListPopupMenu::mt_Rename_Name:
        oldText = m_nameWidget->text();
        newText = KInputDialog::getText(i18n("Association Name"),
                                        i18n("Enter association name:"),
                                        oldText, 0, view, &v);
        if (newText != oldText && FloatingTextWidget::isTextValid(newText)) {
            setName(newText);
        }
        break;

    case ListPopupMenu::mt_Rename_RoleAName:
        r = Uml::A;   // fall through
    case ListPopupMenu::mt_Rename_RoleBName:
        oldText = roleName(r);
        newText = KInputDialog::getText(i18n("Role Name"),
                                        i18n("Enter role name:"),
                                        oldText, 0, view, &v);
        if (newText != oldText && FloatingTextWidget::isTextValid(newText)) {
            setRoleName(newText, r);
        }
        break;

    case ListPopupMenu::mt_Change_Font:
        font = this->font();
        if (KFontDialog::getFont(font, KFontChooser::NoDisplayFlags, view)) {
            lwSetFont(font);
        }
        break;

    case ListPopupMenu::mt_Change_Font_Selection:
        font = this->font();
        if (KFontDialog::getFont(font, KFontChooser::NoDisplayFlags, view)) {
            if (scene) {
                scene->selectionSetFont(font);
            }
            umlDoc()->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Line_Color:
    {
        QColor newColor = lineColor();
        if (KColorDialog::getColor(newColor) && scene) {
            scene->selectionSetLineColor(newColor);
            umlDoc()->setModified(true);
        }
    }
        break;

#if 0  // TODO:
    case ListPopupMenu::mt_Cut:
        m_pView->setStartedCut();
        UMLApp::app()->slotEditCut();
        break;
#endif

#if 0  // TODO:
    case ListPopupMenu::mt_Copy:
        UMLApp::app()->slotEditCopy();
        break;
#endif

#if 0  // TODO:
    case ListPopupMenu::mt_Paste:
        UMLApp::app()->slotEditPaste();
        break;
#endif

    case ListPopupMenu::mt_Reset_Label_Positions:
        resetTextPositions();
        break;

    default:
        DEBUG(DBG_SRC) << "MenuType " << ListPopupMenu::toString(sel) << " not implemented";
        break;
    } //end switch
}

/**
 * Reimplemented to handle updation of underlying UMLObject
 * @note syncToModel() is deprecated as I see no point in setting the object's variable
 * value to itself. Probably the intention was to emit the various changed signals indirectly,
 * which infact can be done directly.
 */
void AssociationWidget::slotUMLObjectDataChanged()
{
    UMLObject *obj = umlObject();
    if (!obj) {
        WidgetBase::slotUMLObjectDataChanged();
        return;
    }
    const UMLObject::ObjectType ot = obj->baseType();
    if (ot == UMLObject::ot_Operation) {
        if (m_nameWidget) {
            m_nameWidget->setMessageText();
        }
    } else if (ot == UMLObject::ot_Attribute) {
        UMLAttribute *attr = static_cast<UMLAttribute*>(obj);
        setVisibility(attr->visibility(), Uml::B);
        setRoleName(attr->name(), Uml::B);
    } else if (ot == UMLObject::ot_Association) {
        WidgetRole &a = m_role[Uml::A];
        WidgetRole &b = m_role[Uml::B];

        a.multiplicityWidget->setText(multiplicity(Uml::A));
        b.multiplicityWidget->setText(multiplicity(Uml::B));

        a.roleWidget->setPreText(visibility(Uml::A).toString(true));
        b.roleWidget->setPreText(visibility(Uml::B).toString(true));

        a.roleWidget->setText(roleName(Uml::A));
        b.roleWidget->setText(roleName(Uml::B));

        if (!m_slotUMLObjectDataChangedFirstCall) {
            a.changeabilityWidget->setText(changeability(Uml::A).toString());
            b.changeabilityWidget->setText(changeability(Uml::B).toString());
        } else {
            a.changeabilityWidget->setText("");
            b.changeabilityWidget->setText("");
        }
    }

    m_slotUMLObjectDataChangedFirstCall = false;
    WidgetBase::slotUMLObjectDataChanged();
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
    } else if (change == FontHasChanged) {
        QFont newFont = font();
        for (unsigned role = Uml::A; role <= Uml::B; ++role) {
            m_role[role].multiplicityWidget->setFont(newFont);
            m_role[role].changeabilityWidget->setFont(newFont);
            m_role[role].roleWidget->setFont(newFont);
        }
        m_nameWidget->setFont(newFont);
    } else if (change == NameHasChanged) {
        m_nameWidget->setText(name());
        updateNameWidgetRole();
    }
    return WidgetBase::attributeChange(change, oldValue);
}

void AssociationWidget::mousePressEvent(UMLSceneMouseEvent *event)
{
    setSelected(true);
    // and show association documentation in doc window
    umlScene()->showDocumentation(this, true);

    m_associationLine->mousePressEvent(event);
}

void AssociationWidget::mouseMoveEvent(UMLSceneMouseEvent *event)
{
    m_associationLine->mouseMoveEvent(event);
}

void AssociationWidget::mouseReleaseEvent(UMLSceneMouseEvent *event)
{
    m_associationLine->mouseReleaseEvent(event);
}

void AssociationWidget::mouseDoubleClickEvent(UMLSceneMouseEvent *event)
{
    DEBUG(DBG_SRC) << "at " << event->pos();
    m_associationLine->mouseDoubleClickEvent(event);
}

void AssociationWidget::hoverEnterEvent(UMLSceneHoverEvent *event)
{
    m_associationLine->hoverEnterEvent(event);
}

void AssociationWidget::hoverMoveEvent(UMLSceneHoverEvent *event)
{
    m_associationLine->hoverMoveEvent(event);
}

void AssociationWidget::hoverLeaveEvent(UMLSceneHoverEvent *event)
{
    m_associationLine->hoverLeaveEvent(event);
}

/**
 * Event handler for context menu events, called from the line segments.
 */
void AssociationWidget::contextMenu(const QPointF& pos, Qt::KeyboardModifiers modifiers, const QPoint& screenPos)
{
    Uml::AssociationType type = associationType();
    ListPopupMenu::MenuType menuType = ListPopupMenu::mt_Association_Selected;
    if (type == Uml::AssociationType::Anchor ||
            m_associationLine->onAssociationClassLine(pos)) {
        menuType = ListPopupMenu::mt_Anchor;
    } else if (isCollaboration()) {
        menuType = ListPopupMenu::mt_Collaboration_Message;
    } else if (!association()) {
        menuType = ListPopupMenu::mt_AttributeAssociation;
    } else if (AssocRules::allowRole(type)) {
        menuType = ListPopupMenu::mt_FullAssociation;
    }

    UMLScene *scene = umlScene();
    QWidget *parent = 0;
    if (scene) {
        parent = scene->activeView();
    }

    if (!isSelected() && scene && !scene->selectedItems().isEmpty()) {
        Qt::KeyboardModifiers forSelection = (Qt::ControlModifier | Qt::ShiftModifier);
        if ((modifiers & forSelection) == 0) {
            scene->clearSelection();
        }
    }
    setSelected(true);
    DEBUG(DBG_SRC) << "menue type = " << ListPopupMenu::toString(menuType);
    QPointer<ListPopupMenu> menu = new ListPopupMenu(parent, menuType, this);
    QAction *triggered = menu->exec(screenPos);
    ListPopupMenu *parentMenu = ListPopupMenu::menuFromAction(triggered);

    if (!parentMenu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }

    WidgetBase *ownerWidget = parentMenu->ownerWidget();
    // assert because logic is based on only WidgetBase being the owner of 
    // ListPopupMenu actions executed in this context menu.
    Q_ASSERT_X(ownerWidget != 0, "AssociationWidget::contextMenuEvent",
            "ownerWidget is null which means action belonging to UMLView, UMLScene"
            " or UMLObject is the one triggered in ListPopupMenu");

    ownerWidget->slotMenuSelection(triggered);

    delete menu.data();
}

/**
 * Reimplemented to cleanup connectivity with the old UMLObject.
 */ 
void AssociationWidget::umlObjectChanged(UMLObject *old)
{
    if (!old) {
        return;
    }

    const UMLObject::ObjectType ot = old->baseType();
    if (ot == UMLObject::ot_Association) {

        UMLAssociation *oldAssoc = static_cast<UMLAssociation*>(old);
        oldAssoc->nrof_parent_widgets--;
        // TODO: Discussion on ownership of UMLAssociation
        if (oldAssoc->nrof_parent_widgets == 0) {
            // TODO: Delete oldAssoc or not ? Depends on cut/copy implementation
        }

    } else if (ot == UMLObject::ot_Attribute) {

        UMLClassifier *klass = static_cast<UMLClassifier*>(old->parent());
        disconnect(klass, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));

    } else if (ot == UMLObject::ot_EntityAttribute) {

        UMLEntity *ent = static_cast<UMLEntity*>(old->parent());
        disconnect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));

    } else if (ot == UMLObject::ot_ForeignKeyConstraint) {

        UMLEntity *ent = static_cast<UMLEntity*>(old->parent());
        disconnect(ent, SIGNAL(entityConstraintRemoved(UMLClassifierListItem*)),
                this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));

    } else {
        uError() << "Had a wrong association of type " << ot;
    }

    WidgetBase::umlObjectChanged(old);
}

void AssociationWidget::setFloatingText(Uml::TextRole role, const QString& text, FloatingTextWidget* ft)
{
    if (! FloatingTextWidget::isTextValid(text)) {
        // FloatingTextWidgets are no longer deleted/reconstructed to make it easier.
        // Set an empty text, so that floatingtext is implicitly hidden.
        ft->setText("");
        return;
    }

    ft->setText(text);
    setTextPosition(role);
}

UMLScenePoint AssociationWidget::calculateTextPosition(Uml::TextRole role)
{
    const qreal SPACE = 2;
    QPointF p(-1, -1), q(-1, -1);

    // used to find out if association end point (p)
    // is at top or bottom edge of widget.

    if (role == Uml::TextRole::MultiA || role == Uml::TextRole::ChangeA || role == Uml::TextRole::RoleAName) {
        p = m_associationLine->point( 0 );
        q = m_associationLine->point( 1 );
    } else if (role == Uml::TextRole::MultiB || role == Uml::TextRole::ChangeB || role == Uml::TextRole::RoleBName) {
        const uint lastSegment = m_associationLine->count() - 1;
        p = m_associationLine->point(lastSegment);
        q = m_associationLine->point(lastSegment - 1);
    } else if (role != Uml::TextRole::Name) {
        uError() << "called with unsupported TextRole " << role.toString();
        return QPoint(-1, -1);
    }

    FloatingTextWidget *text = textWidgetByRole(role);
    qreal textW = 0, textH = 0;
    if (text) {
        textW = text->width();
        textH = text->height();
    }

    qreal x = 0, y = 0;

    if (role == Uml::TextRole::MultiA || role == Uml::TextRole::MultiB) {
        const bool isHorizontal = (p.y() == q.y());
        const qreal atBottom = p.y() + SPACE;
        const qreal atTop = p.y() - SPACE - textH;
        const qreal atLeft = p.x() - SPACE - textW;
        const qreal atRight = p.x() + SPACE;
        y = (p.y() > q.y()) == isHorizontal ? atBottom : atTop;
        x = (p.x() < q.x()) == isHorizontal ? atRight : atLeft;

    } else if (role == Uml::TextRole::ChangeA || role == Uml::TextRole::ChangeB) {

        if( p.y() > q.y() ) {
            y = p.y() - SPACE - (textH * 2);
        }
        else {
            y = p.y() + SPACE + textH;
        }

        if( p.x() < q.x() ) {
            x = p.x() + SPACE;
        }
        else {
            x = p.x() - SPACE - textW;
        }

    } else if (role == Uml::TextRole::RoleAName || role == Uml::TextRole::RoleBName) {

        if( p.y() > q.y() ) {
            y = p.y() - SPACE - textH;
        }
        else {
            y = p.y() + SPACE;
        }

        if( p.x() < q.x() ) {
            x = p.x() + SPACE;
        }
        else {
            x = p.x() - SPACE - textW;
        }

    } else if (role == Uml::TextRole::Name) {

        calculateNameTextSegment();
        x = ( m_associationLine->point(m_nameSegmentIndex).x() +
                m_associationLine->point(m_nameSegmentIndex + 1).x() ) / 2.0;

        y = ( m_associationLine->point(m_nameSegmentIndex).y() +
                m_associationLine->point(m_nameSegmentIndex + 1).y() ) / 2.0;
    }

    if (text) {
        constrainTextPos(x, y, textW, textH, role);
    }
    p = QPointF( x, y );
    DEBUG(DBG_SRC) << "set to [" << p.x() << ", " << p.y() << "].";  //:TODO:
    return p;
}

void AssociationWidget::setTextPosition(Uml::TextRole role)
{
    bool startMove = false;
    //TODO: Check startMove removal
    if (startMove) {
        return;
    }
    FloatingTextWidget *ft = textWidgetByRole(role);
    if (!ft) {
        return;
    }
    QPointF pos = calculateTextPosition(role);
    if ( (pos.x() < 0.0 || pos.x() > FloatingTextWidget::restrictPositionMax) ||
            (pos.y() < 0.0 || pos.y() > FloatingTextWidget::restrictPositionMax) ) {
        DEBUG(DBG_SRC) << "(x=" << pos.x() << " , y=" << pos.y() << ") "
            << "- was blocked because at least one value is out of bounds: ["
            << "0 ... " << FloatingTextWidget::restrictPositionMax << "]";
        return;
    }
    DEBUG(DBG_SRC) << "set to [" << pos.x() << ", " << pos.y() << "].";  //:TODO:
    ft->setPos(pos);
}

void AssociationWidget::updateNameWidgetRole()
{
    Uml::TextRole textRole = Uml::TextRole::Name;
    UMLScene *scene = umlScene();
    if (scene) {
        if (scene->type() == Uml::DiagramType::Collaboration) {
            if (isSelf()) {
                textRole = Uml::TextRole::Coll_Message;
            } else {
                textRole = Uml::TextRole::Coll_Message;
            }
        } else if (scene->type() == Uml::DiagramType::Sequence) {
            if (isSelf()) {
                textRole = Uml::TextRole::Seq_Message_Self;
            } else {
                textRole = Uml::TextRole::Seq_Message;
            }
        }
    }

    m_nameWidget->setTextRole(textRole);
}

/**
 * Initialize attributes of this class at construction time.
 */
void AssociationWidget::init()
{
    m_associationType = Uml::AssociationType::Association;
    m_associationClass = 0;
    m_associationLine = new AssociationLine(this);
    m_slotUMLObjectDataChangedFirstCall = true;

    m_nameWidget = new FloatingTextWidget(Uml::TextRole::Name);
    m_nameWidget->setLink(this);
    m_nameSegmentIndex = -1;

    m_role[Uml::A].initFloatingWidgets(Uml::A, this);
    m_role[Uml::B].initFloatingWidgets(Uml::B, this);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);

    DEBUG_REGISTER(DBG_SRC);
}

/**
 * Saves this widget to the "assocwidget" XMI element.
 */
void AssociationWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    QDomElement assocElement = qDoc.createElement("assocwidget");

    WidgetBase::saveToXMI(qDoc, assocElement);
    if (umlObject()) {
        assocElement.setAttribute("xmi.id", ID2STR(umlObject()->id()));
    }
    assocElement.setAttribute("type", associationType());
    if (!association()) {
        assocElement.setAttribute("visibilityA", visibility(Uml::A));
        assocElement.setAttribute("visibilityB", visibility(Uml::B));
        assocElement.setAttribute("changeabilityA", changeability(Uml::A));
        assocElement.setAttribute("changeabilityB", changeability(Uml::B));
        if (!umlObject()) {
            assocElement.setAttribute("roleAdoc", roleDocumentation(Uml::A));
            assocElement.setAttribute("roleBdoc", roleDocumentation(Uml::B));
            assocElement.setAttribute("documentation", documentation());
        }
    }
    assocElement.setAttribute("widgetaid", ID2STR(widgetIDForRole(Uml::A)));
    assocElement.setAttribute("widgetbid", ID2STR(widgetIDForRole(Uml::B)));
#if 0
//TODO: Port
    assocElement.setAttribute("indexa", m_role[A].m_nIndex);
    assocElement.setAttribute("indexb", m_role[B].m_nIndex);
    assocElement.setAttribute("totalcounta", m_role[A].m_nTotalCount);
    assocElement.setAttribute("totalcountb", m_role[B].m_nTotalCount);
#endif
    m_associationLine->saveToXMI(qDoc, assocElement);

    if (m_nameWidget->hasValidText()) {
        m_nameWidget->saveToXMI(qDoc, assocElement);
    }

    if (multiplicityWidget(Uml::A)->hasValidText()) {
        multiplicityWidget(Uml::A)->saveToXMI(qDoc, assocElement);
    }

    if (multiplicityWidget(Uml::B)->hasValidText()) {
        multiplicityWidget(Uml::B)->saveToXMI(qDoc, assocElement);
    }

    if (roleWidget(Uml::A)->hasValidText()) {
        roleWidget(Uml::A)->saveToXMI(qDoc, assocElement);
    }

    if (roleWidget(Uml::B)->hasValidText()) {
        roleWidget(Uml::B)->saveToXMI(qDoc, assocElement);
    }

    if (changeabilityWidget(Uml::A)->hasValidText()) {
        changeabilityWidget(Uml::A)->saveToXMI(qDoc, assocElement);
    }

    if (changeabilityWidget(Uml::B)->hasValidText()) {
        changeabilityWidget(Uml::B)->saveToXMI(qDoc, assocElement);
    }

    if (m_associationClass) {
        QString acid = ID2STR(m_associationClass->id());
        assocElement.setAttribute("assocclass", acid);
#if 0
//TODO: port
        assocElement.setAttribute("aclsegindex", m_nLinePathSegmentIndex);
#endif
    }

    qElement.appendChild(assocElement);
}

/**
 * Uses the supplied widgetList for resolving
 * the role A and role B widgets. (The other loadFromXMI() queries
 * the UMLView for these widgets.)
 * Required for clipboard operations.
 */
bool AssociationWidget::loadFromXMI(QDomElement& qElement,
                                    const UMLWidgetList& widgets,
                                    const MessageWidgetList* messages)
{
    if (!WidgetBase::loadFromXMI(qElement)) {
        return false;
    }

    // load child widgets first
    QString widgetaid = qElement.attribute("widgetaid", "-1");
    QString widgetbid = qElement.attribute("widgetbid", "-1");
    Uml::IDType aId = STR2ID(widgetaid);
    Uml::IDType bId = STR2ID(widgetbid);
    UMLWidget *pWidgetA = Widget_Utils::findWidget(aId, widgets, messages);
    if (!pWidgetA) {
        uError() << "cannot find widget for roleA id " << ID2STR(aId);
        return false;
    }
    UMLWidget *pWidgetB = Widget_Utils::findWidget(bId, widgets, messages);
    if (!pWidgetB) {
        uError() << "cannot find widget for roleB id " << ID2STR(bId);
        return false;
    }
    setWidgets(pWidgetA, pWidgetB);

    QString type = qElement.attribute("type", "-1");
    Uml::AssociationType aType = Uml::AssociationType(Uml::AssociationType::Value(type.toInt()));

    UMLObject *object = umlObject();
    QString id = qElement.attribute("xmi.id", "-1");
    bool oldStyleLoad = false;
    if (id == "-1") {
        // xmi.id not present, ergo either a pure widget association,
        // or old (pre-1.2) style:
        // Everything is loaded from the AssociationWidget.
        // UMLAssociation may or may not be saved - if it is, it's a dummy.
        // Create the UMLAssociation if both roles are UML objects;
        // else load the info locally.

        if (Uml::AssociationType::hasUMLRepresentation(aType)) {
            // lack of an association in our widget AND presence of
            // both uml objects for each role clearly identifies this
            // as reading in an old-school file. Note it as such, and
            // create, and add, the UMLAssociation for this widget.
            // Remove this special code when backwards compatibility
            // with older files isn't important anymore. -b.t.
            UMLObject* umlRoleA = pWidgetA->umlObject();
            UMLObject* umlRoleB = pWidgetB->umlObject();
            if (!object && umlRoleA && umlRoleB) {
                oldStyleLoad = true; // flag for further special config below
                if (aType == Uml::AssociationType::Aggregation || aType == Uml::AssociationType::Composition) {
                    uWarning()
                        << " Old Style save file? swapping roles on association widget"
                        <<(void*)this;
                    // We have to swap the A and B widgets to compensate
                    // for the long standing bug in LinePath of drawing
                    // the diamond at the wrong end which was fixed
                    // just before the 1.2 release.
                    // The logic here is that the user has understood
                    // that the diamond belongs at the SOURCE end of the
                    // the association (i.e. at the container, not at the
                    // contained), and has compensated for this anomaly
                    // by drawing the aggregations/compositions from
                    // target to source.
                    UMLWidget *tmpWidget = pWidgetA;
                    pWidgetA = pWidgetB;
                    pWidgetB = tmpWidget;
                    setWidgetForRole(pWidgetA, Uml::A);
                    setWidgetForRole(pWidgetB, Uml::B);
                    umlRoleA = pWidgetA->umlObject();
                    umlRoleB = pWidgetB->umlObject();
                }

                setUMLObject(umlDoc()->createUMLAssociation(umlRoleA, umlRoleB, aType));
            }
        }

        setDocumentation(qElement.attribute("documentation", ""));
        setRoleDocumentation(qElement.attribute("roleAdoc", ""), Uml::A);
        setRoleDocumentation(qElement.attribute("roleBdoc", ""), Uml::B);

        // visibility defaults to Public if it cant set it here..
        QString visibilityA = qElement.attribute("visibilityA", "0");
        int vis = visibilityA.toInt();
        if (vis >= 200) {  // bkwd compat.
            vis -= 200;
        }
        setVisibility((Uml::Visibility::Value)vis, Uml::A);

        QString visibilityB = qElement.attribute("visibilityB", "0");
        vis = visibilityB.toInt();
        if (vis >= 200) { // bkwd compat.
            vis -= 200;
        }
        setVisibility((Uml::Visibility::Value)vis, Uml::B);

        // Changeability defaults to "Changeable" if it cant set it here..
        QString changeabilityA = qElement.attribute("changeabilityA", "0");
        if (changeabilityA.toInt() > 0) {
            setChangeability(Uml::Changeability(Uml::Changeability::Value(changeabilityA.toInt())), Uml::A);
        }

        QString changeabilityB = qElement.attribute("changeabilityB", "0");
        if (changeabilityB.toInt() > 0) {
            setChangeability(Uml::Changeability(Uml::Changeability::Value(changeabilityB.toInt())), Uml::B);
        }

    } else {

        // we should disconnect any prior association (can this happen??)
        if (object && object->baseType() == UMLObject::ot_Association) {
            UMLAssociation *umla = association();
            umla->disconnect(this);
            umla->nrof_parent_widgets--;
        }

        // New style: The xmi.id is a reference to the UMLAssociation.
        // If the UMLObject is not found right now, we try again later
        // during the type resolution pass - see activate().
        setID(STR2ID(id));
        UMLObject *myObj = umlDoc()->findObjectById(this->id());
        if (myObj) {
            const UMLObject::ObjectType ot = myObj->baseType();
            setUMLObject(myObj);
            if (ot == UMLObject::ot_Association) {
                aType = static_cast<UMLAssociation*>(myObj)->getAssocType();
            }
        }
    }

    setAssociationType(aType);

#if 0
//TODO: Check if its needed anymore
    QString indexa = qElement.attribute("indexa", "0");
    QString indexb = qElement.attribute("indexb", "0");
    QString totalcounta = qElement.attribute("totalcounta", "0");
    QString totalcountb = qElement.attribute("totalcountb", "0");
    m_role[A].m_nIndex = indexa.toInt();
    m_role[B].m_nIndex = indexb.toInt();
    m_role[A].m_nTotalCount = totalcounta.toInt();
    m_role[B].m_nTotalCount = totalcountb.toInt();
#endif

    QString assocclassid = qElement.attribute("assocclass", "");
    if (! assocclassid.isEmpty()) {
        Uml::IDType acid = STR2ID(assocclassid);
        UMLWidget *w = Widget_Utils::findWidget(acid, widgets);
        if (w) {
            m_associationClass = static_cast<ClassifierWidget*>(w);
            m_associationClass->setClassAssociationWidget(this);
            // Preparation of the assoc class line is done in activate()
            QString aclsegindex = qElement.attribute("aclsegindex", "0");
#if 0
//TODO: Fix after implementing segment support for classifier line.
            m_nLinePathSegmentIndex = aclsegindex.toInt();
#endif
        } else {
            uError() << "cannot find assocclass " << assocclassid;
        }
    }

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    while(!element.isNull()) {
        QString tag = element.tagName();
        if(tag == "linepath") {
            if (m_associationLine->loadFromXMI(element) == false) {
                return false;
            } else {
#if 0
//TODO: Check whether following can be removed.
                // set up 'old' corner from first point in line
                // as IF this ISNT done, then the subsequent call to
                // widgetMoved will inadvertantly think we have made a
                // big move in the position of the association when we haven't.
                QPoint p = m_LinePath.getPoint(0);
                m_role[A].m_OldCorner.setX(p.x());
                m_role[A].m_OldCorner.setY(p.y());
#endif
            }
        } else if (tag == "floatingtext" ||
                tag == "UML:FloatingTextWidget") {  // for bkwd compatibility
            QString r = element.attribute("role", "-1");
            if(r == "-1") {
                return false;
            }
            Uml::TextRole role = Uml::TextRole::Value(r.toInt());
            // FloatingTextWidget are created in constructor and hence valid always.
            // They will be hidden in case of invalid text set.
            FloatingTextWidget *ft = textWidgetByRole(role);
            if(! ft->loadFromXMI(element)) {
                // Most likely cause: The FloatingTextWidget is empty.
                // Ensure it hides by setting it empty again.
                // @see FloatingTextWidget::setText
                ft->setText("");
                node = element.nextSibling();
                element = node.toElement();
                continue;
            }
            // always need this
            ft->setLink(this);

            // Changes from TRUNK(before soc branch merge) version:
            // We use ft directly as there is no point in fetching the same pointer
            // by calling specific multiplicityWidget like methods.
            DEBUG(DBG_SRC) << "oldStyleLoad = " << oldStyleLoad;
            if (ft) {
                switch(role) {
                    case Uml::TextRole::MultiA:
                        if(oldStyleLoad) {
                            setMultiplicity(ft->text(), Uml::A);
                        }
                        break;

                    case Uml::TextRole::MultiB:
                        if(oldStyleLoad) {
                            setMultiplicity(ft->text(), Uml::B);
                        }
                        break;

                    case Uml::TextRole::ChangeA:
                    case Uml::TextRole::ChangeB:
                        break;

                    case Uml::TextRole::Name:
                        if(oldStyleLoad) {
                            setName(ft->text());
                        }
                        break;

                    case Uml::TextRole::Coll_Message:
                    case Uml::TextRole::Coll_Message_Self:
                        ft->setLink(this);
                        ft->setActivatedFlag(true);
                        break;

                    case Uml::TextRole::RoleAName:
                        setRoleName(ft->text(), Uml::A);
                        break;

                    case Uml::TextRole::RoleBName:
                        setRoleName(ft->text(), Uml::B);
                        break;

                    default:
                        DEBUG(DBG_SRC) << "unexpected FloatingTextWidget (textrole " << role.toString() << ")";
#if 0
//TODO: Investigate this delete. Firstly, is this reachable ?
                        delete ft;
#endif
                        break;
                }
            }
        }
        node = element.nextSibling();
        element = node.toElement();
    }

    return true;
}

/**
 *
 */
bool AssociationWidget::loadFromXMI(QDomElement& qElement)
{
    UMLScene *scene = umlScene();
    if (scene) {
        const UMLWidgetList& widgetList = scene->widgetList();
        const MessageWidgetList& messageList = scene->messageList();
        return loadFromXMI(qElement, widgetList, &messageList);
    }
    else {
        DEBUG(DBG_SRC) << "This isn't on UMLScene yet, so can neither fetch"
            "messages nor widgets on umlscene";
        return false;
    }
}

#include "associationwidget.moc"
