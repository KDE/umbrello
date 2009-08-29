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

#include "associationwidget.h"

#include "association.h"
#include "associationline.h"
#include "assocrules.h"
#include "associationspacemanager.h"
#include "attribute.h"
#include "classifier.h"
#include "classifierwidget.h"
#include "dialogs/assocpropdlg.h"
#include "entity.h"
#include "floatingtextwidget.h"
#include "objectwidget.h"
#include "operation.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlwidget.h"
#include "umlview.h"
#include "widget_utils.h"

#include <kinputdialog.h>
#include <klocale.h>
#include <kcolordialog.h>

#include <QPointer>
#include <QRegExpValidator>

#include <cmath>

WidgetRole::WidgetRole()
{
    multiplicityWidget = changeabilityWidget = roleWidget = 0;
    umlWidget = 0;
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

void WidgetRole::initFloatingWidgets(Uml::Role_Type role, AssociationWidget *parent)
{
    Uml::Text_Role textRole = (role == Uml::A ? Uml::tr_MultiA : Uml::tr_MultiB);
    multiplicityWidget = new FloatingTextWidget(textRole);
    multiplicityWidget->setLink(parent);

    textRole = (role == Uml::A ? Uml::tr_ChangeA : Uml::tr_ChangeB);
    changeabilityWidget = new FloatingTextWidget(textRole);
    changeabilityWidget->setPreText("{");
    changeabilityWidget->setPostText("}");
    changeabilityWidget->setText("");
    changeabilityWidget->setLink(parent);

    textRole = (role == Uml::A ? Uml::tr_RoleAName : Uml::tr_RoleBName);
    roleWidget = new FloatingTextWidget(textRole);
    roleWidget->setLink(parent);

    // TODO: Activation of floating text widgets
}


AssociationWidget::AssociationWidget() : WidgetBase(0)
{
    init();
}

AssociationWidget::AssociationWidget(UMLWidget *widgetA, Uml::Association_Type type,
        UMLWidget *widgetB, UMLObject *umlObj) :
    WidgetBase(0) // Set UMLObject in body
{
    init();
    if (umlObj) {
        setUMLObject(umlObj);
    } else if (UMLAssociation::assocTypeHasUMLRepresentation(type)) {
        UMLObject *objectA = widgetA->umlObject();
        UMLObject *objectB = widgetB->umlObject();

        if (objectA && objectB) {
             bool swap = false;

             // THis isnt correct. We could very easily have more than one
             // of the same type of association between the same two objects.
             // Just create the association. This search should have been
             // done BEFORE creation of the widget, if it mattered to the code.
             // But lets leave check in here for the time being so that debugging
             // output is shown, in case there is a collision with code elsewhere.
             UMLAssociation * myAssoc = umlDoc()->findAssociation(type,
                    objectA, objectB, &swap);
             if (myAssoc) {
                 if (type == Uml::at_Generalization) {
                     uDebug() << "Ignoring second construction of same generalization";
                 } else {
                     uDebug() << "Constructing a similar or exact same assoc " <<
                         "as an already existing assoc (swap=" << swap << ")";
                     // now, just create a new association anyways
                     myAssoc = 0;
                 }
             }

             if (!myAssoc) {
                myAssoc = new UMLAssociation(type, objectA, objectB);
             }

             setUMLObject(myAssoc);
        }
    }

    setWidgetForRole(widgetA, Uml::A);
    setWidgetForRole(widgetB, Uml::B);

    setAssociationType(type);

    if (widgetA == widgetB) {
        widgetA->associationSpaceManager()->add(this,
                RegionPair(Uml::reg_North, Uml::reg_North));
    } else {
        widgetA->associationSpaceManager()->add(this,
                RegionPair(Uml::reg_North));
        widgetB->associationSpaceManager()->add(this,
                RegionPair(Uml::reg_Error, Uml::reg_North));
    }
    // TODO: Probably move this calculation to slotInit.
    m_associationLine->calculateInitialEndPoints();
    m_associationLine->reconstructSymbols();


    Q_ASSERT(widgetA->umlScene() == widgetB->umlScene());

    if (!isCollaboration()) {
        m_setCollabIDOnFirstSceneSet = false;
        setActivatedFlag(true);
    } else {
        // Activation flag is set in AssociationWidget::sceneSetFirstTime as
        // generation of collaboration id requrires this widget to be on
        // UMLScene.
        m_setCollabIDOnFirstSceneSet = true;
    }
    updateNameWidgetRole();
}

AssociationWidget::~AssociationWidget()
{
    delete m_associationLine;
}

/**
 * Reimplemented to do more checks and changes while setting a new UMLObject.
 * The old UMLObject's connectivity is removed in @ref umlObjectChanged method, which is
 * invoked by WidgetBase::setUMLObject.
 */
void AssociationWidget::setUMLObject(UMLObject *obj)
{
    if (obj == umlObject()) {
        return;
    }
    if (!obj) {
        WidgetBase::setUMLObject(0);
        return;
    }

    const Uml::Object_Type ot = obj->getBaseType();
    if (ot == Uml::ot_Association) {

        UMLAssociation *assoc = static_cast<UMLAssociation*>(obj);
        if (assoc->nrof_parent_widgets < 0) {
            assoc->nrof_parent_widgets = 0;
        }
        assoc->nrof_parent_widgets++;

    } else if (ot == Uml::ot_Operation) {

        // Nothing special to do.

    } else if (ot == Uml::ot_Attribute) {

        UMLClassifier *klass = static_cast<UMLClassifier*>(obj->parent());
        connect(klass, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
        // attributeChanged is emitted along with modified signal. So its not
        // necessary to handle attributeChanged signal.

    } else if (ot == Uml::ot_EntityAttribute) {

        UMLEntity *ent = static_cast<UMLEntity*>(obj->parent());
        connect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));

    } else if (ot == Uml::ot_ForeignKeyConstraint) {

        UMLEntity *ent = static_cast<UMLEntity*>(obj->parent());
        connect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));

    } else {

        uError() << "UMLAssociation constructor: cannot associate UMLObject of type " << ot;

    }

    WidgetBase::setUMLObject(obj);
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


void AssociationWidget::showPropertiesDialog()
{
    UMLView *view = umlScene() ? umlScene()->activeView() : 0;

    QPointer<AssocPropDlg> dlg = new AssocPropDlg(view, this );
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

        if (umlScene()) {
            umlScene()->showDocumentation(this, true);
        }
    }
    delete dlg;
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


QGraphicsEllipseItem *eellipse = 0;
QGraphicsLineItem *lline = 0;
void AssociationWidget::constrainTextPos(qreal &textX, qreal &textY, qreal textWidth,
        qreal textHeight, Uml::Text_Role tr)
{
    const QPointF textCenter = QRectF(textX, textY, textWidth, textHeight).center();
    const uint numSegments = m_associationLine->count()-1;
    const uint lastSegmentIndex = numSegments-1;

    if (numSegments == 0) {
        uError() << "Called when it had zero segments";
        return;
    }
    QPointF p0, p1;
    switch (tr) {
        case Uml::tr_RoleAName:
        case Uml::tr_MultiA:
        case Uml::tr_ChangeA:
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

        case Uml::tr_RoleBName:
        case Uml::tr_MultiB:
        case Uml::tr_ChangeB:
            p0 = m_associationLine->point(lastSegmentIndex);
            p1 = m_associationLine->point(lastSegmentIndex+1);
            if (numSegments == 1) {
                p0 = (p0 + p1)/2.0;
            }
            break;

        case Uml::tr_Name:
        case Uml::tr_Coll_Message:  // CHECK: collab.msg texts seem to be tr_Name
        case Uml::tr_State:         // CHECK: is this used?
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
            uError() << "unexpected Text_Role " << tr;
            return;
    }
    /* Constraint:
       The midpoint between p0 and p1 is taken to be the center of a circle
       with radius D/2 where D is the distance between p0 and p1.
       The text center needs to be within this circle else it is constrained
       to the nearest point on the circle.
     */
    const QPointF mid = (p0 + p1) / 2.0;
    const qreal radius = QLineF(p0, mid).length();

    QPointF projected = textCenter;
    QLineF line(mid, projected);
    if (line.length() > radius) {
        line.setLength(radius);
        projected = line.p2();
    }

    if (!eellipse) {
        eellipse = new QGraphicsEllipseItem(this);
        lline = new QGraphicsLineItem(this);
    }
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


UMLAssociation* AssociationWidget::association() const
{
    if (!umlObject() || umlObject()->getBaseType() != Uml::ot_Association) {
        return 0;
    }
    return static_cast<UMLAssociation*>(umlObject());
}

UMLAttribute* AssociationWidget::attribute() const
{
    if (!umlObject()) {
        return 0;
    }
    Uml::Object_Type ot = umlObject()->getBaseType();
    if (ot != Uml::ot_Attribute && ot != Uml::ot_EntityAttribute) {
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

FloatingTextWidget* AssociationWidget::multiplicityWidget(Uml::Role_Type role) const
{
    return m_widgetRole[role].multiplicityWidget;
}

QString AssociationWidget::multiplicity(Uml::Role_Type role) const
{
    if (association()) {
        return association()->getMulti(role);
    }
    return m_widgetRole[role].multiplicityWidget->text();
}

void AssociationWidget::setMultiplicity(const QString& text, Uml::Role_Type role)
{
    uDebug() << "Called with " << text << "AB"[role] << " role";
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
    if (value == visibility(role)) {
        return;
    }

    UMLAttribute *attrib = attribute();
    UMLAssociation *assoc = association();

    m_widgetRole[role].visibility = value;
    if (attrib) {
        attrib->setVisibility(value);
    } else if (assoc) {
        assoc->setVisibility(value, role);
    } else {
        QString scopeString = value.toString(true);
        m_widgetRole[role].roleWidget->setPreText(scopeString);
    }
}

FloatingTextWidget* AssociationWidget::changeabilityWidget(Uml::Role_Type role) const
{
    return m_widgetRole[role].changeabilityWidget;
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
    m_widgetRole[role].changeability = c;
    if (association()) {
        association()->setChangeability(c, role);
    } else {
        m_widgetRole[role].changeabilityWidget->setText(UMLAssociation::toString(c));
    }
}

FloatingTextWidget* AssociationWidget::nameWidget() const
{
    return m_nameWidget;
}

FloatingTextWidget* AssociationWidget::roleWidget(Uml::Role_Type role) const
{
    return m_widgetRole[role].roleWidget;
}

QString AssociationWidget::roleName(Uml::Role_Type role) const
{
    if (association()) {
        return association()->getRoleName(role);
    }
    return m_widgetRole[role].roleWidget->text();
}

void AssociationWidget::setRoleName(const QString &strRole, Uml::Role_Type role)
{
    //if the association is not supposed to have a Role FloatingTextWidget
    if (!AssocRules::allowRole(associationType()))  {
        m_widgetRole[role].roleWidget->setText("");
        return;
    }

    if (association()) {
        association()->setRoleName(strRole, role);
        return;
    }

    m_widgetRole[role].roleWidget->setText(strRole);
    m_widgetRole[role].roleWidget->setPreText(visibility(role).toString(true));
}

QString AssociationWidget::roleDocumentation(Uml::Role_Type role) const
{
    UMLAssociation *umlAssoc = association();
    if (!umlAssoc) {
        return QString();
    }
    return umlAssoc->getRoleDoc(role);
}

void AssociationWidget::setRoleDocumentation(const QString& doc, Uml::Role_Type role)
{
    UMLAssociation *umlAssoc = association();
    if (!umlAssoc) {
        m_widgetRole[role].roleDocumentation = doc;
    } else {
        umlAssoc->setRoleDoc(doc, role);
    }
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

FloatingTextWidget* AssociationWidget::textWidgetByRole(Uml::Text_Role tr) const
{
    switch (tr) {
        case Uml::tr_MultiA:
            return m_widgetRole[Uml::A].multiplicityWidget;

        case Uml::tr_MultiB:
            return m_widgetRole[Uml::B].multiplicityWidget;

        case Uml::tr_Name:
        case Uml::tr_Coll_Message:
            return m_nameWidget;

        case Uml::tr_RoleAName:
            return m_widgetRole[Uml::A].roleWidget;

        case Uml::tr_RoleBName:
            return m_widgetRole[Uml::B].roleWidget;

        case Uml::tr_ChangeA:
            return m_widgetRole[Uml::A].changeabilityWidget;

        case Uml::tr_ChangeB:
            return m_widgetRole[Uml::B].changeabilityWidget;

        default:
            break;
    }

    return 0;
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

UMLWidget* AssociationWidget::widgetForRole(Uml::Role_Type role) const
{
    return m_widgetRole[role].umlWidget;
}

void AssociationWidget::setWidgetForRole(UMLWidget *widget, Uml::Role_Type role)
{
    m_widgetRole[role].umlWidget = widget;
}

Uml::IDType AssociationWidget::widgetIDForRole(Uml::Role_Type role) const
{
    UMLWidget *widget = widgetForRole(role);
    if (widget) {
        if (widget->baseType() == Uml::wt_Object) {
            return static_cast<ObjectWidget*>(widget)->localID();
        }
        return widget->id();
    }

    UMLAssociation *assoc = association();
    if (assoc) {
        return assoc->getObjectId(role);
    }

    uError() << "m_widgetRole[role].umlWidget is NULL";
    return Uml::id_None;
}

Uml::Association_Type AssociationWidget::associationType() const
{
    if (!umlObject() || umlObject()->getBaseType() != Uml::ot_Association) {
        return m_associationType;
    }

    return static_cast<UMLAssociation*>(umlObject())->getAssocType();
}

void AssociationWidget::setAssociationType(Uml::Association_Type type)
{
    m_associationType = type;
    if (umlObject() && umlObject()->getBaseType() == Uml::ot_Association) {
        static_cast<UMLAssociation*>(umlObject())->setAssocType(type);
    }

    WidgetRole &a = m_widgetRole[Uml::A];
    WidgetRole &b = m_widgetRole[Uml::B];

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
    Uml::Association_Type at = associationType();
    return (at == Uml::at_Coll_Message || at == Uml::at_Coll_Message_Self);
}

QString AssociationWidget::toString() const
{
    QString string;
    static const QChar colon(':');


    if (widgetForRole(Uml::A)) {
        string = widgetForRole(Uml::A)->name();
    }
    string.append(colon);

    if (m_widgetRole[Uml::A].roleWidget) {
        string += m_widgetRole[Uml::A].roleWidget->text();
    }
    string.append(colon);
    string.append( UMLAssociation::toString(associationType()));
    string.append(colon);

    if (widgetForRole(Uml::B)) {
        string += widgetForRole(Uml::B)->name();
    }

    string.append(colon);
    if (m_widgetRole[Uml::B].roleWidget) {
        string += m_widgetRole[Uml::B].roleWidget->text();
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
    bool hasUMLRepresentation =
        UMLAssociation::assocTypeHasUMLRepresentation(associationType());
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

    Uml::Association_Type type = associationType();
    if (!widgetForRole(Uml::A)) {
        setWidgetForRole(umlScene()->findWidget(widgetIDForRole(Uml::A)), Uml::A);
    }
    if (!widgetForRole(Uml::B)) {
        setWidgetForRole(umlScene()->findWidget(widgetIDForRole(Uml::B)), Uml::B);
    }


    if (!widgetForRole(Uml::A) || !widgetForRole(Uml::B)) {
        return false;
    }

    // TODO: Check whether this comment should be removed.
    //calculateEndPoints();

    if (AssocRules::allowRole(type)) {
        for (unsigned r = Uml::A; r <= Uml::B; ++r) {
            WidgetRole& robj = m_widgetRole[r];
            if (!robj.roleWidget) {
                continue;
            }
            Uml::Visibility vis = visibility((Uml::Role_Type)r);
            robj.roleWidget->setPreText(vis.toString(true));

            if (umlScene()->getType() == Uml::dt_Collaboration) {
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

    setActivatedFlag(true);
    return true;
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

bool AssociationWidget::loadFromXMI(QDomElement& qElement, const UMLWidgetList &widgets,
        const MessageWidgetList* pMessages)
{
    WidgetBase::loadFromXMI(qElement);

    // load child widgets first
    QString widgetaid = qElement.attribute("widgetaid", "-1");
    QString widgetbid = qElement.attribute("widgetbid", "-1");
    Uml::IDType aId = STR2ID(widgetaid);
    Uml::IDType bId = STR2ID(widgetbid);
    UMLWidget *pWidgetA = Widget_Utils::findWidget(aId, widgets, pMessages);
    if (!pWidgetA) {
        uError() << "cannot find widget for roleA id " << ID2STR(aId);
        return false;
    }
    UMLWidget *pWidgetB = Widget_Utils::findWidget(bId, widgets, pMessages);
    if (!pWidgetB) {
        uError() << "cannot find widget for roleB id " << ID2STR(bId);
        return false;
    }
    setWidgetForRole(pWidgetA, Uml::A);
    setWidgetForRole(pWidgetB, Uml::B);

    QString type = qElement.attribute("type", "-1");
    Uml::Association_Type aType = (Uml::Association_Type) type.toInt();

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

        if (UMLAssociation::assocTypeHasUMLRepresentation(aType)) {
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
                if (aType == Uml::at_Aggregation || aType == Uml::at_Composition) {
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
            setChangeability((Uml::Changeability_Type)changeabilityA.toInt(), Uml::A);
        }

        QString changeabilityB = qElement.attribute("changeabilityB", "0");
        if (changeabilityB.toInt() > 0) {
            setChangeability((Uml::Changeability_Type)changeabilityB.toInt(), Uml::B);
        }

    } else {

        // we should disconnect any prior association (can this happen??)
        if (object && object->getBaseType() == Uml::ot_Association) {
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
            const Uml::Object_Type ot = myObj->getBaseType();
            setUMLObject(myObj);
            if (ot == Uml::ot_Association) {
                aType = static_cast<UMLAssociation*>(myObj)->getAssocType();
            }
        }
    }

    setAssociationType(aType);

    // TODO: Check if its needed anymore
#if 0
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
            //TODO: Fix after implementing segment support for classifier line.
#if 0
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
                //TODO: Check whether following can be removed.
#if 0
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
            Uml::Text_Role role = (Uml::Text_Role)r.toInt();
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
            if (ft) {
                switch(role) {
                    case Uml::tr_MultiA:
                        if(oldStyleLoad) {
                            setMultiplicity(ft->text(), Uml::A);
                        }
                        break;

                    case Uml::tr_MultiB:
                        if(oldStyleLoad) {
                            setMultiplicity(ft->text(), Uml::B);
                        }
                        break;

                    case Uml::tr_ChangeA:
                    case Uml::tr_ChangeB:
                        break;

                    case Uml::tr_Name:
                        if(oldStyleLoad) {
                            setName(ft->text());
                        }
                        break;

                    case Uml::tr_Coll_Message:
                    case Uml::tr_Coll_Message_Self:
                        ft->setLink(this);
                        ft->setActivatedFlag(true);
                        break;

                    case Uml::tr_RoleAName:
                        setRoleName(ft->text(), Uml::A);
                        break;

                    case Uml::tr_RoleBName:
                        setRoleName(ft->text(), Uml::B);
                        break;

                    default:
                        uDebug() << "unexpected FloatingTextWidget (textrole " << role << ")";
                        //TODO: Investigate this delete. Firstly, is this reachable ?
#if 0
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

bool AssociationWidget::loadFromXMI(QDomElement& element)
{
    UMLScene *scene = umlScene();
    if (!scene) {
        uDebug() << "This isn't on UMLScene yet, so can neither fetch"
            "messages nor widgets on umlscene";
    }
    const UMLWidgetList& widgetList = scene->getWidgetList();
    const MessageWidgetList& messageList = scene->getMessageList();
    return loadFromXMI(element, widgetList, &messageList);
}

void AssociationWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    QDomElement assocElement = qDoc.createElement("assocwidget");

    WidgetBase::saveToXMI(qDoc, assocElement);
    if (umlObject()) {
        assocElement.setAttribute("xmi.id", ID2STR(umlObject()->getID()));
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
    //TODO: Port
#if 0
    assocElement.setAttribute("indexa", m_role[A].m_nIndex);
    assocElement.setAttribute("indexb", m_role[B].m_nIndex);
    assocElement.setAttribute("totalcounta", m_role[A].m_nTotalCount);
    assocElement.setAttribute("totalcountb", m_role[B].m_nTotalCount);
#endif
    m_associationLine->saveToXMI(qDoc, assocElement);

    if(m_nameWidget->hasValidText()) {
        m_nameWidget->saveToXMI(qDoc, assocElement);
    }

    if(multiplicityWidget(Uml::A)->hasValidText()) {
        multiplicityWidget(Uml::A)->saveToXMI(qDoc, assocElement);
    }

    if(multiplicityWidget(Uml::B)->hasValidText()) {
        multiplicityWidget(Uml::B)->saveToXMI(qDoc, assocElement);
    }

    if(roleWidget(Uml::A)->hasValidText()) {
        roleWidget(Uml::A)->saveToXMI(qDoc, assocElement);
    }

    if(roleWidget(Uml::B)->hasValidText()) {
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
        //TODO: port
#if 0
        assocElement.setAttribute("aclsegindex", m_nLinePathSegmentIndex);
#endif
    }

    qElement.appendChild(assocElement);
}

void AssociationWidget::slotMenuSelection(QAction *action)
{
    QString oldText, newText;
    QFont font;
    QRegExpValidator v(QRegExp(".*"), 0);
    Uml::Association_Type atype = associationType();
    Uml::Role_Type r = Uml::B;
    UMLScene *scene = umlScene();
    UMLView *view = scene ? scene->activeView() : 0;

    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);

    //if it's a collaboration message we now just use the code in floatingtextwidget
    //this means there's some redundant code below but that's better than duplicated code
    if (isCollaboration() && sel != ListPopupMenu::mt_Delete) {
        //TODO: Verify the working of following line of code.
        m_nameWidget->slotMenuSelection(action);
        return;
    }

    switch(sel) {
        case ListPopupMenu::mt_Properties:
            if(atype == Uml::at_Seq_Message || atype == Uml::at_Seq_Message_Self) {
                // show op dlg for seq. diagram here
                // don't worry about here, I don't think it can get here as
                // line is widget on seq. diagram
                // here just in case - remove later after testing
                uDebug() << "mt_Properties: assoctype is " << atype;
            } else {  //standard assoc dialog
                UMLScene *scene = umlScene();
                if (scene) {
                    scene->updateDocumentation( false );
                }
                showPropertiesDialog();
            }
            break;

        case ListPopupMenu::mt_Delete:
            //TODO:
#if 0
            if (m_pAssocClassLineSel0)
                removeAssocClassLine();
            else if (getAssociation())
                m_pView->removeAssocInViewAndDoc(this);
            else
                m_pView->removeAssoc(this);
#endif
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
            if (KFontDialog::getFont(font, false, view)) {
                lwSetFont(font);
            }
            break;

        case ListPopupMenu::mt_Change_Font_Selection:
            font = this->font();
            if (KFontDialog::getFont(font, false, view)) {
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

        case ListPopupMenu::mt_Cut:
            // TODO:
#if 0
            m_pView->setStartedCut();
            UMLApp::app()->slotEditCut();
#endif
            break;

        case ListPopupMenu::mt_Copy:
            // TODO:
#if 0
            UMLApp::app()->slotEditCopy();
#endif
            break;

        case ListPopupMenu::mt_Paste:
            // TODO:
#if 0
            UMLApp::app()->slotEditPaste();
#endif
            break;

        case ListPopupMenu::mt_Reset_Label_Positions:
            resetTextPositions();
            break;

        default:
            uDebug() << "Menu_Type " << sel << " not implemented";
    }//end switch
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
    const Uml::Object_Type ot = obj->getBaseType();
    if (ot == Uml::ot_Operation) {
        if (m_nameWidget) {
            m_nameWidget->setMessageText();
        }
    } else if (ot == Uml::ot_Attribute) {
        UMLAttribute *attr = static_cast<UMLAttribute*>(obj);
        setVisibility(attr->getVisibility(), Uml::B);
        setRoleName(attr->getName(), Uml::B);
    } else if (ot == Uml::ot_Association) {
        WidgetRole &a = m_widgetRole[Uml::A];
        WidgetRole &b = m_widgetRole[Uml::B];

        a.multiplicityWidget->setText(multiplicity(Uml::A));
        b.multiplicityWidget->setText(multiplicity(Uml::B));

        a.roleWidget->setPreText(visibility(Uml::A).toString(true));
        b.roleWidget->setPreText(visibility(Uml::B).toString(true));

        a.roleWidget->setText(roleName(Uml::A));
        b.roleWidget->setText(roleName(Uml::B));

        if (!m_slotUMLObjectDataChangedFirstCall) {
            a.changeabilityWidget->setText(UMLAssociation::toString(changeability(Uml::A)));
            b.changeabilityWidget->setText(UMLAssociation::toString(changeability(Uml::B)));
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
            m_widgetRole[role].multiplicityWidget->setFont(newFont);
            m_widgetRole[role].changeabilityWidget->setFont(newFont);
            m_widgetRole[role].roleWidget->setFont(newFont);
        }
        m_nameWidget->setFont(newFont);
    } else if (change == NameHasChanged) {
        m_nameWidget->setText(name());
        updateNameWidgetRole();
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

void AssociationWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();

    Uml::Association_Type type = associationType();
    ListPopupMenu::Menu_Type menuType = ListPopupMenu::mt_Undefined;
    if (type == Uml::at_Anchor ||
            m_associationLine->onAssociationClassLine(event->pos())) {
        menuType = ListPopupMenu::mt_Anchor;
    } else if (isCollaboration()) {
        menuType = ListPopupMenu::mt_Collaboration_Message;
    } else if (!association()) {
        menuType = ListPopupMenu::mt_AttributeAssociation;
    } else if (AssocRules::allowRole(type)) {
        menuType = ListPopupMenu::mt_FullAssociation;
    } else {
        menuType = ListPopupMenu::mt_Association_Selected;
    }

    UMLScene *scene = umlScene();
    QWidget *parent = 0;
    if (scene) {
        parent = scene->activeView();
    }

    if (!isSelected() && scene && !scene->selectedItems().isEmpty()) {
        Qt::KeyboardModifiers forSelection = (Qt::ControlModifier | Qt::ShiftModifier);
        if ((event->modifiers() & forSelection) == 0) {
            scene->clearSelection();
        }
    }
    setSelected(true);
    QPointer<ListPopupMenu> menu = new ListPopupMenu(parent, menuType, this);
    QAction *triggered = menu->exec(event->screenPos());
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

void AssociationWidget::sceneSetFirstTime()
{
    if (m_setCollabIDOnFirstSceneSet) {
        Q_ASSERT(isCollaboration());
        int collabID = umlScene()->generateCollaborationId();
        setName('m' + QString::number(collabID));
        m_setCollabIDOnFirstSceneSet = false; //reset flag
        setActivatedFlag(true); // Now activate it.
    }
}

/**
 * Reimplemented to cleanup connectivity with the old UMLObject.
 */ 
void AssociationWidget::umlObjectChanged(UMLObject *old)
{
    if (!old) {
        return;
    }

    const Uml::Object_Type ot = old->getBaseType();
    if (ot == Uml::ot_Association) {

        UMLAssociation *oldAssoc = static_cast<UMLAssociation*>(old);
        oldAssoc->nrof_parent_widgets--;
        // TODO: Discussion on ownership of UMLAssociation
        if (oldAssoc->nrof_parent_widgets == 0) {
            // TODO: Delete oldAssoc or not ? Depends on cut/copy implementation
        }

    } else if (ot == Uml::ot_Attribute) {

        UMLClassifier *klass = static_cast<UMLClassifier*>(old->parent());
        disconnect(klass, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));

    } else if (ot == Uml::ot_EntityAttribute) {

        UMLEntity *ent = static_cast<UMLEntity*>(old->parent());
        disconnect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));

    } else if (ot == Uml::ot_ForeignKeyConstraint) {

        UMLEntity *ent = static_cast<UMLEntity*>(old->parent());
        disconnect(ent, SIGNAL(entityConstraintRemoved(UMLClassifierListItem*)),
                this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));

    } else {
        uError() << "Had a wrong association of type " << ot;
    }

    WidgetBase::umlObjectChanged(old);
}

void AssociationWidget::init()
{
    m_baseType = Uml::wt_Association;
    m_associationType = Uml::at_Association;
    m_associationClass = 0;
    m_associationLine = new AssociationLine(this);
    m_setCollabIDOnFirstSceneSet = false;
    m_slotUMLObjectDataChangedFirstCall = true;

    m_nameWidget = new FloatingTextWidget(Uml::tr_Name);
    m_nameWidget->setLink(this);
    m_nameSegmentIndex = -1;

    m_widgetRole[Uml::A].initFloatingWidgets(Uml::A, this);
    m_widgetRole[Uml::B].initFloatingWidgets(Uml::B, this);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
}

void AssociationWidget::setFloatingText(Uml::Text_Role tr, const QString& text, FloatingTextWidget* &ft)
{
    if (! FloatingTextWidget::isTextValid(text)) {
        // FloatingTextWidgets are no longer deleted/reconstructed to make it easier.
        // Set an empty text, so that floatingtext is implicitly hidden.
        ft->setText("");
        return;
    }

    bool wasHidden = !(ft->isVisible());
    ft->setText(text);
    if (wasHidden) {
        setTextPosition(tr);
    }
}

QPointF AssociationWidget::calculateTextPosition(Uml::Text_Role role)
{
    const qreal SPACE = 2;
    QPointF p(-1, -1), q(-1, -1);

    // used to find out if association end point (p)
    // is at top or bottom edge of widget.
    UMLWidget *pWidget = 0;

    if (role == Uml::tr_MultiA || role == Uml::tr_ChangeA || role == Uml::tr_RoleAName) {
        p = m_associationLine->point( 0 );
        q = m_associationLine->point( 1 );
        pWidget = widgetForRole(Uml::A);
    } else if (role == Uml::tr_MultiB || role == Uml::tr_ChangeB || role == Uml::tr_RoleBName) {
        const uint lastSegment = m_associationLine->count() - 1;
        p = m_associationLine->point(lastSegment);
        q = m_associationLine->point(lastSegment - 1);
        pWidget = widgetForRole(Uml::B);
    } else if (role != Uml::tr_Name) {
        uError() << "called with unsupported Text_Role " << role;
        return QPoint(-1, -1);
    }

    FloatingTextWidget *text = textWidgetByRole(role);
    qreal textW = 0, textH = 0;
    if (text) {
        textW = text->width();
        textH = text->height();
    }

    qreal x = 0, y = 0;

    if (role == Uml::tr_MultiA || role == Uml::tr_MultiB) {
        const bool isHorizontal = (p.y() == q.y());
        const qreal atBottom = p.y() + SPACE;
        const qreal atTop = p.y() - SPACE - textH;
        const qreal atLeft = p.x() - SPACE - textW;
        const qreal atRight = p.x() + SPACE;
        y = (p.y() > q.y()) == isHorizontal ? atBottom : atTop;
        x = (p.x() < q.x()) == isHorizontal ? atRight : atLeft;

    } else if (role == Uml::tr_ChangeA || role == Uml::tr_ChangeB) {

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

    } else if (role == Uml::tr_RoleAName || role == Uml::tr_RoleBName) {

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

    } else if (role == Uml::tr_Name) {

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
    return p;
}

void AssociationWidget::setTextPosition(Uml::Text_Role tr)
{
    bool startMove = false;
    //TODO: Check startMove removal
    if (startMove) {
        return;
    }
    FloatingTextWidget *ft = textWidgetByRole(tr);
    if (!ft) {
        return;
    }
    QPointF pos = calculateTextPosition(tr);
    if ( (pos.x() < 0.0 || pos.x() > FloatingTextWidget::restrictPositionMax) ||
            (pos.y() < 0 || pos.y() > FloatingTextWidget::restrictPositionMax) ) {
        uDebug() << "(x=" << pos.x() << " , y=" << pos.y() << ") "
            << "- was blocked because at least one value is out of bounds: ["
            << "0 ... " << FloatingTextWidget::restrictPositionMax << "]";
        return;
    }
    ft->setPos(pos);
}

void AssociationWidget::updateNameWidgetRole()
{
    Uml::Text_Role textRole = Uml::tr_Name;
    UMLScene *scene = umlScene();
    if (scene) {
        if (scene->getType() == Uml::dt_Collaboration) {
            if (isSelf()) {
                textRole = Uml::tr_Coll_Message;
            } else {
                textRole = Uml::tr_Coll_Message;
            }
        } else if (scene->getType() == Uml::dt_Sequence) {
            if (isSelf()) {
                textRole = Uml::tr_Seq_Message_Self;
            } else {
                textRole = Uml::tr_Seq_Message;
            }
        }
    }

    m_nameWidget->setTextRole(textRole);
}

#include "associationwidget.moc"
