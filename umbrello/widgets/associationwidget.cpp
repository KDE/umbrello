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

#include <QPointer>

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
    m_associationLine->setupSymbols();


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


bool AssociationWidget::showDialog()
{
    bool success = false;
    UMLView *view = umlScene() ? umlScene()->activeView() : 0;

    QPointer<AssocPropDlg> dlg = new AssocPropDlg(view, this );
    if (dlg->exec()) {
        success = true;
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
    return success;
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


void AssociationWidget::constrainTextPos(qreal &textX, qreal &textY, qreal textWidth,
        qreal textHeight, Uml::Text_Role tr)
{
    const QPointF textCenter = QRectF(textX, textY, textWidth, textHeight).center();
    const uint numSegments = m_associationLine->count()-1;
    const uint lastSegmentIndex = numSegments-1;

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
    const QPointF textAbs(textCenterX, textCenterY);
    const QPointF textRel = textAbs - mid;

    QPointF projected = textRel;
    QLineF line(QPointF(0, 0), textRel);
    if (line.length() > radius) {
        line.setLength(radius);
        projected = line.p2();
    }
    projected += mid;

    textX = projected.x() - .5 * textWidth;
    textY = projected.y() - .5 * textHeight;
}


void AssociationWidget::calculateNameTextSegment()
{
    //TODO: Implement this
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
    if (c == changeability(role)) {
        return;
    }
    QString changeString = UMLAssociation::toString(c);
    if (association()) {
        association()->setChangeability(c, role);
    }
    m_widgetRole[role].changeability = c;
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
    if (m_widgetRole[role].roleWidget) {
        return m_widgetRole[role].roleWidget->text();
    }
    return QString();
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

void AssociationWidget::activate()
{
    Q_ASSERT(umlScene());
    setActivatedFlag(false);
    if (!umlObject() &&
            UMLAssociation::assocTypeHasUMLRepresentation(associationType())) {
        UMLObject *myObj = umlDoc()->findObjectById(id());
        if (!myObj) {
            uError() << "cannot find UMLObject " << ID2STR(id());
            return;
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
        return;
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
        // TODO: Check for removal of comment
        // calculateNameTextSegment();
    }

    // Prepare the association class line if needed.
    m_associationLine->calculateAssociationClassLine();

    setActivatedFlag(true);
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

bool AssociationWidget::loadFromXMI(QDomElement& element)
{
    //TODO: Port
    return false;
}

bool AssociationWidget::loadFromXMI(const QDomElement& element, UMLWidgetList &list)
{
    //TODO: Port
    return false;
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
    }
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

    m_nameWidget = new FloatingTextWidget(Uml::tr_Name);
    m_nameWidget->setLink(this);

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

void AssociationWidget::setTextPosition(Uml::Text_Role tr)
{
    Q_UNUSED(tr);
    // TODO: Implement this stub
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
