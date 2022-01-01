/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "associationwidget.h"

// app includes
#include "association.h"
#include "associationpropertiesdialog.h"
#include "associationwidgetpopupmenu.h"
#include "assocrules.h"
#include "attribute.h"
#include "classifier.h"
#include "classifierwidget.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "docwindow.h"
#include "entity.h"
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "operation.h"
#include "optionstate.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"
#include "widget_utils.h"
#include "instance.h"
#include "instanceattribute.h"

// kde includes
#if QT_VERSION < 0x050000
#include <kcolordialog.h>
#include <kfontdialog.h>
#endif
#include <KLocalizedString>

// qt includes
#if QT_VERSION >= 0x050000
#include <QColorDialog>
#include <QFontDialog>
#endif
#include <QPainterPath>
#include <QPointer>
#include <QApplication>
#include <QXmlStreamWriter>

// system includes
#include <cmath>

#define DBG_AW() DEBUG(QLatin1String("AssociationWidget"))
DEBUG_REGISTER_DISABLED(AssociationWidget)

using namespace Uml;

/**
  * Constructor is private because the static create() methods shall
  * be used for constructing AssociationWidgets.
  *
  * @param scene   The parent view of this widget.
  */
AssociationWidget::AssociationWidget(UMLScene *scene)
  : WidgetBase(scene, WidgetBase::wt_Association),
    m_positions_len(0),
    m_activated(false),
    m_unNameLineSegment(-1),
    m_nLinePathSegmentIndex(-1),
    m_pAssocClassLine(0),
    m_pAssocClassLineSel0(0),
    m_pAssocClassLineSel1(0),
    m_associationLine(this),
    m_associationClass(0),
    m_associationType(Uml::AssociationType::Association),
    m_nameWidget(0)
{
    m_role[0].setParent(this);
    m_role[1].setParent(this);
    // propagate line color and width set by base class constructor
    // which does not call the virtual methods from this class.
    setLineColor(lineColor());
    setLineWidth(lineWidth());

    setFlag(QGraphicsLineItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

/**
 * This constructor is really only for loading from XMI, otherwise it
 * should not be allowed as it creates an incomplete associationwidget.
  *
  * @param scene   The parent view of this widget.
 */
AssociationWidget* AssociationWidget::create(UMLScene *scene)
{
    AssociationWidget* instance = new AssociationWidget(scene);
    return instance;
}

/**
  * Preferred constructor (static factory method.)
  *
  * @param scene      The parent view of this widget.
  * @param pWidgetA   Pointer to the role A widget for the association.
  * @param assocType  The AssociationType::Enum for this association.
  * @param pWidgetB   Pointer to the role B widget for the association.
  * @param umlobject  Pointer to the underlying UMLObject (if applicable.)
  */
AssociationWidget* AssociationWidget::create
                                    (UMLScene *scene, UMLWidget* pWidgetA,
                                     Uml::AssociationType::Enum assocType, UMLWidget* pWidgetB,
                                     UMLObject *umlobject /* = 0 */)
{
    AssociationWidget* instance = new AssociationWidget(scene);
    if (umlobject) {
        instance->setUMLObject(umlobject);
    } else {
        // set up UMLAssociation obj if assoc is represented and both roles are UML objects
        if (Uml::AssociationType::hasUMLRepresentation(assocType)) {
            UMLObject* umlRoleA = pWidgetA->umlObject();
            UMLObject* umlRoleB = pWidgetB->umlObject();
            if (umlRoleA != 0 && umlRoleB != 0) {
                bool swap;
                // Check that we are not attempting to create the same Generalization /
                // Dependency / Association_Self / Coll_Mesg_Self / Seq_Message_Self /
                // Containment / Realization between the same two objects when such an
                // association already exists.
                UMLDoc *doc = UMLApp::app()->document();
                UMLAssociation *myAssoc = doc->findAssociation(assocType, umlRoleA, umlRoleB, &swap);
                if (myAssoc != 0) {
                    switch (assocType) {
                        case Uml::AssociationType::Generalization:
                        case Uml::AssociationType::Dependency:
                        case Uml::AssociationType::Association_Self:
                        case Uml::AssociationType::Coll_Mesg_Self:
                        case Uml::AssociationType::Seq_Message_Self:
                        case Uml::AssociationType::Containment:
                        case Uml::AssociationType::Realization:
                            DBG_AW() << "Ignoring second construction of same assoctype "
                                     << assocType << " between " << umlRoleA->name()
                                     << " and " << umlRoleB->name() << " (swap=" << swap << ")";
                            break;
                        default:
                            DBG_AW() << "constructing a similar or exact same assoctype "
                                     << assocType << " between " << umlRoleA->name() << " and "
                                     << umlRoleB->name() << "as an already existing assoc (swap="
                                     << swap << ")";
                            // now, just create a new association anyways
                            myAssoc = 0;
                            break;
                    }
                }
                if (myAssoc == 0) {
                    myAssoc = new UMLAssociation(assocType, umlRoleA, umlRoleB);
                    // CHECK: myAssoc is not yet inserted at any parent UMLPackage -
                    // need to check carefully that all callers do this, lest it be
                    // orphaned.
                    // ToolBarStateAssociation::addAssociationInViewAndDoc() is
                    // okay in this regard.
                }
                instance->setUMLAssociation(myAssoc);
            }
        }
    }

    instance->setWidgetForRole(pWidgetA, RoleType::A);
    instance->setWidgetForRole(pWidgetB, RoleType::B);

    instance->setAssociationType(assocType);

    instance->calculateEndingPoints();

    instance->associationLine().calculateInitialEndPoints();
    instance->associationLine().reconstructSymbols();

    //The AssociationWidget is set to Activated because it already has its side widgets
    instance->setActivated(true);

    // sync UML meta-data to settings here
    instance->mergeAssociationDataIntoUMLRepresentation();

    // Collaboration messages need a name label because it's that
    // which lets operator== distinguish them, which in turn
    // permits us to have more than one message between two objects.
    if (instance->isCollaboration()) {
        // Create a temporary name to bring on setName()
        int collabID = instance->m_scene->generateCollaborationId();
        instance->setName(QLatin1Char('m') + QString::number(collabID));
    }

    return instance;
}

/**
 * Destructor.
 */
AssociationWidget::~AssociationWidget()
{
}

/**
 * Overriding the method from WidgetBase because we need to do
 * something extra in case this AssociationWidget represents
 * an attribute of a classifier.
 */
void AssociationWidget::setUMLObject(UMLObject *obj)
{
    WidgetBase::setUMLObject(obj);
    if (obj == 0)
        return;
    UMLClassifier *klass = 0;
    UMLAttribute *attr = 0;
    UMLEntity *ent = 0;
    const UMLObject::ObjectType ot = obj->baseType();
    switch (ot) {
        case UMLObject::ot_Association:
            setUMLAssociation(obj->asUMLAssociation());
            break;
        case UMLObject::ot_Operation:
            setOperation(obj->asUMLOperation());
            break;
        case UMLObject::ot_Attribute:
            klass = obj->umlParent()->asUMLClassifier();
            connect(klass, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                    this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
            attr = obj->asUMLAttribute();
            connect(attr, SIGNAL(attributeChanged()), this, SLOT(slotAttributeChanged()));
            break;
        case UMLObject::ot_EntityAttribute:
            ent = obj->umlParent()->asUMLEntity();
            connect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                    this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
            break;
        case UMLObject::ot_ForeignKeyConstraint:
            ent = obj->umlParent()->asUMLEntity();
            connect(ent, SIGNAL(entityConstraintRemoved(UMLClassifierListItem*)),
                    this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
            break;
        default:
            uError() << "cannot associate UMLObject of type " << UMLObject::toString(ot);
            break;
    }
}

/**
 * Set all 'owned' child widgets to this font.
 */
void AssociationWidget::lwSetFont (QFont font)
{
    if (m_nameWidget) {
        m_nameWidget->setFont(font);
    }
    m_role[RoleType::A].setFont(font);
    m_role[RoleType::B].setFont(font);
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 * @todo Move to LinkWidget.
 */
UMLClassifier *AssociationWidget::operationOwner()
{
    Uml::RoleType::Enum role = (isCollaboration() ? Uml::RoleType::B : Uml::RoleType::A);
    UMLObject *o = widgetForRole(role)->umlObject();
    if (!o) {
        return 0;
    }
    UMLClassifier *c = o->asUMLClassifier();
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
    return m_umlObject->asUMLOperation();
}

/**
 * Implements operation from LinkWidget.
 * Motivated by FloatingTextWidget.
 */
void AssociationWidget::setOperation(UMLOperation *op)
{
    if (m_umlObject)
        disconnect(m_umlObject, SIGNAL(modified()), m_nameWidget, SLOT(setMessageText()));
    m_umlObject = op;
    if (m_umlObject)
        connect(m_umlObject, SIGNAL(modified()), m_nameWidget, SLOT(setMessageText()));
    if (m_nameWidget)
        m_nameWidget->setMessageText();
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
    if (m_role[RoleType::A].multiplicityWidget) {
        setTextPosition(TextRole::MultiA);
    }
    if (m_role[RoleType::B].multiplicityWidget) {
        setTextPosition(Uml::TextRole::MultiB);
    }
    if (m_role[RoleType::A].changeabilityWidget) {
        setTextPosition(Uml::TextRole::ChangeA);
    }
    if (m_role[RoleType::B].changeabilityWidget) {
        setTextPosition(Uml::TextRole::ChangeB);
    }
    if (m_nameWidget) {
        setTextPosition(Uml::TextRole::Name);
    }
    if (m_role[RoleType::A].roleWidget) {
        setTextPosition(Uml::TextRole::RoleAName);
    }
    if (m_role[RoleType::B].roleWidget) {
        setTextPosition(Uml::TextRole::RoleBName);
    }
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param ft   The text widget which to update.
 */
void AssociationWidget::setMessageText(FloatingTextWidget *ft)
{
    if (isCollaboration()) {
        ft->setSequenceNumber(m_SequenceNumber);
        if (m_umlObject != 0) {
            ft->setText(operationText(m_scene));
        } else {
            ft->setText(name());
        }
    } else {
        ft->setText(name());
    }
}

/**
 * Sets the text of the given FloatingTextWidget.
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
void AssociationWidget::setText(FloatingTextWidget *ft, const QString &text)
{
    Uml::TextRole::Enum role = ft->textRole();
    switch (role) {
        case Uml::TextRole::Name:
            setName(text);
            break;
        case Uml::TextRole::RoleAName:
            setRoleName(text, RoleType::A);
            break;
        case Uml::TextRole::RoleBName:
            setRoleName(text, RoleType::B);
            break;
        case Uml::TextRole::MultiA:
            setMultiplicity(text, RoleType::A);
            break;
        case Uml::TextRole::MultiB:
            setMultiplicity(text, RoleType::B);
            break;
        default:
            uWarning() << "Unhandled TextRole: " << Uml::TextRole::toString(role);
            break;
    }
}

/**
 * Shows the association properties dialog and updates the
 * corresponding texts if its execution is successful.
 */
bool AssociationWidget::showPropertiesDialog()
{
    bool result = false;
    UMLApp::app()->docWindow()->updateDocumentation();
    QPointer<AssociationPropertiesDialog> dlg = new AssociationPropertiesDialog(static_cast<QWidget*>(m_scene->activeView()), this);
    if (dlg->exec()) {
        UMLApp::app()->docWindow()->showDocumentation(this, true);
        result = true;
    }
    delete dlg;
    return result;
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
QString AssociationWidget::lwOperationText()
{
    return name();
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @return classifier
 */
UMLClassifier* AssociationWidget::lwClassifier()
{
    UMLObject *o = widgetForRole(RoleType::B)->umlObject();
    UMLClassifier *c = o->asUMLClassifier();
    return c;
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param op       The new operation string to set.
 */
void AssociationWidget::setOperationText(const QString &op)
{
    if (!op.isEmpty()) {
        setName(op);
    }
}

/**
 * Calculates the m_unNameLineSegment index according to m_nameWidget
 * middle point PT.
 * It iterates through all AssociationLine's segments and for each one
 * calculates the sum of PT's distance to the start point + PT's
 * distance to the end point. The segment with the smallest sum will
 * be the RoleTextSegment (if this segment moves then the RoleText
 * will move with it). It sets m_unNameLineSegment to the start point
 * of the chosen segment.
 *
 * Overrides operation from LinkWidget (i.e. this method is also
 * required by FloatingTextWidget.)
 */
void AssociationWidget::calculateNameTextSegment()
{
    if (!m_nameWidget) {
        return;
    }
    //changed to use the middle of the text
    //i think this will give a better result.
    //never know what sort of lines people come up with
    //and text could be long to give a false reading
    qreal xt = m_nameWidget->x();
    qreal yt = m_nameWidget->y();
    xt += m_nameWidget->width() / 2;
    yt += m_nameWidget->height() / 2;
    int size = m_associationLine.count();
    //sum of length(PTP1) and length(PTP2)
    qreal total_length = 0;
    qreal smallest_length = 0;
    for (int i = 0; i < size - 1; ++i) {
        QPointF pi = m_associationLine.point( i );
        QPointF pj = m_associationLine.point( i+1 );
        qreal xtiDiff = xt - pi.x();
        qreal xtjDiff = xt - pj.x();
        qreal ytiDiff = yt - pi.y();
        qreal ytjDiff = yt - pj.y();
        total_length =  sqrt( double(xtiDiff * xtiDiff + ytiDiff * ytiDiff) )
                        + sqrt( double(xtjDiff * xtjDiff + ytjDiff * ytjDiff) );
        //this gives the closest point
        if (total_length < smallest_length || i == 0) {
            smallest_length = total_length;
            m_unNameLineSegment = i;
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
    if (m_umlObject == 0 || m_umlObject->baseType() != UMLObject::ot_Association)
        return 0;
    return m_umlObject->asUMLAssociation();
}

/**
 * Returns the UMLAttribute representation of this object.
 *
 * @return  Pointer to the UMLAttribute that is represented by
 *          this AsociationWidget.
 */
UMLAttribute* AssociationWidget::attribute() const
{
    if (m_umlObject == 0)
        return 0;
    UMLObject::ObjectType ot = m_umlObject->baseType();
    if (ot != UMLObject::ot_Attribute && ot != UMLObject::ot_EntityAttribute && ot != UMLObject::ot_InstanceAttribute)
        return 0;
    return m_umlObject->asUMLAttribute();
}

#if 0  //:TODO:
/**
 * Overrides the assignment operator.
 */
AssociationWidget& AssociationWidget::operator=(const AssociationWidget& other)
{
    m_associationLine = other.m_associationLine;

    if (other.m_nameWidget) {
        m_nameWidget = new FloatingTextWidget(m_scene);
        *m_nameWidget = *(other.m_nameWidget);
    } else {
        m_nameWidget = 0;
    }

    for (unsigned r = (unsigned)A; r <= (unsigned)B; ++r) {
        WidgetRole& lhs = m_role[r];
        const WidgetRole& rhs = other.m_role[r];
        lhs.m_nIndex = rhs.m_nIndex;
        lhs.m_nTotalCount = rhs.m_nTotalCount;

        if (rhs.multiplicityWidget) {
            lhs.multiplicityWidget = new FloatingTextWidget(m_scene);
            *(lhs.multiplicityWidget) = *(rhs.multiplicityWidget);
        } else {
            lhs.multiplicityWidget = 0;
        }

        if (rhs.roleWidget) {
            lhs.roleWidget = new FloatingTextWidget(m_scene);
            *(lhs.roleWidget) = *(rhs.roleWidget);
        } else {
            lhs.roleWidget = 0;
        }

        if (rhs.changeabilityWidget) {
            lhs.changeabilityWidget = new FloatingTextWidget(m_scene);
            *(lhs.changeabilityWidget) = *(rhs.changeabilityWidget);
        } else {
            lhs.changeabilityWidget = 0;
        }

        lhs.umlWidget = rhs.umlWidget;
        lhs.m_WidgetRegion = rhs.m_WidgetRegion;
    }

    m_activated = other.m_activated;
    m_unNameLineSegment = other.m_unNameLineSegment;
    setUMLAssociation(other.association());
    setSelected(other.isSelected());

    return *this;
}
#endif  //:TODO:

/**
 * Overrides the equality test operator.
 */
bool AssociationWidget::operator==(const AssociationWidget& other) const
{
    if (this == &other)
        return true;

    // if no model representation exists, then the widgets are not equal
    if (association() == 0 && other.association() == 0)
        return false;

    if (!m_umlObject || !other.m_umlObject ) {
        if (!other.m_umlObject && m_umlObject)
            return false;
        if (other.m_umlObject && !m_umlObject)
            return false;

    } else if (m_umlObject != other.m_umlObject)
        return false;

    if (associationType() != other.associationType())
        return false;

    if (widgetIDForRole(RoleType::A) != other.widgetIDForRole(RoleType::A))
        return false;

    if (widgetIDForRole(RoleType::B) != other.widgetIDForRole(RoleType::B))
        return false;

    if (widgetForRole(RoleType::A)->isObjectWidget() &&
            other.widgetForRole(RoleType::A)->isObjectWidget()) {
        if (widgetForRole(RoleType::A)->localID() != other.widgetForRole(RoleType::A)->localID())
            return false;
    }

    if (widgetForRole(RoleType::B)->isObjectWidget() &&
            other.widgetForRole(RoleType::B)->isObjectWidget()) {
        if (widgetForRole(RoleType::B)->localID() != other.widgetForRole(RoleType::B)->localID())
            return false;
    }

    // Two objects in a collaboration can have multiple messages between each other.
    // Here we depend on the messages having names, and the names must be different.
    // That is the reason why collaboration messages have strange initial names like
    // "m29997" or similar.
    return (name() == other.name());
}

/**
 * Overrides the != operator.
 */
bool AssociationWidget::operator!=(AssociationWidget& other) const
{
    return !(*this == other);
}

/**
 * Returns a const reference to the association widget's line path.
 */
const AssociationLine& AssociationWidget::associationLine() const
{
    return m_associationLine;
}

/**
 * Returns a writable reference to the association widget's line path.
 */
AssociationLine& AssociationWidget::associationLine()
{
    return m_associationLine;
}

/**
 * Activates the AssociationWidget after a load.
 *
 * @return  true for success
 */
bool AssociationWidget::activate(IDChangeLog *changeLog)
{
    Q_UNUSED(changeLog);

    if (m_umlObject == 0 &&
        AssociationType::hasUMLRepresentation(m_associationType)) {
        UMLObject *myObj = umlDoc()->findObjectById(m_nId);
        if (myObj == 0) {
            uError() << "cannot find UMLObject " << Uml::ID::toString(m_nId);
            return false;
        } else {
            const UMLObject::ObjectType ot = myObj->baseType();
            if (ot == UMLObject::ot_Association) {
                UMLAssociation * myAssoc = myObj->asUMLAssociation();
                setUMLAssociation(myAssoc);
            } else {
                setUMLObject(myObj);
                setAssociationType(m_associationType);
            }
        }
    }

    if (m_activated)
        return true;

    Uml::AssociationType::Enum type = associationType();

    if (m_role[RoleType::A].umlWidget == 0)
        setWidgetForRole(m_scene->findWidget(widgetIDForRole(RoleType::A)), RoleType::A);
    if (m_role[RoleType::B].umlWidget == 0)
        setWidgetForRole(m_scene->findWidget(widgetIDForRole(RoleType::B)), RoleType::B);

    if (!m_role[RoleType::A].umlWidget || !m_role[RoleType::B].umlWidget) {
        DEBUG(DBG_SRC) << "Cannot make association!";
        return false;
    }

    calculateEndingPoints();

    if (AssocRules::allowRole(type)) {
        for (unsigned r = RoleType::A; r <= RoleType::B; ++r) {
            AssociationWidgetRole& robj = m_role[r];
            if (robj.roleWidget == 0)
                continue;
            robj.roleWidget->setLink(this);
            TextRole::Enum tr = (r == RoleType::A ? TextRole::RoleAName : TextRole::RoleBName);
            robj.roleWidget->setTextRole(tr);
            Uml::Visibility::Enum vis = visibility(Uml::RoleType::fromInt(r));
            robj.roleWidget->setPreText(Uml::Visibility::toString(vis, true));

            if (FloatingTextWidget::isTextValid(robj.roleWidget->text()))
                robj.roleWidget->show();
            else
                robj.roleWidget->hide();
            if (m_scene->type() == DiagramType::Collaboration)
                robj.roleWidget->setUMLObject(robj.umlWidget->umlObject());
            robj.roleWidget->activate();
        }
    }

    if (m_nameWidget != 0) {
        m_nameWidget->setLink(this);
        m_nameWidget->setTextRole(calculateNameType(TextRole::Name));

        if (FloatingTextWidget::isTextValid(m_nameWidget->text())) {
            m_nameWidget->show();
        } else {
            m_nameWidget->hide();
        }
        m_nameWidget->activate();
        calculateNameTextSegment();
    }

    for (unsigned r = RoleType::A; r <= RoleType::B; ++r) {
        AssociationWidgetRole& robj = m_role[r];

        FloatingTextWidget* pMulti = robj.multiplicityWidget;
        if (pMulti != 0 &&
                AssocRules::allowMultiplicity(type, robj.umlWidget->baseType())) {
            pMulti->setLink(this);
            TextRole::Enum tr = (r == RoleType::A ? TextRole::MultiA : TextRole::MultiB);
            pMulti->setTextRole(tr);
            if (FloatingTextWidget::isTextValid(pMulti->text()))
                pMulti->show();
            else
                pMulti->hide();
            pMulti->activate();
        }

        FloatingTextWidget* pChangeWidget = robj.changeabilityWidget;
        if (pChangeWidget != 0) {
            pChangeWidget->setLink(this);
            TextRole::Enum tr = (r == RoleType::A ? TextRole::ChangeA : TextRole::ChangeB);
            pChangeWidget->setTextRole(tr);
            if (FloatingTextWidget::isTextValid(pChangeWidget->text()))
                pChangeWidget->show();
            else
                pChangeWidget->hide ();
            pChangeWidget->activate();
        }
    }

    // Prepare the association class line if needed.
    if (m_associationClass && !m_pAssocClassLine) {
        createAssocClassLine();
    }

    m_activated = true;
    return true;
}

/**
 * Set the widget of the given role.
 * Add this AssociationWidget at the widget.
 * If this AssociationWidget has an underlying UMLAssociation then set
 * the widget's underlying UMLObject at the UMLAssociation's role object.
 *
 * @param widget    Pointer to the UMLWidget.
 * @param role      Role for which to set the widget.
 */
void AssociationWidget::setWidgetForRole(UMLWidget* widget, Uml::RoleType::Enum role)
{
    m_role[role].umlWidget = widget;
    if (widget) {
        m_role[role].umlWidget->addAssoc(this);
        if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
            association()->setObject(widget->umlObject(), role);
    }
}

/**
 * Return the multiplicity FloatingTextWidget widget of the given role.
 *
 * @return  Pointer to the multiplicity FloatingTextWidget object.
 */
FloatingTextWidget* AssociationWidget::multiplicityWidget(Uml::RoleType::Enum role) const
{
    return m_role[role].multiplicityWidget;
}

/**
 * Read property of FloatingTextWidget* m_nameWidget.
 *
 * @return  Pointer to the FloatingTextWidget name widget.
 */
FloatingTextWidget* AssociationWidget::nameWidget() const
{
    return m_nameWidget;
}

/**
 * Return the given role's FloatingTextWidget object.
 *
 * @return  Pointer to the role's FloatingTextWidget widget.
 */
FloatingTextWidget* AssociationWidget::roleWidget(Uml::RoleType::Enum role) const
{
    return m_role[role].roleWidget;
}

/**
 * Return the given role's changeability FloatingTextWidget widget.
 */
FloatingTextWidget* AssociationWidget::changeabilityWidget(Uml::RoleType::Enum role) const
{
    return m_role[role].changeabilityWidget;
}

/**
 * Return the FloatingTextWidget object indicated by the given TextRole::Enum.
 *
 * @return  Pointer to the text role's FloatingTextWidget widget.
 */
FloatingTextWidget* AssociationWidget::textWidgetByRole(Uml::TextRole::Enum tr) const
{
    switch (tr) {
        case Uml::TextRole::MultiA:
            return m_role[RoleType::A].multiplicityWidget;
        case Uml::TextRole::MultiB:
            return m_role[RoleType::B].multiplicityWidget;
        case Uml::TextRole::Name:
        case Uml::TextRole::Coll_Message:
            return m_nameWidget;
        case Uml::TextRole::RoleAName:
            return m_role[RoleType::A].roleWidget;
        case Uml::TextRole::RoleBName:
            return m_role[RoleType::B].roleWidget;
        case Uml::TextRole::ChangeA:
            return m_role[RoleType::A].changeabilityWidget;
        case Uml::TextRole::ChangeB:
            return m_role[RoleType::B].changeabilityWidget;
        default:
            break;
    }
    return 0;
}

/**
 * Returns the m_nameWidget's text.
 *
 * @return  Text of the FloatingTextWidget name widget.
 */
QString AssociationWidget::name() const
{
    if (m_nameWidget == 0)
        return QString();
    return m_nameWidget->text();
}

/**
 * Sets the text in the FloatingTextWidget widget representing the Name
 * of this association.
 */
void AssociationWidget::setName(const QString &strName)
{
    // set attribute of UMLAssociation associated with this associationwidget
    UMLAssociation *umla = association();
    if (umla)
        umla->setName(strName);

    bool newLabel = false;
    if (!m_nameWidget) {
        // Don't construct the FloatingTextWidget if the string is empty.
        if (! FloatingTextWidget::isTextValid(strName))
            return;

        newLabel = true;
        m_nameWidget = new FloatingTextWidget(m_scene, calculateNameType(Uml::TextRole::Name), strName);
        m_nameWidget->setParentItem(this);
        m_nameWidget->setLink(this);
    } else {
        m_nameWidget->setText(strName);
        if (! FloatingTextWidget::isTextValid(strName)) {
            //m_nameWidget->hide();
            m_scene->removeWidget(m_nameWidget);
            m_nameWidget = 0;
            return;
        }
    }

    setTextPosition(Uml::TextRole::Name);
    if (newLabel) {
        m_nameWidget->setActivated();
        m_scene->addFloatingTextWidget(m_nameWidget);
    }

    m_nameWidget->show();
}

void AssociationWidget::setStereotype(const QString &stereo) {
    UMLAssociation *umlassoc = association();
    if (umlassoc) {
        umlassoc->setStereotype(stereo);
        if (!m_nameWidget) {
            QString text = umlassoc->stereotype(true);
            // Don't construct the FloatingTextWidget if the string is empty.
            if (! FloatingTextWidget::isTextValid(text))
                return;

            m_nameWidget = new FloatingTextWidget(m_scene, calculateNameType(Uml::TextRole::Name), text);
            m_nameWidget->setParentItem(this);
            m_nameWidget->setLink(this);
            m_nameWidget->activate();
            setTextPosition(calculateNameType(Uml::TextRole::Name));
        } else {
            m_nameWidget->setText(umlassoc->stereotype(true));
        }
    } else {
        uDebug() << "not setting " << stereo << " because association is NULL";
    }
}

/**
 * Return the given role's FloatingTextWidget widget text.
 *
 * @return  The name set at the FloatingTextWidget.
 */
QString AssociationWidget::roleName(Uml::RoleType::Enum role) const
{
    if (m_role[role].roleWidget == 0)
        return QString();
    return m_role[role].roleWidget->text();
}

/**
 * Sets the text to the FloatingTextWidget that display the Role text of this
 * association.
 * For this function to work properly, the associated widget
 *  should already be set.
 */
void AssociationWidget::setRoleName(const QString &strRole, Uml::RoleType::Enum role)
{
    Uml::AssociationType::Enum type = associationType();
    //if the association is not supposed to have a Role FloatingTextWidget
    if (!AssocRules::allowRole(type))  {
        return;
    }

    TextRole::Enum tr = (role == RoleType::A ? TextRole::RoleAName : TextRole::RoleBName);
    setFloatingText(tr, strRole, m_role[role].roleWidget);
    if (m_role[role].roleWidget) {
        Uml::Visibility::Enum vis = visibility(role);
        if (FloatingTextWidget::isTextValid(m_role[role].roleWidget->text())) {
            m_role[role].roleWidget->setPreText(Uml::Visibility::toString(vis, true));
            //m_role[role].roleWidget->show();
        } else {
            m_role[role].roleWidget->setPreText(QString());
            //m_role[role].roleWidget->hide();
        }
    }

    // set attribute of UMLAssociation associated with this associationwidget
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
        association()->setRoleName(strRole, role);
}

/**
 * Set the documentation on the given role.
 */
void AssociationWidget::setRoleDocumentation(const QString &doc, Uml::RoleType::Enum role)
{
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
        association()->setRoleDoc(doc, role);
    else
        m_role[role].roleDocumentation = doc;
}

/**
 * Returns the given role's documentation.
 */
QString AssociationWidget::roleDocumentation(Uml::RoleType::Enum role) const
{
    if (m_umlObject == 0 || m_umlObject->baseType() != UMLObject::ot_Association)
        return QString();
    const UMLAssociation *umla = m_umlObject->asUMLAssociation();
    return umla->getRoleDoc(role);
}

/**
 * Change, create, or delete the FloatingTextWidget indicated by the given TextRole::Enum.
 *
 * @param tr    TextRole::Enum of the FloatingTextWidget to change or create.
 * @param text  Text string that controls the action:
 *              If empty and ft is NULL then setFloatingText() is a no-op.
 *              If empty and ft is non-NULL then the existing ft is deleted.
 *              If non-empty and ft is NULL then a new FloatingTextWidget is created
 *              and returned in ft with the text set.
 *              If non-empty and ft is non-NULL then the existing ft text is modified.
 * @param ft    Reference to the pointer to FloatingTextWidget to change or create.
 *              On creation/deletion, the pointer value will be changed.
 */
void AssociationWidget::setFloatingText(Uml::TextRole::Enum role,
                                        const QString &text,
                                        FloatingTextWidget* &ft)
{
    if (! FloatingTextWidget::isTextValid(text)) {
        if (ft) {
            // Remove preexisting FloatingTextWidget
            m_scene->removeWidget(ft);  // physically deletes ft
            ft = 0;
        }
        return;
    }

    if (ft == 0) {
        ft = new FloatingTextWidget(m_scene, role, text);
        ft->setParentItem(this);
        ft->setLink(this);
        ft->activate();
        setTextPosition(role);
        m_scene->addFloatingTextWidget(ft);
    } else {
        bool newLabel = ft->text().isEmpty();
        ft->setText(text);
        if (newLabel)
            setTextPosition(role);
    }

    ft->show();
}

/**
 * Return the given role's multiplicity text.
 *
 * @return  Text of the given role's multiplicity widget.
 */
QString AssociationWidget::multiplicity(Uml::RoleType::Enum role) const
{
    if (m_role[role].multiplicityWidget == 0)
        return QString();
    return m_role[role].multiplicityWidget->text();
}

/**
 * Sets the text in the FloatingTextWidget representing the multiplicity
 * at the given side of the association.
 */
void AssociationWidget::setMultiplicity(const QString& text, Uml::RoleType::Enum role)
{
    TextRole::Enum tr = (role == RoleType::A ? TextRole::MultiA : TextRole::MultiB);

    setFloatingText(tr, text, m_role[role].multiplicityWidget);

    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
        association()->setMultiplicity(text, role);
}

/**
 * Gets the visibility on the given role of the association.
 */
Visibility::Enum AssociationWidget::visibility(Uml::RoleType::Enum role) const
{
    const UMLAssociation *assoc = association();
    if (assoc)
        return assoc->visibility(role);
    const UMLAttribute *attr = attribute();
    if (attr)
        return attr->visibility();
    return m_role[role].visibility;
}

/**
 * Sets the visibility on the given role of the association.
 */
void AssociationWidget::setVisibility(Visibility::Enum value, Uml::RoleType::Enum role)
{
    if (value != visibility(role) && m_umlObject) {
        // update our model object
        const UMLObject::ObjectType ot = m_umlObject->baseType();
        if (ot == UMLObject::ot_Association) {
            UMLAssociation *a = association();
            a->blockSignals(true);
            a->setVisibility(value, role);
            a->blockSignals(false);
        }
        else if (ot == UMLObject::ot_Attribute) {
            UMLAttribute *a = attribute();
            a->blockSignals(true);
            a->setVisibility(value);
            a->blockSignals(false);
        }
    }
    m_role[role].visibility = value;
    // update role pre-text attribute as appropriate
    if (m_role[role].roleWidget) {
        QString scopeString = Visibility::toString(value, true);
        m_role[role].roleWidget->setPreText(scopeString);
    }
}

/**
 * Gets the changeability on the given end of the Association.
 */
Uml::Changeability::Enum AssociationWidget::changeability(Uml::RoleType::Enum role) const
{
    if (m_umlObject == 0 || m_umlObject->baseType() != UMLObject::ot_Association)
        return m_role[role].changeability;
    const UMLAssociation *umla = m_umlObject->asUMLAssociation();
    return umla->changeability(role);
}

/**
 * Sets the changeability on the given end of the Association.
 */
void AssociationWidget::setChangeability(Uml::Changeability::Enum value, Uml::RoleType::Enum role)
{
    if (value == changeability(role))
        return;
    QString changeString = Uml::Changeability::toString(value);
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)  // update our model object
        association()->setChangeability(value, role);
    m_role[role].changeability = value;
    // update our string representation
    setChangeWidget(changeString, role);
}

/**
 * For internal purposes only.
 * Other classes/users should use setChangeability() instead.
 */
void AssociationWidget::setChangeWidget(const QString &strChangeWidget, Uml::RoleType::Enum role)
{
    bool newLabel = false;
    TextRole::Enum tr = (role == RoleType::A ? TextRole::ChangeA : TextRole::ChangeB);

    if (!m_role[role].changeabilityWidget) {
        // Don't construct the FloatingTextWidget if the string is empty.
        if (strChangeWidget.isEmpty())
            return;

        newLabel = true;
        m_role[role].changeabilityWidget = new FloatingTextWidget(m_scene, tr, strChangeWidget);
        m_role[role].changeabilityWidget->setParentItem(this);
        m_role[role].changeabilityWidget->setLink(this);
        m_scene->addFloatingTextWidget(m_role[role].changeabilityWidget);
        m_role[role].changeabilityWidget->setPreText(QLatin1String("{")); // all types have this
        m_role[role].changeabilityWidget->setPostText(QLatin1String("}")); // all types have this
    } else {
        if (m_role[role].changeabilityWidget->text().isEmpty()) {
            newLabel = true;
        }
        m_role[role].changeabilityWidget->setText(strChangeWidget);
    }
    m_role[role].changeabilityWidget->setActivated();

    if (newLabel) {
        setTextPosition(tr);
    }

    if (FloatingTextWidget::isTextValid(m_role[role].changeabilityWidget->text()))
        m_role[role].changeabilityWidget->show();
    else
        m_role[role].changeabilityWidget->hide();
}

/**
 * Returns true if the line path starts at the given widget.
 */
bool AssociationWidget::linePathStartsAt(const UMLWidget* widget)
{
    QPointF lpStart = m_associationLine.point(0);
    int startX = lpStart.x();
    int startY = lpStart.y();
    int wX = widget->scenePos().x();
    int wY = widget->scenePos().y();
    int wWidth = widget->width();
    int wHeight = widget->height();
    bool result1 = (startX >= wX && startX <= wX + wWidth &&
                    startY >= wY && startY <= wY + wHeight);

    bool result = widget->contains(m_associationLine.point(0));
    DEBUG(DBG_SRC) << "widget=" << widget->name() << " : result1=" << result1 << " / result=" << result;
    return result || result1;
}

/**
 * This function calculates which role should be set for the m_nameWidget FloatingTextWidget.
 */
Uml::TextRole::Enum AssociationWidget::calculateNameType(Uml::TextRole::Enum defaultRole)
{
    TextRole::Enum result = defaultRole;
    if (m_scene->type() == DiagramType::Collaboration) {
        if (m_role[RoleType::A].umlWidget == m_role[RoleType::B].umlWidget) {
            result = TextRole::Coll_Message;//for now same as other Coll_Message
        } else {
            result = TextRole::Coll_Message;
        }
    } else if (m_scene->type() == DiagramType::Sequence) {
        if (m_role[RoleType::A].umlWidget == m_role[RoleType::B].umlWidget) {
            result = TextRole::Seq_Message_Self;
        } else {
            result = TextRole::Seq_Message;
        }
    }

    return result;
}

/**
 * Gets the given role widget.
 *
 * @return  Pointer to the role's UMLWidget.
 */
UMLWidget* AssociationWidget::widgetForRole(Uml::RoleType::Enum role) const
{
    return m_role[role].umlWidget;
}

/**
 * Cleans up all the association's data in the related widgets.
 */
void AssociationWidget::cleanup()
{
    //let any other associations know we are going so they can tidy their positions up
    if (m_role[RoleType::A].m_nTotalCount > 2)
        updateAssociations(m_role[RoleType::A].m_nTotalCount - 1, m_role[RoleType::A].m_WidgetRegion, RoleType::A);
    if (m_role[RoleType::B].m_nTotalCount > 2)
        updateAssociations(m_role[RoleType::B].m_nTotalCount - 1, m_role[RoleType::B].m_WidgetRegion, RoleType::B);

    m_role[RoleType::A].cleanup();
    m_role[RoleType::B].cleanup();

    if (m_nameWidget) {
        m_scene->removeWidget(m_nameWidget);
        m_nameWidget = 0;
    }

    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association) {
        /*
           We do not remove the UMLAssociation from the document.
           Why? - Well, for example we might be in the middle of
           a cut/paste. If the UMLAssociation is removed by the cut
           then upon pasteing we have a problem.
           This is not quite clean yet - there should be a way to
           explicitly delete a UMLAssociation.  The Right Thing would
           be to have a ListView representation for UMLAssociation.
        `
                IF we are cut n pasting, why are we handling this association as a pointer?
                We should be using the XMI representation for a cut and paste. This
                allows us to be clean here, AND a choice of recreating the object
                w/ same id IF its a "cut", or a new object if its a "copy" operation
                (in which case we wouldnt be here, in cleanup()).
         */
        setUMLAssociation(0);
    }

    m_associationLine.cleanup();
    removeAssocClassLine();
}

/**
 * @brief Return state if the association line point in the vicinity of the last context
 *        menu event position is addable or not.
 * A point is addable if the association is not an Exception and there is no point nearby.
 *
 * @return true if point is addable
 */
bool AssociationWidget::isPointAddable()
{
    if (!isSelected() || associationType() == Uml::AssociationType::Exception)
        return false;
    int i = m_associationLine.closestPointIndex(m_eventScenePos);
    return i == -1;
}

/**
 * @brief Return state if the association line point in the vicinity of the last context
 *        menu event position is removable or not.
 * A point is removable if the association is not an Exception and is not the start or end point.
 *
 * @return true if point is removable
 */
bool AssociationWidget::isPointRemovable()
{
    if (!isSelected() || associationType() == Uml::AssociationType::Exception || m_associationLine.count() <= 2)
        return false;
    int i = m_associationLine.closestPointIndex(m_eventScenePos);
    return i > 0 && i < m_associationLine.count() - 1;
}

bool AssociationWidget::isAutoLayouted()
{
    if (associationType() == Uml::AssociationType::Exception)
        return true;
    if (!isSelected() || m_associationLine.count() <= 2)
        return false;
    return  m_associationLine.isAutoLayouted();
}

/**
 * if layout of this widget can be changed
 * @return true if layout could be changed
 * @return false if layout could not be changed
 */
bool AssociationWidget::isLayoutChangeable()
{
    return associationType() != Uml::AssociationType::Exception;
}

/**
 * Set our internal umlAssociation.
 */
void AssociationWidget::setUMLAssociation (UMLAssociation * assoc)
{
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association) {
        UMLAssociation *umla = association();

        // safety check. Did some num-nuts try to set the existing
        // association again? If so, just bail here
        if (umla == assoc)
            return;

        //umla->disconnect(this);  //Qt does disconnect automatically upon destruction.
        umla->nrof_parent_widgets--;

        // ANSWER: This is the wrong treatment of cut and paste. Associations that
        // are being cut/n pasted should be serialized to XMI, then reconstituted
        // (IF a paste operation) rather than passing around object pointers. Its
        // just too hard otherwise to prevent problems in the code. Bottom line: we need to
        // delete orphaned associations or we well get code crashes and memory leaks.
        if (umla->nrof_parent_widgets <= 0) {
            //umla->deleteLater();
        }

        m_umlObject = 0;
    }

    if (assoc) {
        m_umlObject = assoc;

        // move counter to "0" from "-1" (which means, no assocwidgets)
        if (assoc->nrof_parent_widgets < 0)
            assoc->nrof_parent_widgets = 0;

        assoc->nrof_parent_widgets++;
        connect(assoc, SIGNAL(modified()), this, SLOT(syncToModel()));
    }

}

/**
 * Returns true if the Widget is either at the starting or ending side of the association.
 */
bool AssociationWidget::containsAsEndpoint(UMLWidget* widget)
{
    return (widget == m_role[RoleType::A].umlWidget || widget == m_role[RoleType::B].umlWidget);
}

/**
 * Returns true if this AssociationWidget represents a collaboration message.
 */
bool AssociationWidget::isCollaboration() const
{
    Uml::AssociationType::Enum at = associationType();
    return (at == AssociationType::Coll_Mesg_Sync
            || at == AssociationType::Coll_Mesg_Async
            || at == AssociationType::Coll_Mesg_Self);
}

/**
 * Returns true if this AssociationWidget represents a self message.
 */
bool AssociationWidget::isSelf() const
{
    return widgetForRole(Uml::RoleType::A) == widgetForRole(Uml::RoleType::B);
}

/**
 * Gets the association's type.
 *
 * @return  This AssociationWidget's AssociationType::Enum.
 */
Uml::AssociationType::Enum AssociationWidget::associationType() const
{
    if (m_umlObject == 0 || m_umlObject->baseType() != UMLObject::ot_Association)
        return m_associationType;
    const UMLAssociation *umla = m_umlObject->asUMLAssociation();
    return umla->getAssocType();
}

/**
 * Sets the association's type.
 *
 * @param type   The AssociationType::Enum to set.
 */
void AssociationWidget::setAssociationType(Uml::AssociationType::Enum type)
{
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association) {
        association()->setAssociationType(type);
    }
    m_associationType = type;
    // If the association new type is not supposed to have Multiplicity
    // FloatingTexts and a Role FloatingTextWidget then set the texts
    // to empty.
    // NB We do not physically delete the floatingtext widgets here because
    // those widgets are also stored in the UMLView::m_WidgetList.
    if (!AssocRules::allowMultiplicity(type, widgetForRole(RoleType::A)->baseType())) {
        if (m_role[RoleType::A].multiplicityWidget) {
            m_role[RoleType::A].multiplicityWidget->setName(QString());
        }
        if (m_role[RoleType::B].multiplicityWidget) {
            m_role[RoleType::B].multiplicityWidget->setName(QString());
        }
    }
    if (!AssocRules::allowRole(type)) {
        if (m_role[RoleType::A].roleWidget) {
            m_role[RoleType::A].roleWidget->setName(QString());
        }
        if (m_role[RoleType::B].roleWidget) {
            m_role[RoleType::B].roleWidget->setName(QString());
        }
        setRoleDocumentation(QString(), RoleType::A);
        setRoleDocumentation(QString(), RoleType::B);
    }
    m_associationLine.reconstructSymbols();
    m_associationLine.updatePenStyle();
}

/**
 * Gets the ID of the given role widget.
 */
Uml::ID::Type AssociationWidget::widgetIDForRole(Uml::RoleType::Enum role) const
{
    if (m_role[role].umlWidget == 0) {
        if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association) {
            const UMLAssociation *umla = m_umlObject->asUMLAssociation();
            return umla->getObjectId(role);
        }
        uError() << "umlWidget is NULL";
        return Uml::ID::None;
    }
    if (m_role[role].umlWidget->isObjectWidget())
        return m_role[role].umlWidget->localID();
    Uml::ID::Type id = m_role[role].umlWidget->id();
    return id;
}

/**
 * Gets the local ID of the given role widget.
 */
Uml::ID::Type AssociationWidget::widgetLocalIDForRole(Uml::RoleType::Enum role) const
{
    if (m_role[role].umlWidget == 0) {
        if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association) {
            const UMLAssociation *umla = m_umlObject->asUMLAssociation();
            return umla->getObjectId(role);
        }
        uError() << "umlWidget is NULL";
        return Uml::ID::None;
    }
    Uml::ID::Type id = m_role[role].umlWidget->localID();
    return id;
}

/**
 * Returns a QString Object representing this AssociationWidget.
 */
QString AssociationWidget::toString() const
{
    QString string;
    static const QChar colon(QLatin1Char(':'));
    static const QChar space(QLatin1Char(' '));

    if (widgetForRole(RoleType::A)) {
        string = widgetForRole(RoleType::A)->name();
    }
    string.append(colon);

    if (m_role[RoleType::A].roleWidget) {
        string += m_role[RoleType::A].roleWidget->text();
    }
    string.append(space);
    string.append(Uml::AssociationType::toStringI18n(associationType()));
    string.append(space);

    if (widgetForRole(RoleType::B)) {
        string += widgetForRole(RoleType::B)->name();
    }

    string.append(colon);
    if (m_role[RoleType::B].roleWidget) {
        string += m_role[RoleType::B].roleWidget->text();
    }

    return string;
}

/**
 * Adds a break point (if left mouse button).
 */
void AssociationWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        uDebug() << "widget = " << name() << " / type = " << baseTypeStr();
        showPropertiesDialog();
        event->accept();
    }
}

/**
 * Overrides moveEvent.
 */
void AssociationWidget::moveEvent(QGraphicsSceneMouseEvent *me)
{
    // 2004-04-30: Achim Spangler
    // Simple Approach to block moveEvent during load of XMI
    /// @todo avoid trigger of this event during load

    if (umlDoc()->loading()) {
        // hmmh - change of position during load of XMI
        // -> there is something wrong
        // -> avoid movement during opening
        // -> print warn and stay at old position
        uWarning() << "called during load of XMI for ViewType: "
            << m_scene->type() << ", and BaseType: " << baseType();
        return;
    }
    /*to be here a line segment has moved.
      we need to see if the three text widgets needs to be moved.
      there are a few things to check first though:

      1) Do they exist
      2) does it need to move:
      2a) for the multi widgets only move if they changed region, otherwise they are close enough
      2b) for role name move if the segment it is on moves.
    */
    //first see if either the first or last segments moved, else no need to recalculate their point positions

    QPointF oldNamePoint = calculateTextPosition(TextRole::Name);
    QPointF oldMultiAPoint = calculateTextPosition(TextRole::MultiA);
    QPointF oldMultiBPoint = calculateTextPosition(TextRole::MultiB);
    QPointF oldChangeAPoint = calculateTextPosition(TextRole::ChangeA);
    QPointF oldChangeBPoint = calculateTextPosition(TextRole::ChangeB);
    QPointF oldRoleAPoint = calculateTextPosition(TextRole::RoleAName);
    QPointF oldRoleBPoint = calculateTextPosition(TextRole::RoleBName);

    int movingPoint = m_associationLine.closestPointIndex(me->scenePos());
    if (movingPoint != -1)
        m_associationLine.setPoint(movingPoint, me->scenePos());
    int pos = m_associationLine.count() - 1;//set to last point for widget b

    if ( movingPoint == 1 || (movingPoint == pos-1) ) {
        calculateEndingPoints();
    }
    if (m_role[RoleType::A].changeabilityWidget && (movingPoint == 1)) {
        setTextPositionRelatively(TextRole::ChangeA, oldChangeAPoint);
    }
    if (m_role[RoleType::B].changeabilityWidget && (movingPoint == 1)) {
        setTextPositionRelatively(TextRole::ChangeB, oldChangeBPoint);
    }
    if (m_role[RoleType::A].multiplicityWidget && (movingPoint == 1)) {
        setTextPositionRelatively(TextRole::MultiA, oldMultiAPoint);
    }
    if (m_role[RoleType::B].multiplicityWidget && (movingPoint == pos-1)) {
        setTextPositionRelatively(TextRole::MultiB, oldMultiBPoint);
    }

    if (m_nameWidget) {
        if (movingPoint == m_unNameLineSegment ||
                movingPoint - 1 == m_unNameLineSegment) {
            setTextPositionRelatively(TextRole::Name, oldNamePoint);
        }
    }

    if (m_role[RoleType::A].roleWidget) {
        setTextPositionRelatively(TextRole::RoleAName, oldRoleAPoint);
    }
    if (m_role[RoleType::B].roleWidget) {
        setTextPositionRelatively(TextRole::RoleBName, oldRoleBPoint);
    }

    if (m_pAssocClassLine) {
        computeAssocClassLine();
    }
}

/**
 * Calculates and sets the first and last point in the Association's AssociationLine.
 * Each point is a middle point of its respecting UMLWidget's Bounding rectangle
 * or a corner of it.
 * This method picks which sides to use for the association.
 */
void AssociationWidget::calculateEndingPoints()
{
    /*
     * For each UMLWidget the diagram is divided in four regions by its diagonals
     * as indicated below
     *                              Region 2
     *                         \                /
     *                           \            /
     *                             +--------+
     *                             | \    / |
     *                Region 1     |   ><   |    Region 3
     *                             | /    \ |
     *                             +--------+
     *                           /            \
     *                         /                \
     *                              Region 4
     *
     * Each diagonal is defined by two corners of the bounding rectangle
     *
     * To calculate the first point in the AssociationLine we have to find out in which
     * Region (defined by WidgetA's diagonals) is WidgetB's center
     * (let's call it Region M.) After that the first point will be the middle
     * point of the rectangle's side contained in Region M.
     *
     * To calculate the last point in the AssociationLine we repeat the above but
     * in the opposite direction (from widgetB to WidgetA)
     */

    UMLWidget *pWidgetA = m_role[RoleType::A].umlWidget;
    UMLWidget *pWidgetB = m_role[RoleType::B].umlWidget;
    if (!pWidgetA || !pWidgetB) {
        uWarning() << "Returning - one of the role widgets is not set.";
        return;
    }

    int size = m_associationLine.count();
    if (size < 2) {
        QPointF pA = pWidgetA->scenePos();
        QPointF pB = pWidgetB->scenePos();
        QPolygonF polyA = pWidgetA->shape().toFillPolygon().translated(pA);
        QPolygonF polyB = pWidgetB->shape().toFillPolygon().translated(pB);
        QLineF nearestPoints = Widget_Utils::closestPoints(polyA, polyB);
        if (nearestPoints.isNull()) {
            uError() << "Widget_Utils::closestPoints failed, falling back to simple widget positions";
        } else {
            pA = nearestPoints.p1();
            pB = nearestPoints.p2();
        }
        m_associationLine.setEndPoints(pA, pB);
    }

    // See if an association to self.
    // See if it needs to be set up before we continue:
    // If self association/message and doesn't have the minimum 4 points
    // then create it. Make sure no points are out of bounds of viewing area.
    // This only happens on first time through that we are worried about.
    if (isSelf() && size < 4) {
        createPointsSelfAssociation();
        return;
    }

    if (associationType() == AssociationType::Exception && size < 4) {
        createPointsException();
        updatePointsException();
        return;
    }

    // If the line has more than one segment change the values to calculate
    // from widget to point 1.
    qreal xB = pWidgetB->scenePos().x() + pWidgetB->width() / 2;
    qreal yB = pWidgetB->scenePos().y() + pWidgetB->height() / 2;
    if (size > 2) {
        QPointF p = m_associationLine.point(1);
        xB = p.x();  // m_associationLine points are scene positions,
        yB = p.y();  // therefore we don't need to call scenePos() here
    }
    doUpdates(QPointF(xB, yB), RoleType::A);

    // Now do the same for widgetB.
    // If the line has more than one segment change the values to calculate
    // from widgetB to the last point away from it.
    qreal xA = pWidgetA->scenePos().x() + pWidgetA->width() / 2;
    qreal yA = pWidgetA->scenePos().y() + pWidgetA->height() / 2;
    if (size > 2 ) {
        QPointF p = m_associationLine.point(size - 2);
        xA = p.x();
        yA = p.y();
    }
    doUpdates(QPointF(xA, yA), RoleType::B);

    computeAssocClassLine();
}

/**
 * Used by @ref calculateEndingPoints.
 */
void AssociationWidget::doUpdates(const QPointF &otherP, RoleType::Enum role)
{
    // Find widget region.
    Uml::Region::Enum oldRegion = m_role[role].m_WidgetRegion;
    UMLWidget *pWidget = m_role[role].umlWidget;
    QRectF rc(pWidget->scenePos().x(), pWidget->scenePos().y(),
              pWidget->width(), pWidget->height());
    Uml::Region::Enum& region = m_role[role].m_WidgetRegion;  // alias for brevity
    region = findPointRegion(rc, otherP);
    // Move some regions to the standard ones.
    switch( region ) {
    case Uml::Region::NorthWest:
        region = Uml::Region::North;
        break;
    case Uml::Region::NorthEast:
        region = Uml::Region::East;
        break;
    case Uml::Region::SouthEast:
        region = Uml::Region::South;
        break;
    case Uml::Region::SouthWest:
    case Uml::Region::Center:
        region = Uml::Region::West;
        break;
    default:
        break;
    }
    int regionCount = getRegionCount(region, role) + 2; //+2 = (1 for this one and one to halve it)
    int totalCount = m_role[role].m_nTotalCount;
    if (oldRegion != region) {
        updateRegionLineCount(regionCount - 1, regionCount, region, role);
        updateAssociations(totalCount - 1, oldRegion, role);
    } else if (totalCount != regionCount) {
        updateRegionLineCount(regionCount - 1, regionCount, region, role);
    } else {
        updateRegionLineCount(m_role[role].m_nIndex, totalCount, region, role);
    }
    updateAssociations(regionCount, region, role);
}

/**
 * Read property of bool m_activated.
 */
bool AssociationWidget::isActivated() const
{
    return m_activated;
}

/**
 * Set the m_activated flag of a widget but does not perform the Activate method.
 */
void AssociationWidget::setActivated(bool active)
{
    m_activated = active;
}

/**
 * Synchronize this widget from the UMLAssociation.
 */
void AssociationWidget::syncToModel()
{
    UMLAssociation *uml = association();

    if (uml == 0) {
        UMLAttribute *attr = attribute();
        if (attr == 0)
            return;
        setVisibility(attr->visibility(), RoleType::B);
        setRoleName(attr->name(), RoleType::B);
        return;
    }
    // block signals until finished
    uml->blockSignals(true);

    setName(uml->name());
    setRoleName(uml->getRoleName(RoleType::A), RoleType::A);
    setRoleName(uml->getRoleName(RoleType::B), RoleType::B);
    setVisibility(uml->visibility(RoleType::A), RoleType::A);
    setVisibility(uml->visibility(RoleType::B), RoleType::B);
    setChangeability(uml->changeability(RoleType::A), RoleType::A);
    setChangeability(uml->changeability(RoleType::B), RoleType::B);
    setMultiplicity(uml->getMultiplicity(RoleType::A), RoleType::A);
    setMultiplicity(uml->getMultiplicity(RoleType::B), RoleType::B);

    uml->blockSignals(false);
}

/**
 * Merges/syncs the association widget data into UML object
 * representation.
 * This will synchronize UMLAssociation w/ this new Widget
 * CHECK: Can we get rid of this.
 */
void AssociationWidget::mergeAssociationDataIntoUMLRepresentation()
{
    UMLAssociation *umlassoc = association();
    UMLAttribute *umlattr = attribute();
    if (umlassoc == 0 && umlattr == 0)
        return;

    // block emit modified signal, or we get a horrible loop
    m_umlObject->blockSignals(true);

    // would be desirable to do the following
    // so that we can be sure its back to initial state
    // in case we missed something here.
    //uml->init();

    // floating text widgets
    FloatingTextWidget *text = nameWidget();
    if (text)
        m_umlObject->setName(text->text());

    text = roleWidget(RoleType::A);
    if (text && umlassoc)
        umlassoc->setRoleName(text->text(), RoleType::A);

    text = roleWidget(RoleType::B);
    if (text) {
        if (umlassoc)
            umlassoc->setRoleName(text->text(), RoleType::B);
        else if (umlattr)
            umlattr->setName(text->text());
    }

    text = multiplicityWidget(RoleType::A);
    if (text && umlassoc)
        umlassoc->setMultiplicity(text->text(), RoleType::A);

    text = multiplicityWidget(RoleType::B);
    if (text && umlassoc)
        umlassoc->setMultiplicity(text->text(), RoleType::B);

    // unblock
    m_umlObject->blockSignals(false);
}

/**
 * Auxiliary method for widgetMoved():
 * Saves all ideally computed floatingtext positions before doing any
 * kind of change.  This is necessary because a single invocation of
 * calculateEndingPoints() modifies the AssociationLine ending points on ALL
 * AssociationWidgets.  This means that if we don't save the old ideal
 * positions then they are irretrievably lost as soon as
 * calculateEndingPoints() is invoked.
 */
void AssociationWidget::saveIdealTextPositions()
{
    m_oldNamePoint    = calculateTextPosition(TextRole::Name);
    m_oldMultiAPoint  = calculateTextPosition(TextRole::MultiA);
    m_oldMultiBPoint  = calculateTextPosition(TextRole::MultiB);
    m_oldChangeAPoint = calculateTextPosition(TextRole::ChangeA);
    m_oldChangeBPoint = calculateTextPosition(TextRole::ChangeB);
    m_oldRoleAPoint   = calculateTextPosition(TextRole::RoleAName);
    m_oldRoleBPoint   = calculateTextPosition(TextRole::RoleBName);
}

/**
 * Adjusts the ending point of the association that connects to Widget.
 */
void AssociationWidget::widgetMoved(UMLWidget* widget, qreal dx, qreal dy)
{
    Q_UNUSED(dx); Q_UNUSED(dy);

    // Simple Approach to block moveEvent during load of XMI
    /// @todo avoid trigger of this event during load
    if (umlDoc()->loading()) {
        // change of position during load of XMI
        // -> there is something wrong
        // -> avoid movement during opening
        // -> print warn and stay at old position
        DEBUG(DBG_SRC) << "called during load of XMI for ViewType: " << m_scene->type()
                       << ", and BaseType: " << baseTypeStr();
        return;
    }

    DEBUG(DBG_SRC) << "association type=" << Uml::AssociationType::toString(associationType());
    if (associationType() == AssociationType::Exception) {
        updatePointsException();
        setTextPosition(TextRole::Name);
    }
    else {
        calculateEndingPoints();
        computeAssocClassLine();
    }

    // Assoc to self - move all points:
    if (isSelf()) {
        updatePointsSelfAssociation();

        if (m_nameWidget && !m_nameWidget->isSelected()) {
            setTextPositionRelatively(TextRole::Name, m_oldNamePoint);
        }

    }//end if widgetA = widgetB
    else if (m_role[RoleType::A].umlWidget == widget) {
        if (m_nameWidget && m_unNameLineSegment == 0 && !m_nameWidget->isSelected() ) {
            //only calculate position and move text if the segment it is on is moving
            setTextPositionRelatively(TextRole::Name, m_oldNamePoint);
        }
        if (m_role[RoleType::B].umlWidget && m_role[RoleType::B].umlWidget->changesShape())
            m_role[RoleType::B].umlWidget->updateGeometry(false);
    }//end if widgetA moved
    else if (m_role[RoleType::B].umlWidget == widget) {
        const int size = m_associationLine.count();
        if (m_nameWidget && (m_unNameLineSegment == size-2) && !m_nameWidget->isSelected() ) {
            //only calculate position and move text if the segment it is on is moving
            setTextPositionRelatively(TextRole::Name, m_oldNamePoint);
        }
        if (m_role[RoleType::A].umlWidget && m_role[RoleType::A].umlWidget->changesShape())
            m_role[RoleType::A].umlWidget->updateGeometry(false);
    }//end if widgetB moved

    if (m_role[RoleType::A].roleWidget && !m_role[RoleType::A].roleWidget->isSelected()) {
        setTextPositionRelatively(TextRole::RoleAName, m_oldRoleAPoint);
    }
    if (m_role[RoleType::B].roleWidget && !m_role[RoleType::B].roleWidget->isSelected()) {
        setTextPositionRelatively(TextRole::RoleBName, m_oldRoleBPoint);
    }
    if (m_role[RoleType::A].multiplicityWidget && !m_role[RoleType::A].multiplicityWidget->isSelected()) {
        setTextPositionRelatively(TextRole::MultiA, m_oldMultiAPoint);
    }
    if (m_role[RoleType::B].multiplicityWidget && !m_role[RoleType::B].multiplicityWidget->isSelected()) {
        setTextPositionRelatively(TextRole::MultiB, m_oldMultiBPoint);
    }
    if (m_role[RoleType::A].changeabilityWidget && !m_role[RoleType::A].changeabilityWidget->isSelected()) {
        setTextPositionRelatively(TextRole::ChangeA, m_oldChangeAPoint);
    }
    if (m_role[RoleType::B].changeabilityWidget && !m_role[RoleType::B].changeabilityWidget->isSelected()) {
        setTextPositionRelatively(TextRole::ChangeB, m_oldChangeBPoint);
    }
}

/**
 * Creates the points of the self association.
 * Method called when a widget end points are calculated by calculateEndingPoints().
 */
void AssociationWidget::createPointsSelfAssociation()
{
    UMLWidget *pWidgetA = m_role[RoleType::A].umlWidget;

    const int DISTANCE = 50;
    qreal x = pWidgetA->x();
    qreal y = pWidgetA->y();
    qreal h = pWidgetA->height();
    qreal w = pWidgetA->width();
    // see if above widget ok to start
    if (y - DISTANCE > 0) {
        m_associationLine.setEndPoints(QPointF(x + w / 4, y) , QPointF(x + w * 3 / 4, y));
        m_associationLine.insertPoint(1, QPointF(x + w / 4, y - DISTANCE));
        m_associationLine.insertPoint(2, QPointF(x + w * 3 / 4, y - DISTANCE));
        m_role[RoleType::A].m_WidgetRegion = m_role[RoleType::B].m_WidgetRegion = Uml::Region::North;
    } else {
        m_associationLine.setEndPoints(QPointF(x + w / 4, y + h), QPointF(x + w * 3 / 4, y + h));
        m_associationLine.insertPoint(1, QPointF(x + w / 4, y + h + DISTANCE));
        m_associationLine.insertPoint(2, QPointF(x + w * 3 / 4, y + h + DISTANCE));
        m_role[RoleType::A].m_WidgetRegion = m_role[RoleType::B].m_WidgetRegion = Uml::Region::South;
    }
}

/**
 * Adjusts the points of the self association.
 * Method called when a widget was moved by widgetMoved(widget, x, y).
 */
void AssociationWidget::updatePointsSelfAssociation()
{
    UMLWidget *pWidgetA = m_role[RoleType::A].umlWidget;

    const int DISTANCE = 50;
    qreal x = pWidgetA->x();
    qreal y = pWidgetA->y();
    qreal h = pWidgetA->height();
    qreal w = pWidgetA->width();
    // see if above widget ok to start
    if (y - DISTANCE > 0) {
        m_associationLine.setEndPoints(QPointF(x + w / 4, y) , QPointF(x + w * 3 / 4, y));
        m_associationLine.setPoint(1, QPointF(x + w / 4, y - DISTANCE));
        m_associationLine.setPoint(2, QPointF(x + w * 3 / 4, y - DISTANCE));
        m_role[RoleType::A].m_WidgetRegion = m_role[RoleType::B].m_WidgetRegion = Uml::Region::North;
    } else {
        m_associationLine.setEndPoints(QPointF(x + w / 4, y + h), QPointF(x + w * 3 / 4, y + h));
        m_associationLine.setPoint(1, QPointF(x + w / 4, y + h + DISTANCE));
        m_associationLine.setPoint(2, QPointF(x + w * 3 / 4, y + h + DISTANCE));
        m_role[RoleType::A].m_WidgetRegion = m_role[RoleType::B].m_WidgetRegion = Uml::Region::South;
    }
}

/**
 * Creates the points of the association exception.
 * Method called when a widget end points are calculated by calculateEndingPoints().
 */
void AssociationWidget::createPointsException()
{
    UMLWidget *pWidgetA = m_role[RoleType::A].umlWidget;
    UMLWidget *pWidgetB = m_role[RoleType::B].umlWidget;

    qreal xa = pWidgetA->x();
    qreal ya = pWidgetA->y();
    qreal ha = pWidgetA->height();
    qreal wa = pWidgetA->width();

    qreal xb = pWidgetB->x();
    qreal yb = pWidgetB->y();
    qreal hb = pWidgetB->height();
    //qreal wb = pWidgetB->width();

    m_associationLine.setEndPoints(QPointF(xa + wa , ya + ha/2) , QPointF(xb , yb + hb/2));
    m_associationLine.insertPoint(1, QPointF(xa + wa , ya + ha/2));
    m_associationLine.insertPoint(2, QPointF(xb , yb + hb/2));
}

/**
 * Adjusts the points of the association exception.
 * Method called when a widget was moved by widgetMoved(widget, x, y).
 */
void AssociationWidget::updatePointsException()
{
    UMLWidget *pWidgetA = m_role[RoleType::A].umlWidget;
    UMLWidget *pWidgetB = m_role[RoleType::B].umlWidget;

    qreal xa = pWidgetA->x();
    qreal ya = pWidgetA->y();
    qreal ha = pWidgetA->height();
    qreal wa = pWidgetA->width();

    qreal xb = pWidgetB->x();
    qreal yb = pWidgetB->y();
    qreal hb = pWidgetB->height();
    qreal wb = pWidgetB->width();
    qreal xmil, ymil;
    qreal xdeb, ydeb;
    qreal xfin, yfin;
    qreal ESPACEX, ESPACEY;
    QPointF p1;
    QPointF p2;
    //calcul des coordonnées au milieu de la flèche eclair
    if (xb - xa - wa >= 45) {
        ESPACEX = 0;
        xdeb = xa + wa;
        xfin = xb;
    } else if (xa - xb - wb > 45) {
        ESPACEX = 0;
        xdeb = xa;
        xfin = xb + wb;
    } else {
        ESPACEX = 15;
        xdeb = xa + wa/2;
        xfin = xb + wb/2;
    }

    xmil = xdeb + (xfin - xdeb)/2;

    if (yb - ya - ha >= 45)  {
        ESPACEY = 0;
        ydeb = ya + ha;
        yfin = yb;
    } else if (ya - yb - hb > 45) {
        ESPACEY = 0;
        ydeb = ya;
        yfin = yb + hb;
    } else {
        ESPACEY = 15;
        ydeb = ya + ha/2;
        yfin = yb + hb/2;
    }

    ymil = ydeb + (yfin - ydeb)/2;

    p1.setX(xmil + (xfin - xmil)*1/2); p1.setY(ymil + (yfin - ymil)*1/3);
    p2.setX(xmil - (xmil - xdeb)*1/2); p2.setY(ymil - (ymil - ydeb)*1/3);

    if (fabs(p1.x() - p2.x()) <= 10)
        ESPACEX = 15;
    if (fabs(p1.y() - p2.y()) <= 10)
        ESPACEY = 15;

    m_associationLine.setEndPoints(QPointF(xdeb, ydeb), QPointF(xfin, yfin));
    m_associationLine.setPoint(1, QPointF(p1.x() + ESPACEX, p1.y() + ESPACEY));
    m_associationLine.setPoint(2, QPointF(p2.x() - ESPACEX, p2.y() - ESPACEY));

    m_role[RoleType::A].m_WidgetRegion = m_role[RoleType::B].m_WidgetRegion = Uml::Region::North;
}

/**
 * Finds out which region of rectangle 'rect' contains the point 'pos' and returns the region
 * number:
 * 1 = Region 1
 * 2 = Region 2
 * 3 = Region 3
 * 4 = Region 4
 * 5 = On diagonal 2 between Region 1 and 2
 * 6 = On diagonal 1 between Region 2 and 3
 * 7 = On diagonal 2 between Region 3 and 4
 * 8 = On diagonal 1 between Region 4 and 1
 * 9 = On diagonal 1 and On diagonal 2 (the center)
 */
Uml::Region::Enum AssociationWidget::findPointRegion(const QRectF& rect, const QPointF &pos)
{
    qreal w = rect.width();
    qreal h = rect.height();
    qreal x = rect.x();
    qreal y = rect.y();
    qreal slope2 = w / h;
    qreal slope1 = slope2 *(-1.0);
    qreal b1 = x + w - (slope1 * y);
    qreal b2 = x - (slope2 * y);

    qreal eval1 = slope1 * pos.y() + b1;
    qreal eval2 = slope2 * pos.y() + b2;

    Uml::Region::Enum result = Uml::Region::Error;
    //if inside region 1
    if (eval1 > pos.x() && eval2 > pos.x()) {
        result = Uml::Region::West;
    }
    //if inside region 2
    else if (eval1 > pos.x() && eval2 < pos.x()) {
        result = Uml::Region::North;
    }
    //if inside region 3
    else if (eval1 < pos.x() && eval2 < pos.x()) {
        result = Uml::Region::East;
    }
    //if inside region 4
    else if (eval1 < pos.x() && eval2 > pos.x()) {
        result = Uml::Region::South;
    }
    //if inside region 5
    else if (eval1 == pos.x() && eval2 < pos.x()) {
        result = Uml::Region::NorthWest;
    }
    //if inside region 6
    else if (eval1 < pos.x() && eval2 == pos.x()) {
        result = Uml::Region::NorthEast;
    }
    //if inside region 7
    else if (eval1 == pos.x() && eval2 > pos.x()) {
        result = Uml::Region::SouthEast;
    }
    //if inside region 8
    else if (eval1 > pos.x() && eval2 == pos.x()) {
        result = Uml::Region::SouthWest;
    }
    //if inside region 9
    else if (eval1 == pos.x() && eval2 == pos.x()) {
        result = Uml::Region::Center;
    }
    return result;
}

/**
 * Returns a point with interchanged X and Y coordinates.
 */
QPointF AssociationWidget::swapXY(const QPointF &p)
{
    QPointF swapped( p.y(), p.x() );
    return swapped;
}

#if 0  // not used at the moment
/**
 * Calculates which point of segment P1P2 has a distance equal to
 * Distance from P1.
 * Let's say such point is PX, the distance from P1 to PX must be equal
 * to Distance and if PX is not a point of the segment P1P2 then the
 * function returns (-1, -1).
 */
QPointF AssociationWidget::calculatePointAtDistance(const QPointF &P1, const QPointF &P2, float Distance)
{
    /*
      the distance D between points (x1, y1) and (x3, y3) has the following formula:
          ---     ------------------------------
      D =    \   /         2         2
              \ /   (x3 - x1)  +  (y3 - y1)

      D, x1 and y1 are known and the point (x3, y3) is inside line (x1, y1)(x2, y2), so if the
      that line has the formula y = mx + b
      then y3 = m*x3 + b

       2             2             2
      D   = (x3 - x1)  +  (y3 - y1)

       2       2                 2      2                 2
      D    = x3    - 2*x3*x1 + x1   + y3   - 2*y3*y1  + y1

       2       2       2       2                  2
      D    - x1    - y1    = x3    - 2*x3*x1  + y3   - 2*y3*y1

       2       2       2       2                          2
      D    - x1    - y1    = x3    - 2*x3*x1  + (m*x3 + b)  - 2*(m*x3 + b)*y1

       2       2       2              2       2 2
      D    - x1    - y1   + 2*b*y1 - b   =  (m  + 1)*x3   + (-2*x1 + 2*m*b -2*m*y1)*x3

       2      2       2       2
      C  = - D    + x1    + y1   - 2*b*y1 + b


       2
      A  = (m    + 1)

      B  = (-2*x1 + 2*m*b -2*m*y1)

      and we have
       2
      A * x3 + B * x3 - C = 0

                         ---------------
             -B +  ---  /  2
                      \/  B   - 4*A*C
      sol_1  = --------------------------------
                       2*A


                         ---------------
             -B -  ---  /  2
                      \/  B   - 4*A*C
      sol_2  = --------------------------------
                       2*A


      then in the distance formula we have only one variable x3 and that is easy
      to calculate
    */
    int x1 = P1.y();
    int y1 = P1.x();
    int x2 = P2.y();
    int y2 = P2.x();

    if (x2 == x1) {
        return QPointF(x1, y1 + (int)Distance);
    }
    float slope = ((float)y2 - (float)y1) / ((float)x2 - (float)x1);
    float b = (y1 - slope*x1);
    float A = (slope * slope) + 1;
    float B = (2*slope*b) - (2*x1)  - (2*slope*y1);
    float C = (b*b) - (Distance*Distance) + (x1*x1) + (y1*y1) - (2*b*y1);
    float t = B*B - 4*A*C;

    if (t < 0) {
        return QPointF(-1, -1);
    }
    float sol_1 = ((-1* B) + sqrt(t)) / (2*A);
    float sol_2 = ((-1*B) - sqrt(t)) / (2*A);

    if (sol_1 < 0.0 && sol_2 < 0.0) {
        return QPointF(-1, -1);
    }
    QPointF sol1Point((int)(slope*sol_1 + b), (int)(sol_1));
    QPointF sol2Point((int)(slope*sol_2 + b), (int)(sol_2));
    if (sol_1 < 0 && sol_2 >=0) {
        if (x2 > x1) {
            if (x1 <= sol_2 && sol_2 <= x2)
                return sol2Point;
        } else {
            if (x2 <= sol_2 && sol_2 <= x1)
                return sol2Point;
        }
    } else if (sol_1 >= 0 && sol_2 < 0) {
        if (x2 > x1) {
            if (x1 <= sol_1 && sol_1 <= x2)
                return sol1Point;
        } else {
            if (x2 <= sol_1 && sol_1 <= x1)
                return sol1Point;
        }
    } else {
        if (x2 > x1) {
            if (x1 <= sol_1 && sol_1 <= x2)
                return sol1Point;
            if (x1 <= sol_2 && sol_2 <= x2)
                return sol2Point;
        } else {
            if (x2 <= sol_1 && sol_1 <= x1)
                return sol1Point;
            if (x2 <= sol_2 && sol_2 <= x1)
                return sol2Point;
        }
    }
    return QPointF(-1, -1);
}

/**
 * Calculates which point of a perpendicular line to segment P1P2 that contains P2
 *  has a distance equal to Distance from P2,
 * Lets say such point is P3,  the distance from P2 to P3 must be equal to Distance
 */
QPointF AssociationWidget::calculatePointAtDistanceOnPerpendicular(const QPointF &P1, const QPointF &P2, float Distance)
{
    /*
      the distance D between points (x2, y2) and (x3, y3) has the following formula:

          ---     ------------------------------
      D =    \   /         2             2
              \ / (x3 - x2)  +  (y3 - y2)

      D, x2 and y2 are known and line P2P3 is perpendicular to line (x1, y1)(x2, y2), so if the
      line P1P2 has the formula y = m*x + b,
      then      (x1 - x2)
          m =  -----------, because it is perpendicular to line P1P2
                (y2 - y1)

      also y2 = m*x2 + b
      => b = y2 - m*x2

      then P3 = (x3, m*x3 + b)

       2            2            2
      D  = (x3 - x2)  + (y3 - y2)

       2     2               2     2               2
      D  = x3  - 2*x3*x2 + x2  + y3  - 2*y3*y2 + y2

       2     2     2     2               2
      D  - x2  - y2  = x3  - 2*x3*x2 + y3  - 2*y3*y2



       2     2     2     2                       2
      D  - x2  - y2  = x3  - 2*x3*x2 + (m*x3 + b)  - 2*(m*x3 + b)*y2

       2     2     2                   2        2       2
      D  - x2  - y2  + 2*b*y2 - b  = (m  + 1)*x3  + (-2*x2 + 2*m*b -2*m*y2)*x3

              2       2       2              2
      C  = - D    + x2    + y2   - 2*b*y2 + b

             2
      A  = (m  + 1)

      B  = (-2*x2 + 2*m*b -2*m*y2)

      and we have
       2
      A * x3 + B * x3 - C = 0


                           ---------------
                     ---  /  2
                -B +    \/  B   - 4*A*C
      sol_1 = --------------------------------
                        2*A


                           ---------------
                     ---  /  2
                -B -    \/  B   - 4*A*C
      sol_2 = --------------------------------
                        2*A

      then in the distance formula we have only one variable x3 and that is easy
      to calculate
    */
    if (P1.x() == P2.x()) {
        return QPointF((int)(P2.x() + Distance), P2.y());
    }
    const int x1 = P1.y();
    const int y1 = P1.x();
    const int x2 = P2.y();
    const int y2 = P2.x();

    float slope = ((float)x1 - (float)x2) / ((float)y2 - (float)y1);
    float b = (y2 - slope*x2);
    float A = (slope * slope) + 1;
    float B = (2*slope*b) - (2*x2) - (2*slope*y2);
    float C = (b*b) - (Distance*Distance) + (x2*x2) + (y2*y2) - (2*b*y2);
    float t = B*B - 4*A*C;
    if (t < 0) {
        return QPointF(-1, -1);
    }
    float sol_1 = ((-1* B) + sqrt(t)) / (2*A);

    float sol_2 = ((-1*B) - sqrt(t)) / (2*A);

    if (sol_1 < 0 && sol_2 < 0) {
        return QPointF(-1, -1);
    }
    QPointF sol1Point((int)(slope*sol_1 + b), (int)sol_1);
    QPointF sol2Point((int)(slope*sol_2 + b), (int)sol_2);
    if (sol_1 < 0 && sol_2 >=0) {
        return sol2Point;
    } else if (sol_1 >= 0 && sol_2 < 0) {
        return sol1Point;
    } else {    // Choose one solution, either will work fine
        if (slope >= 0) {
            if (sol_1 <= sol_2)
                return sol2Point;
            else
                return sol1Point;
        } else {
            if (sol_1 <= sol_2)
                return sol1Point;
            else
                return sol2Point;
        }

    }
    return QPointF(-1, -1);  // never reached, just keep compilers happy
}

/** 
 * Calculates the intersection (PS) between line P1P2 and a perpendicular line containing
 * P3, the result is returned in ResultingPoint. and result value represents the distance
 * between ResultingPoint and P3; if this value is negative an error occurred. 
 */
float AssociationWidget::perpendicularProjection(const QPointF& P1, const QPointF& P2, const QPointF& P3,
        QPointF& ResultingPoint)
{
    //line P1P2 is Line 1 = y=slope1*x + b1

    //line P3PS is Line 1 = y=slope2*x + b2

    float slope2 = 0;
    float slope1 = 0;
    float sx = 0, sy = 0;
    int y2 = P2.x();
    int y1 = P1.x();
    int x2 = P2.y();
    int x1 = P1.y();
    int y3 = P3.x();
    int x3 = P3.y();
    float distance = 0;
    float b1 = 0;

    float b2 = 0;

    if (x2 == x1) {
        sx = x2;
        sy = y3;
    } else if (y2 == y1) {
        sy = y2;
        sx = x3;
    } else {
        slope1 = (y2 - y1)/ (x2 - x1);
        slope2 = (x1 - x2)/ (y2 - y1);
        b1 = y2 - (slope1 * x2);
        b2 = y3 - (slope2 * x3);
        sx = (b2 - b1) / (slope1 - slope2);
        sy = slope1*sx + b1;
    }
    distance = (int)(sqrt(((x3 - sx)*(x3 - sx)) + ((y3 - sy)*(y3 - sy))));

    ResultingPoint.setX((int)sy);
    ResultingPoint.setY((int)sx);

    return distance;
}
#endif

/**
 * Calculates the position of the text widget depending on the role
 * that widget is playing.
 * Returns the point at which to put the widget.
 */
QPointF AssociationWidget::calculateTextPosition(Uml::TextRole::Enum role)
{
    const int SPACE = 2;
    QPointF p(-1, -1), q(-1, -1);

    // used to find out if association end point (p)
    // is at top or bottom edge of widget.

    if (role == TextRole::MultiA || role == TextRole::ChangeA || role == TextRole::RoleAName) {
        p = m_associationLine.point(0);
        q = m_associationLine.point(1);
    } else if (role == TextRole::MultiB || role == TextRole::ChangeB || role == TextRole::RoleBName) {
        const int lastSegment = m_associationLine.count() - 1;
        p = m_associationLine.point(lastSegment);
        q = m_associationLine.point(lastSegment - 1);
    } else if (role != TextRole::Name) {
        uError() << "called with unsupported TextRole::Enum " << role;
        return QPointF(-1, -1);
    }

    FloatingTextWidget *text = textWidgetByRole(role);
    int textW = 0, textH = 0;
    if (text) {
        textW = text->width();
        textH = text->height();
    }

    qreal x = 0.0, y = 0.0;

    if (role == TextRole::MultiA || role == TextRole::MultiB) {
        const bool isHorizontal = (p.y() == q.y());
        const int atBottom = p.y() + SPACE;
        const int atTop = p.y() - SPACE - textH;
        const int atLeft = p.x() - SPACE - textW;
        const int atRight = p.x() + SPACE;
        y = (p.y() > q.y()) == isHorizontal ? atBottom : atTop;
        x = (p.x() < q.x()) == isHorizontal ? atRight : atLeft;

    } else if (role == TextRole::ChangeA || role == TextRole::ChangeB) {

        if (p.y() > q.y())
            y = p.y() - SPACE - (textH * 2);
        else
            y = p.y() + SPACE + textH;

        if (p.x() < q.x())
            x = p.x() + SPACE;
        else
            x = p.x() - SPACE - textW;

    } else if (role == TextRole::RoleAName || role == TextRole::RoleBName) {

        if (p.y() > q.y())
            y = p.y() - SPACE - textH;
        else
            y = p.y() + SPACE;

        if (p.x() < q.x())
            x = p.x() + SPACE;
        else
            x = p.x() - SPACE - textW;

    } else if (role == TextRole::Name) {

        calculateNameTextSegment();
        if (m_unNameLineSegment == -1) {
            uWarning() << "TODO:negative line segment index";
            m_unNameLineSegment = 0;
        }
        x = ( m_associationLine.point(m_unNameLineSegment).x() +
                     m_associationLine.point(m_unNameLineSegment + 1).x() ) / 2;
        y = ( m_associationLine.point(m_unNameLineSegment).y() +
                     m_associationLine.point(m_unNameLineSegment + 1).y() ) / 2;
    }

    if (text) {
        constrainTextPos(x, y, textW, textH, role);
    }
    p = QPointF( x, y );
    return p;
}

/**
 * Return the mid point between p0 and p1
 */
QPointF AssociationWidget::midPoint(const QPointF& p0, const QPointF& p1)
{
    QPointF midP;
    if (p0.x() < p1.x())
        midP.setX(p0.x() + (p1.x() - p0.x()) / 2);
    else
        midP.setX(p1.x() + (p0.x() - p1.x()) / 2);
    if (p0.y() < p1.y())
        midP.setY(p0.y() + (p1.y() - p0.y()) / 2);
    else
        midP.setY(p1.y() + (p0.y() - p1.y()) / 2);
    return midP;
}

/**
 * Constrains the FloatingTextWidget X and Y values supplied.
 * Implements the abstract operation from LinkWidget.
 *
 * @param textX       Candidate X value (may be modified by the constraint.)
 * @param textY       Candidate Y value (may be modified by the constraint.)
 * @param textWidth   Width of the text.
 * @param textHeight  Height of the text.
 * @param tr          Uml::Text_Role of the text.
 */
void AssociationWidget::constrainTextPos(qreal &textX, qreal &textY,
                                         qreal textWidth, qreal textHeight,
                                         Uml::TextRole::Enum tr)
{
    const int textCenterX = textX + textWidth / 2;
    const int textCenterY = textY + textHeight / 2;
    const int lastSegment = m_associationLine.count() - 1;
    QPointF p0, p1;
    switch (tr) {
        case TextRole::RoleAName:
        case TextRole::MultiA:
        case TextRole::ChangeA:
            p0 = m_associationLine.point(0);
            p1 = m_associationLine.point(1);
            // If we are dealing with a single line then tie the
            // role label to the proper half of the line, i.e.
            // the role label must be closer to the "other"
            // role object.
            if (lastSegment == 1)
                p1 = midPoint(p0, p1);
            break;
        case TextRole::RoleBName:
        case TextRole::MultiB:
        case TextRole::ChangeB:
            p0 = m_associationLine.point(lastSegment - 1);
            p1 = m_associationLine.point(lastSegment);
            if (lastSegment == 1)
                p0 = midPoint(p0, p1);
            break;
        case TextRole::Name:
        case TextRole::Coll_Message:  // CHECK: collab.msg texts seem to be TextRole::Name
        case TextRole::State:         // CHECK: is this used?
            // Find the linepath segment to which the (textX, textY) is closest
            // and constrain to the corridor of that segment (see farther below)
            {
                int minDistSquare = 100000;  // utopian initial value
                int lpIndex = 0;
                for (int i = 0; i < lastSegment; ++i) {
                    p0 = m_associationLine.point(i);
                    p1 = m_associationLine.point(i + 1);
                    QPointF midP = midPoint(p0, p1);
                    const int deltaX = textCenterX - midP.x();
                    const int deltaY = textCenterY - midP.y();
                    const int cSquare = deltaX * deltaX + deltaY * deltaY;
                    if (cSquare < minDistSquare) {
                        minDistSquare = cSquare;
                        lpIndex = i;
                    }
                }
                p0 = m_associationLine.point(lpIndex);
                p1 = m_associationLine.point(lpIndex + 1);
            }
            break;
        default:
            uError() << "unexpected TextRole::Enum " << tr;
            return;
            break;
    }
    /* Constraint:
       The midpoint between p0 and p1 is taken to be the center of a circle
       with radius D/2 where D is the distance between p0 and p1.
       The text center needs to be within this circle else it is constrained
       to the nearest point on the circle.
     */
    p0 = swapXY(p0);    // go to the natural coordinate system
    p1 = swapXY(p1);    // with (0,0) in the lower left corner
    QPointF midP = midPoint(p0, p1);
    // If (textX,textY) is not inside the circle around midP then
    // constrain (textX,textY) to the nearest point on that circle.
    const int x0 = p0.x();
    const int y0 = p0.y();
    const int x1 = p1.x();
    const int y1 = p1.y();
    double r = sqrt((double)((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0))) / 2;
    if (textWidth > r)
        r = textWidth;
    // swap textCenter{X,Y} to convert from Qt coord.system.
    const QPointF origTextCenter(textCenterY, textCenterX);
    const int relX = fabs(origTextCenter.x() - midP.x());
    const int relY = fabs(origTextCenter.y() - midP.y());
    const double negativeWhenInsideCircle = relX * relX + relY * relY - r * r;
    if (negativeWhenInsideCircle <= 0.0) {
        return;
    }
    /*
     The original constraint was to snap the text position to the
     midpoint but that creates unpleasant visual jitter:
    textX = midP.y() - textWidth / 2;   // go back to Qt coord.sys.
    textY = midP.x() - textHeight / 2;  // go back to Qt coord.sys.

     Rather, we project the text position onto the closest point
     on the circle:

     Circle equation:
       relX^2 + relY^2 - r^2 = 0, or in other words
       relY^2 = r^2 - relX^2, or
       relY = sqrt(r^2 - relX^2)
     Line equation:
       relY = a * relX + b
         We can omit "b" because relX and relY are already relative to
         the circle origin, therefore we can also write:
       a = relY / relX
     To obtain the point of intersection between the circle of radius r
     and the line connecting the circle origin with the point (relX, relY),
     we equate the relY:
       a * x = sqrt(r^2 - x^2), or in other words
       a^2 * x^2 = r^2 - x^2, or
       x^2 * (a^2 + 1) = r^2, or
       x^2 = r^2 / (a^2 + 1), or
       x = sqrt(r^2 / (a^2 + 1))
     and then
       y = a * x
     The resulting x and y are relative to the circle origin so we just add
     the circle origin (X, Y) to obtain the constrained (textX, textY).
     */
    // Handle the special case, relX = 0.
    if (relX == 0) {
        if (origTextCenter.y() > midP.y())
            textX = midP.y() + (int)r;   // go back to Qt coord.sys.
        else
            textX = midP.y() - (int)r;   // go back to Qt coord.sys.
        textX -= textWidth / 2;
        return;
    }
    const double a = (double)relY / (double)relX;
    const double x = sqrt(r*r / (a*a + 1));
    const double y = a * x;
    if (origTextCenter.x() > midP.x())
        textY = midP.x() + (int)x;   // go back to Qt coord.sys.
    else
        textY = midP.x() - (int)x;   // go back to Qt coord.sys.
    textY -= textHeight / 2;
    if (origTextCenter.y() > midP.y())
        textX = midP.y() + (int)y;   // go back to Qt coord.sys.
    else
        textX = midP.y() - (int)y;   // go back to Qt coord.sys.
    textX -= textWidth / 2;
}

/**
 * Puts the text widget with the given role at the given position.
 * This method calls @ref calculateTextPostion to get the needed position.
 * I.e. the line segment it is on has moved and it should move the same
 * amount as the line.
 */
void AssociationWidget::setTextPosition(Uml::TextRole::Enum role)
{
    bool startMove = false;
    if (m_role[RoleType::A].getStartMove())
        startMove = true;
    else if (m_role[RoleType::B].getStartMove())
        startMove = true;
    else if (m_nameWidget && m_nameWidget->getStartMove())
        startMove = true;
    if (startMove) {
        return;
    }
    FloatingTextWidget *ft = textWidgetByRole(role);
    if (ft == 0)
        return;
    QPointF pos = calculateTextPosition(role);
    ft->setX(pos.x());
    ft->setY(pos.y());
}

/**
 * Moves the text widget with the given role by the difference between
 * the two points.
 */
void AssociationWidget::setTextPositionRelatively(Uml::TextRole::Enum role, const QPointF &oldPosition)
{
    bool startMove = false;
    if (m_role[RoleType::A].getStartMove())
        startMove = true;
    else if (m_role[RoleType::B].getStartMove())
        startMove = true;
    else if (m_nameWidget && m_nameWidget->getStartMove())
        startMove = true;

    if (startMove) {
        return;
    }
    FloatingTextWidget *ft = textWidgetByRole(role);
    if (ft == 0)
        return;
    qreal ftX = ft->x();
    qreal ftY = ft->y();

    QPointF pos = calculateTextPosition(role);
    int relX = pos.x() - oldPosition.x();
    int relY = pos.y() - oldPosition.y();
    qreal ftNewX = ftX + relX;
    qreal ftNewY = ftY + relY;

    bool oldIgnoreSnapToGrid = ft->getIgnoreSnapToGrid();
    ft->setIgnoreSnapToGrid(true);
    ft->setX(ftNewX);
    ft->setY(ftNewY);
    ft->setIgnoreSnapToGrid(oldIgnoreSnapToGrid);
}

/**
 * Remove dashed connecting line for association class.
 */
void AssociationWidget::removeAssocClassLine()
{
    delete m_pAssocClassLineSel0;
    m_pAssocClassLineSel0 = 0;
    delete m_pAssocClassLineSel1;
    m_pAssocClassLineSel1 = 0;
    delete m_pAssocClassLine;
    m_pAssocClassLine = 0;
    if (m_associationClass) {
        m_associationClass->setClassAssociationWidget(0);
        m_associationClass = 0;
    }
}

/**
 * Creates the association class connecting line.
 */
void AssociationWidget::createAssocClassLine()
{
    if (m_pAssocClassLine == 0) {
        m_pAssocClassLine = new QGraphicsLineItem(this);
    }
    QPen pen(lineColor(), lineWidth(), Qt::DashLine);
    m_pAssocClassLine->setPen(pen);
    // decoration points
    m_pAssocClassLineSel0 = Widget_Utils::decoratePoint(m_pAssocClassLine->line().p1(),
                                                        m_pAssocClassLine);
    m_pAssocClassLineSel1 = Widget_Utils::decoratePoint(m_pAssocClassLine->line().p2(),
                                                        m_pAssocClassLine);
    computeAssocClassLine();
    selectAssocClassLine(false);
}

/**
 * Creates the association class connecting line using the specified
 * ClassifierWidget.
 *
 * @param classifier The ClassifierWidget to use.
 * @param linePathSegmentIndex The index of the segment where the
 *        association class is created.
 */
void AssociationWidget::createAssocClassLine(ClassifierWidget* classifier,
                                             int linePathSegmentIndex)
{
    m_nLinePathSegmentIndex = linePathSegmentIndex;

    if (m_nLinePathSegmentIndex < 0) {
        return;
    }

    m_associationClass = classifier;
    m_associationClass->setClassAssociationWidget(this);
    m_associationClass->addAssoc(this);  // to get widgetMoved(...) for association classes

    createAssocClassLine();
}

/**
 * Compute the end points of m_pAssocClassLine in case this
 * association has an attached association class.
 * TODO: The decoration points make no sense for now, because they are not movable.
 */
void AssociationWidget::computeAssocClassLine()
{
    if (m_associationClass == 0 || m_pAssocClassLine == 0) {
        return;
    }
    if (m_nLinePathSegmentIndex < 0) {
        uError() << "m_nLinePathSegmentIndex is not set";
        return;
    }

    QPointF segStart = m_associationLine.point(m_nLinePathSegmentIndex);
    QPointF segEnd = m_associationLine.point(m_nLinePathSegmentIndex + 1);
    const qreal midSegX = segStart.x() + (segEnd.x() - segStart.x()) / 2.0;
    const qreal midSegY = segStart.y() + (segEnd.y() - segStart.y()) / 2.0;
    QPointF segmentMidPoint(midSegX, midSegY);

    QLineF possibleAssocLine = QLineF(segmentMidPoint,
                                      m_associationClass->mapRectToScene(m_associationClass->rect()).center());
    QPointF intersectionPoint;
    QLineF::IntersectType type = intersect(m_associationClass->mapRectToScene(m_associationClass->boundingRect()),
                                           possibleAssocLine,
                                           &intersectionPoint);
    // DEBUG(DBG_SRC) << "intersect type=" << type << " / point=" << intersectionPoint;

    if (type == QLineF::BoundedIntersection) {
        m_pAssocClassLine->setLine(midSegX, midSegY,
                                   intersectionPoint.x(), intersectionPoint.y());

        if (m_pAssocClassLineSel0 && m_pAssocClassLineSel1) {
            m_pAssocClassLineSel0->setPos(m_pAssocClassLine->line().p1());
            m_pAssocClassLineSel1->setPos(m_pAssocClassLine->line().p2());
        }
    }
}

/**
 * Renders the association class connecting line selected.
 */
void AssociationWidget::selectAssocClassLine(bool sel)
{
    if (m_pAssocClassLineSel0 && m_pAssocClassLineSel1) {
        m_pAssocClassLineSel0->setVisible(sel);
        m_pAssocClassLineSel1->setVisible(sel);
    }
}

/**
 * Sets the association to be selected.
 */
void AssociationWidget::mousePressEvent(QGraphicsSceneMouseEvent * me)
{
    // clear other selected stuff on the screen of ShiftKey
    if (me->modifiers() != Qt::ShiftModifier) {
        m_scene->clearSelected();
    }

    if (me->button() == Qt::LeftButton && me->modifiers() == Qt::ControlModifier) {
        if (checkRemovePoint(me->scenePos()))
            return;
    }

    // make sure we should be here depending on the button
    if (me->button() != Qt::RightButton && me->button() != Qt::LeftButton) {
        return;
    }
    QPointF mep = me->scenePos();
    // see if `mep' is on the connecting line to the association class
    if (onAssocClassLine(mep)) {
        setSelected(true);
        selectAssocClassLine();
        return;
    }
    setSelected(!isSelected());
    m_associationLine.mousePressEvent(me);
}

/**
 * Displays the right mouse buttom menu if right button is pressed.
 */
void AssociationWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent * me)
{
    m_associationLine.mouseReleaseEvent(me);
}

/**
 * Handles the selection from the popup menu.
 */
void AssociationWidget::slotMenuSelection(QAction* action)
{
    QString oldText, newText;
    bool ok = false;
    Uml::AssociationType::Enum atype = associationType();
    Uml::RoleType::Enum r = RoleType::B;
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    DEBUG(DBG_SRC) << "menu selection = " << ListPopupMenu::toString(sel);

    // if it's a collaboration message we now just use the code in floatingtextwidget
    // this means there's some redundant code below but that's better than duplicated code
    if (isCollaboration() && sel != ListPopupMenu::mt_Delete) {
        m_nameWidget->slotMenuSelection(action);
        return;
    }

    switch(sel) {
    case ListPopupMenu::mt_Properties:
        if (atype == AssociationType::Seq_Message || atype == AssociationType::Seq_Message_Self) {
            // show op dlg for seq. diagram here
            // don't worry about here, I don't think it can get here as
            // line is widget on seq. diagram
            // here just in case - remove later after testing
            DEBUG(DBG_SRC) << "mt_Properties: assoctype is " << atype;
        } else {  //standard assoc dialog
            UMLApp::app()->docWindow()->updateDocumentation(false);
            showPropertiesDialog();
        }
        break;

    case ListPopupMenu::mt_Add_Point:
        checkAddPoint(m_eventScenePos);
        break;

    case ListPopupMenu::mt_Delete_Point:
        checkRemovePoint(m_eventScenePos);
        break;
        
    case ListPopupMenu::mt_Auto_Layout_Spline:
        checkAutoLayoutSpline();
        break;

    case ListPopupMenu::mt_Delete:
        if (!Dialog_Utils::askDeleteAssociation())
            break;
        if (m_pAssocClassLineSel0)
            removeAssocClassLine();
        else if (association())
            m_scene->removeAssocInViewAndDoc(this);
        else
            m_scene->removeWidgetCmd(this);
        break;

    case ListPopupMenu::mt_Rename_MultiA:
        r = RoleType::A;   // fall through
    case ListPopupMenu::mt_Rename_MultiB:
        if (m_role[r].multiplicityWidget)
            oldText = m_role[r].multiplicityWidget->text();
        else
            oldText = QString();
        newText = oldText;
        ok = Dialog_Utils::askName(i18n("Multiplicity"),
                                   i18n("Enter multiplicity:"),
                                   newText);
        if (ok && newText != oldText) {
            if (FloatingTextWidget::isTextValid(newText)) {
                setMultiplicity(newText, r);
            } else {
                m_scene->removeWidget(m_role[r].multiplicityWidget);
                m_role[r].multiplicityWidget = 0;
            }
        }
        break;

    case ListPopupMenu::mt_Rename_Name:
        if (m_nameWidget)
            oldText = m_nameWidget->text();
        else
            oldText = QString();
        newText = oldText;
        ok = Dialog_Utils::askName(i18n("Association Name"),
                                   i18n("Enter association name:"),
                                   newText);
        if (ok && newText != oldText) {
            if (FloatingTextWidget::isTextValid(newText)) {
                setName(newText);
            } else if (m_nameWidget) {
                m_scene->removeWidget(m_nameWidget);
                m_nameWidget = 0;
            }
        }
        break;

    case ListPopupMenu::mt_Rename_RoleAName:
        r = RoleType::A;   // fall through
    case ListPopupMenu::mt_Rename_RoleBName:
        if (m_role[r].roleWidget)
            oldText = m_role[r].roleWidget->text();
        else
            oldText = QString();
        newText = oldText;
        ok = Dialog_Utils::askName(i18n("Role Name"),
                                   i18n("Enter role name:"),
                                   newText);
        if (ok && newText != oldText) {
            if (FloatingTextWidget::isTextValid(newText)) {
                setRoleName(newText, r);
            } else {
                m_scene->removeWidget(m_role[r].roleWidget);
                m_role[r].roleWidget = 0;
            }
        }
        break;

    case ListPopupMenu::mt_Change_Font:
        {
#if QT_VERSION >= 0x050000
            bool ok = false;
            QFont fnt = QFontDialog::getFont(&ok, font(), m_scene->activeView());
            if (ok)
#else
            QFont fnt = font();
            if (KFontDialog::getFont(fnt, KFontChooser::NoDisplayFlags, m_scene->activeView()))
#endif
                lwSetFont(fnt);
        }
        break;

    case ListPopupMenu::mt_Line_Color:
        {
#if QT_VERSION >= 0x050000
            QColor newColor = QColorDialog::getColor(lineColor());
            if (newColor.isValid() && newColor != lineColor())
#else
            QColor newColor;
            if (KColorDialog::getColor(newColor))
#endif
            {
                m_scene->selectionSetLineColor(newColor);
                umlDoc()->setModified(true);
            }
        }
        break;

    case ListPopupMenu::mt_Cut:
        m_scene->setStartedCut();
        UMLApp::app()->slotEditCut();
        break;

    case ListPopupMenu::mt_Copy:
        UMLApp::app()->slotEditCopy();
        break;

    case ListPopupMenu::mt_Paste:
        UMLApp::app()->slotEditPaste();
        break;

    case ListPopupMenu::mt_Reset_Label_Positions:
        resetTextPositions();
        break;

    case ListPopupMenu::mt_LayoutDirect:
        m_associationLine.setLayout(Uml::LayoutType::Direct);
        break;
    case ListPopupMenu::mt_LayoutSpline:
        m_associationLine.setLayout(Uml::LayoutType::Spline);
        break;
    case ListPopupMenu::mt_LayoutOrthogonal:
        m_associationLine.setLayout(Uml::LayoutType::Orthogonal);
        break;
    case ListPopupMenu::mt_LayoutPolyline:
        m_associationLine.setLayout(Uml::LayoutType::Polyline);
        break;

    default:
        DEBUG(DBG_SRC) << "MenuType " << ListPopupMenu::toString(sel) << " not implemented";
        break;
    }//end switch
}

/**
 * Return the first font found being used by any child widget. (They
 * could be different fonts, so this is a slightly misleading method.)
 */
QFont AssociationWidget::font() const
{
    //:TODO: find a general font for the association
    QFont font;

    if (m_role[RoleType::A].roleWidget)
        font = m_role[RoleType::A].roleWidget->font();
    else    if (m_role[RoleType::B].roleWidget)
        font = m_role[RoleType::B].roleWidget->font();
    else    if (m_role[RoleType::A].multiplicityWidget)
        font = m_role[RoleType::A].multiplicityWidget->font();
    else    if (m_role[RoleType::B].multiplicityWidget)
        font = m_role[RoleType::B].multiplicityWidget->font();
    else    if (m_role[RoleType::A].changeabilityWidget)
        font = m_role[RoleType::A].changeabilityWidget->font();
    else    if (m_role[RoleType::B].changeabilityWidget)
        font = m_role[RoleType::B].changeabilityWidget->font();
    else    if (m_nameWidget)
        font = m_nameWidget->font();
    else
        font = m_role[RoleType::A].umlWidget->font();

    return font;
}

/**
 * Set all 'owned' child widgets to this text color.
 */
void AssociationWidget::setTextColor(const QColor &color)
{
    WidgetBase::setTextColor(color);
    if (m_nameWidget) {
        m_nameWidget->setTextColor(color);
    }
    if (m_role[RoleType::A].roleWidget) {
        m_role[RoleType::A].roleWidget->setTextColor(color);
    }
    if (m_role[RoleType::B].roleWidget) {
        m_role[RoleType::B].roleWidget->setTextColor(color);
    }
    if (m_role[RoleType::A].multiplicityWidget) {
        m_role[RoleType::A].multiplicityWidget->setTextColor(color);
    }
    if (m_role[RoleType::B].multiplicityWidget) {
        m_role[RoleType::B].multiplicityWidget->setTextColor(color);
    }
    if (m_role[RoleType::A].changeabilityWidget)
        m_role[RoleType::A].changeabilityWidget->setTextColor(color);
    if (m_role[RoleType::B].changeabilityWidget)
        m_role[RoleType::B].changeabilityWidget->setTextColor(color);
}

void AssociationWidget::setLineColor(const QColor &color)
{
    WidgetBase::setLineColor(color);
    QPen pen = m_associationLine.pen();
    pen.setColor(color);
    m_associationLine.setPen(pen);
    if (m_pAssocClassLine) {
        m_pAssocClassLine->setPen(pen);
    }
}

void AssociationWidget::setLineWidth(uint width)
{
    WidgetBase::setLineWidth(width);
    QPen pen = m_associationLine.pen();
    pen.setWidth(width);
    m_associationLine.setPen(pen);
    if (m_pAssocClassLine) {
        m_pAssocClassLine->setPen(pen);
    }
}

bool AssociationWidget::checkAddPoint(const QPointF &scenePos)
{
    if (associationType() == AssociationType::Exception) {
        return false;
    }

    // if there is no point around the mouse pointer, we insert a new one
    if (m_associationLine.closestPointIndex(scenePos) < 0) {
        int i = m_associationLine.closestSegmentIndex(scenePos);
        if (i < 0) {
            DEBUG(DBG_SRC) << "no closest segment found!";
            return false;
        }
        // switch type to see additional points by default
        if (m_associationLine.count() == 2)
            m_associationLine.setLayout(Uml::LayoutType::Polyline);
        m_associationLine.insertPoint(i + 1, scenePos);
        if (m_nLinePathSegmentIndex == i) {
            QPointF segStart = m_associationLine.point(i);
            QPointF segEnd = m_associationLine.point(i + 2);
            const int midSegX = segStart.x() + (segEnd.x() - segStart.x()) / 2;
            const int midSegY = segStart.y() + (segEnd.y() - segStart.y()) / 2;
            /*
            DEBUG(DBG_SRC) << "segStart=" << segStart << ", segEnd=" << segEnd
                           << ", midSeg=(" << midSegX << "," << midSegY
                           << "), mp=" << mp;
             */
            if (midSegX > scenePos.x() || midSegY < scenePos.y()) {
                m_nLinePathSegmentIndex++;
                DEBUG(DBG_SRC) << "setting m_nLinePathSegmentIndex to "
                               << m_nLinePathSegmentIndex;
                computeAssocClassLine();
            }
            m_associationLine.update();
            calculateNameTextSegment();
            umlDoc()->setModified(true);
            setSelected(true);
        }
        return true;
    }
    else {
        DEBUG(DBG_SRC) << "found point already close enough!";
        return false;
    }
}

/**
 * Remove point close to the given point and redraw the association.
 * @param scenePos   point which should be removed
 * @return   success status of the remove action
 */
bool AssociationWidget::checkRemovePoint(const QPointF &scenePos)
{
    int i = m_associationLine.closestPointIndex(scenePos);
    if (i == -1)
        return false;

    m_associationLine.setSelected(false);

    // there was a point so we remove the point
    m_associationLine.removePoint(i);

    // switch type back to simple line
    if (m_associationLine.count() == 2)
        m_associationLine.setLayout(Uml::LayoutType::Direct);

    // Maybe reattach association class connecting line
    // to different association linepath segment.
    const int numberOfLines = m_associationLine.count() - 1;
    if (m_nLinePathSegmentIndex >= numberOfLines) {
        m_nLinePathSegmentIndex = numberOfLines - 1;
    }
    calculateEndingPoints();

    // select the line path
    m_associationLine.setSelected(true);

    m_associationLine.update();

    calculateNameTextSegment();
    umlDoc()->setModified(true);
    return true;
}

bool AssociationWidget::checkAutoLayoutSpline() {
    m_associationLine.enableAutoLayout();
    m_associationLine.update();
    return true;
}

/**
 * Moves the break point being dragged.
 */
void AssociationWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
    if (me->buttons() != Qt::LeftButton) {
        return;
    }

    setSelected(true);

    m_associationLine.mouseMoveEvent(me);
    moveEvent(me);
    m_scene->resizeSceneToItems();
}

/**
 * Returns the Region the widget to line intersection is for the given
 * widget in this Association.  If the given widget is not in the
 * Association then Region::Error is returned.
 * Used by @ref calculateEndingPoints to work these positions out for
 * another Association - since the number of Associations on the same
 * region for the same widget will mean the lines will need to be
 * spread out across the region.
 */
//Uml::Region::Enum AssociationWidget::getWidgetRegion(AssociationWidget * widget) const
//{
//    if (widget->widgetForRole(RoleType::A) == m_role[RoleType::A].umlWidget)
//        return m_role[RoleType::A].m_WidgetRegion;
//    if (widget->widgetForRole(RoleType::B) == m_role[RoleType::B].umlWidget)
//        return m_role[RoleType::B].m_WidgetRegion;
//    return Uml::Region::Error;
//}

/**
 * Returns the number of lines there are on the given region for
 * either widget A or B of the association.
 */
int AssociationWidget::getRegionCount(Uml::Region::Enum region, Uml::RoleType::Enum role)
{
    if ((region == Uml::Region::Error) | (umlScene() == 0)) {
        return 0;
    }
    int widgetCount = 0;
    AssociationWidgetList list = m_scene->associationList();
    foreach (AssociationWidget* assocwidget, list) {
        //don't count this association
        if (assocwidget == this)
            continue;
        const AssociationWidgetRole& otherA = assocwidget->m_role[RoleType::A];
        const AssociationWidgetRole& otherB = assocwidget->m_role[RoleType::B];
        const UMLWidget *a = otherA.umlWidget;
        const UMLWidget *b = otherB.umlWidget;
        /*
        //don't count associations to self if both of their end points are on the same region
        //they are different and placement won't interfere with them
        if (a == b && otherA.m_WidgetRegion == otherB.m_WidgetRegion)
                continue;
         */
        if (m_role[role].umlWidget == a && region == otherA.m_WidgetRegion)
            widgetCount++;
        else if (m_role[role].umlWidget == b && region == otherB.m_WidgetRegion)
            widgetCount++;
    }//end foreach
    return widgetCount;
}

/**
 * Find the border point of the given rect when a line is drawn from the
 * given point to the rect.
 * @param rect   rect of a classifier
 * @param line   a line to the rect
 * @param intersectionPoint   the intercept point on the border of the rect
 * @return   the type of the intersection @ref QLineF::IntersectType
 */
QLineF::IntersectType AssociationWidget::intersect(const QRectF &rect, const QLineF &line,
                                                   QPointF* intersectionPoint)
{
    QList<QLineF> lines;
    lines << QLineF(rect.topLeft(), rect.topRight());
    lines << QLineF(rect.topRight(), rect.bottomRight());
    lines << QLineF(rect.bottomRight(), rect.bottomLeft());
    lines << QLineF(rect.bottomLeft(), rect.topLeft());
    foreach (const QLineF& rectLine, lines) {
        QLineF::IntersectType type = rectLine.intersect(line, intersectionPoint);
        if (type == QLineF::BoundedIntersection) {
            return type;
        }
    }
    return QLineF::NoIntersection;
}

/**
 * Given a rectangle and a point, findInterceptOnEdge computes the
 * connecting line between the middle point of the rectangle and
 * the point, and returns the intercept of this line with the
 * edge of the rectangle identified by `region'.
 * When the region is North or South, the X value is returned (Y is
 * constant.)
 * When the region is East or West, the Y value is returned (X is
 * constant.)
 * @todo This is buggy. Try replacing by intersect()
 */
qreal AssociationWidget::findInterceptOnEdge(const QRectF &rect,
                                             Uml::Region::Enum region,
                                             const QPointF &point)
{
    uDebug() << "AssociationWidget::findInterceptOnEdge : rect.x =" << rect.x() << ", rect.y =" << rect.y();
    // The Qt coordinate system has (0, 0) in the top left corner.
    // In order to go to the regular XY coordinate system with (0, 0)
    // in the bottom left corner, we swap the X and Y axis.
    // That's why the following assignments look twisted.
    const qreal rectHalfWidth = rect.height() / 2.0;
    const qreal rectHalfHeight = rect.width() / 2.0;
    const qreal rectMidX = rect.y() + rectHalfWidth;
    const qreal rectMidY = rect.x() + rectHalfHeight;
    const qreal dX = rectMidX - point.y();
    const qreal dY = rectMidY - point.x();
    switch (region) {
    case Uml::Region::West:
        region = Uml::Region::South;
        break;
    case Uml::Region::North:
        region = Uml::Region::West;
        break;
    case Uml::Region::East:
        region = Uml::Region::North;
        break;
    case Uml::Region::South:
        region = Uml::Region::East;
        break;
    default:
        break;
    }
    // Now we have regular coordinates with the point (0, 0) in the
    // bottom left corner.
    if (region == Uml::Region::North || region == Uml::Region::South) {
        if (dX == 0)
            return rectMidY;
        // should be rectMidX, but we go back to Qt coord.sys.
        if (dY == 0) {
            uError() << "usage error: " << "North/South (dY == 0)";
            return -1.0;
        }
        const qreal m = dY / dX;
        qreal relativeX;
        if (region == Uml::Region::North)
            relativeX = rectHalfHeight / m;
        else
            relativeX = -rectHalfHeight / m;
        return (rectMidY + relativeX);
        // should be rectMidX, but we go back to Qt coord.sys.
    } else {
        if (dY == 0)
            return rectMidX;
        // should be rectMidY, but we go back to Qt coord.sys.
        if (dX == 0) {
            uError() << "usage error: " << "East/West (dX == 0)";
            return -1.0;
        }
        const qreal m = dY / dX;
        qreal relativeY = m * rectHalfWidth;
        if (region == Uml::Region::West)
            relativeY = -relativeY;
        return (rectMidX + relativeY);
        // should be rectMidY, but we go back to Qt coord.sys.
    }
}

/**
 * Auxiliary method for updateAssociations():
 * Put position into m_positions and assoc into m_ordered at the
 * correct index.
 * m_positions and m_ordered move in parallel and are sorted by
 * ascending position.
 */
void AssociationWidget::insertIntoLists(qreal position, const AssociationWidget* assoc)
{
    bool did_insertion = false;
    for (int index = 0; index < m_positions_len; ++index) {
        if (position < m_positions[index]) {
            for (int moveback = m_positions_len; moveback > index; moveback--)
                m_positions[moveback] = m_positions[moveback - 1];
            m_positions[index] = position;
            m_ordered.insert(index, const_cast<AssociationWidget*>(assoc));
            did_insertion = true;
            break;
        }
    }
    if (! did_insertion) {
        m_positions[m_positions_len] = position;
        m_ordered.append(const_cast<AssociationWidget*>(assoc));
    }
    m_positions_len++;
}

/**
 * Tells all the other view associations the new count for the
 * given widget on a certain region. And also what index they should be.
 */
void AssociationWidget::updateAssociations(int totalCount,
                                           Uml::Region::Enum region,
                                           Uml::RoleType::Enum role)
{
    if ((region == Uml::Region::Error) | (umlScene() == 0)) {
        return;
    }
    AssociationWidgetList list = m_scene->associationList();

    UMLWidget *ownWidget = m_role[role].umlWidget;
    m_positions_len = 0;
    m_ordered.clear();
    // we order the AssociationWidget list by region and x/y value
    foreach (AssociationWidget* assocwidget, list) {
        AssociationWidgetRole *roleA = &assocwidget->m_role[RoleType::A];
        AssociationWidgetRole *roleB = &assocwidget->m_role[RoleType::B];
        UMLWidget *wA = roleA->umlWidget;
        UMLWidget *wB = roleB->umlWidget;
        // Skip self associations.
        if (wA == wB)
            continue;
        // Now we must find out with which end the assocwidget connects
        // to the input widget (ownWidget).
        bool inWidgetARegion = (ownWidget == wA &&
                                 region == roleA->m_WidgetRegion);
        bool inWidgetBRegion = (ownWidget == wB &&
                                 region == roleB->m_WidgetRegion);
        if (!inWidgetARegion && !inWidgetBRegion)
            continue;
        // Determine intercept position on the edge indicated by `region'.
        UMLWidget * otherWidget = (inWidgetARegion ? wB : wA);
        const AssociationLine& linepath = assocwidget->associationLine();
        QPointF refpoint;
        if (assocwidget->linePathStartsAt(otherWidget))
            refpoint = linepath.point(linepath.count() - 2);
        else
            refpoint = linepath.point(1);
        // The point is authoritative if we're called for the second time
        // (i.e. role==B) or it is a waypoint on the line path.
        bool pointIsAuthoritative = (role == RoleType::B || linepath.count() > 2);
        if (! pointIsAuthoritative) {
            // If the point is not authoritative then we use the other
            // widget's center.
            refpoint.setX(otherWidget->scenePos().x() + otherWidget->width() / 2);
            refpoint.setY(otherWidget->scenePos().y() + otherWidget->height() / 2);
        }
        QRectF rect = ownWidget->rect();
        rect.setX(ownWidget->scenePos().x());
        rect.setY(ownWidget->scenePos().y());
        qreal intercept = findInterceptOnEdge(rect, region, refpoint);
        if (intercept < 0) {
            DEBUG(DBG_SRC) << "error from findInterceptOnEdge for"
                           << " assocType=" << assocwidget->associationType()
                           << " ownWidget=" << ownWidget->name()
                           << " otherWidget=" << otherWidget->name();
            continue;
        }
        insertIntoLists(intercept, assocwidget);
    } // while ((assocwidget = assoc_it.current()))

    // we now have an ordered list and we only have to call updateRegionLineCount
    int index = 1;
    foreach (AssociationWidget* assocwidget, m_ordered ) {
        if (ownWidget == assocwidget->widgetForRole(RoleType::A)) {
            assocwidget->updateRegionLineCount(index++, totalCount, region, RoleType::A);
        } else if (ownWidget == assocwidget->widgetForRole(RoleType::B)) {
            assocwidget->updateRegionLineCount(index++, totalCount, region, RoleType::B);
        }
    } // for (assocwidget = ordered.first(); ...)
}

/**
 * Called to tell the association that another association has added
 * a line to the region of one of its widgets. The widget is identified
 * by its role (A or B).
 *
 * Called by @ref updateAssociations which is called by
 * @ref calculateEndingPoints when required.
 */
void AssociationWidget::updateRegionLineCount(int index, int totalCount,
                                              Uml::Region::Enum region,
                                              Uml::RoleType::Enum role)
{
    if ((region == Uml::Region::Error) | (umlScene() == 0)) {
        return;
    }
    // If the association is to self and the line ends are on the same region then
    // use a different calculation.
    if (isSelf() &&
            m_role[RoleType::A].m_WidgetRegion == m_role[RoleType::B].m_WidgetRegion) {
        UMLWidget * pWidget = m_role[RoleType::A].umlWidget;
        qreal x = pWidget->scenePos().x();
        qreal y = pWidget->scenePos().y();
        qreal wh = pWidget->height();
        qreal ww = pWidget->width();
        int size = m_associationLine.count();
        // See if above widget ok to place assoc.
        switch( m_role[RoleType::A].m_WidgetRegion ) {
        case Uml::Region::North:
            m_associationLine.setPoint( 0, QPointF( x + ( ww / 4 ), y ) );
            m_associationLine.setPoint( size - 1, QPointF(x + ( ww * 3 / 4 ), y ) );
            break;

        case Uml::Region::South:
            m_associationLine.setPoint( 0, QPointF( x + ( ww / 4 ), y + wh ) );
            m_associationLine.setPoint( size - 1, QPointF( x + ( ww * 3 / 4 ), y + wh ) );
            break;

        case Uml::Region::East:
            m_associationLine.setPoint( 0, QPointF( x + ww, y + ( wh / 4 ) ) );
            m_associationLine.setPoint( size - 1, QPointF( x + ww, y + ( wh * 3 / 4 ) ) );
            break;

        case Uml::Region::West:
            m_associationLine.setPoint( 0, QPointF( x, y + ( wh / 4 ) ) );
            m_associationLine.setPoint( size - 1, QPointF( x, y + ( wh * 3 / 4 ) ) );
            break;
        default:
            break;
        }//end switch

        return;
    }

    AssociationWidgetRole& robj = m_role[role];
    UMLWidget * pWidget = robj.umlWidget;

    robj.m_nIndex = index;
    robj.m_nTotalCount = totalCount;
    qreal x = pWidget->scenePos().x();
    qreal y = pWidget->scenePos().y();
    qreal ww = pWidget->width();
    qreal wh = pWidget->height();
    const bool angular = Settings::optionState().generalState.angularlines;
    qreal ch = 0;
    qreal cw = 0;
    if (angular) {
        uint nind = (role == RoleType::A ? 1 : m_associationLine.count() - 2);
        QPointF neighbour = m_associationLine.point(nind);
        if (neighbour.x() < x)
            cw = 0;
        else if (neighbour.x() > x + ww)
            cw = 0 + ww;
        else
            cw = neighbour.x() - x;
        if (neighbour.y() < y)
            ch = 0;
        else if (neighbour.y() > y + wh)
            ch = 0 + wh;
        else
            ch = neighbour.y() - y;
    } else {
        ch = wh * index / totalCount;
        cw = ww * index / totalCount;
    }

    qreal newX = x + cw;
    qreal newY = y + ch;

    QPointF pt;
    if (angular) {
        pt = QPointF(newX, newY);
    } else {
        UMLWidget *pWidgetA = m_role[RoleType::A].umlWidget;
        UMLWidget *pWidgetB = m_role[RoleType::B].umlWidget;
        QList<QPolygonF> polyListA = pWidgetA->shape().toSubpathPolygons();
        QPolygonF polyA = polyListA.at(0);
        if (polyListA.size() > 1) {
            for (int i = 1; i < polyListA.size(); i++) {
                 polyA = polyA.united(polyListA.at(i));
            }
        }
        polyA = polyA.translated(pWidgetA->getPos());
        QList<QPolygonF> polyListB = pWidgetB->shape().toSubpathPolygons();
        QPolygonF polyB = polyListB.at(0);
        if (polyListB.size() > 1) {
            for (int i = 1; i < polyListB.size(); i++) {
                 polyB = polyB.united(polyListB.at(i));
            }
        }
        polyB = polyB.translated(pWidgetB->getPos());
        QLineF nearestPoints = Widget_Utils::closestPoints(polyA, polyB);
        if (nearestPoints.isNull()) {
            uError() << "Widget_Utils::closestPoints failed, falling back to simple widget positions";
            switch(region) {
                case Uml::Region::West:
                    pt.setX(x);
                    pt.setY(newY);
                    break;
                case Uml::Region::North:
                    pt.setX(newX);
                    pt.setY(y);
                    break;
                case Uml::Region::East:
                    pt.setX(x + ww);
                    pt.setY(newY);
                    break;
                case Uml::Region::South:
                    pt.setX(newX);
                    pt.setY(y + wh);
                    break;
                case Uml::Region::Center:
                    pt.setX(x + ww / 2);
                    pt.setY(y + wh / 2);
                    break;
                default:
                    break;
            }
        } else {
            if (role == RoleType::A)
                pt = nearestPoints.p1();
            else
                pt = nearestPoints.p2();
        }
    }
    if (role == RoleType::A) {
        m_associationLine.setPoint(0, pt);
    }
    else {
        m_associationLine.setPoint(m_associationLine.count() - 1, pt);
    }
}

/**
 * Sets the state of whether the widget is selected.
 *
 * @param _select   The state of whether the widget is selected.
 */
void AssociationWidget::setSelected(bool _select /* = true */)
{
    WidgetBase::setSelected(_select);
    if ( m_nameWidget)
        m_nameWidget->setSelected( _select );

    m_role[RoleType::A].setSelected(_select);
    m_role[RoleType::B].setSelected(_select);

    // Update the docwindow for this association.
    // This is done last because each of the above setSelected calls
    // overwrites the docwindow, but we want the main association doc
    // to win.
    if (_select) {
        UMLApp::app()->docWindow()->showDocumentation(this, false);
    } else
        UMLApp::app()->docWindow()->updateDocumentation(true);

    m_associationLine.setSelected(_select);
    if (! _select) {
        // For now, if _select is true we don't make the assoc class line
        // selected. But that's certainly open for discussion.
        // At any rate, we need to deselect the assoc class line
        // if _select is false.
        selectAssocClassLine(false);
    }
    UMLApp::app()->document()->writeToStatusBar(_select ? i18n("Press Ctrl with left mouse click to delete a point") : QString());
}

/**
 * Reimplement method from WidgetBase in order to check owned floating texts.
 *
 * @param p Point to be checked.
 *
 * @return pointer to widget at the provided point
 * @return 0 is no widget has been found
 */
UMLWidget* AssociationWidget::onWidget(const QPointF &p)
{
    if (m_nameWidget && m_nameWidget->onWidget(p))
        return m_nameWidget;

    UMLWidget *w = m_role[RoleType::A].onWidget(p);
    if (w)
        return w;

    w = m_role[RoleType::B].onWidget(p);
    if (w)
        return w;

    return nullptr;
}

/**
 * Returns true if the given point is on the connecting line to
 * the association class. Returns false if there is no association
 * class attached, or if the given point is not on the connecting
 * line.
 */
bool AssociationWidget::onAssocClassLine(const QPointF &point)
{
    bool onLine = false;
    if (m_pAssocClassLine) {
//:TODO:
//        const QPointF mapped = m_pAssocClassLine->mapFromParent(point);
//        bool onLine = m_pAssocClassLine->contains(mapped);
//        return onLine;
        UMLSceneItemList list = m_scene->collisions(point);
        UMLSceneItemList::iterator end(list.end());
        for (UMLSceneItemList::iterator item_it(list.begin()); item_it != end; ++item_it) {
            if (*item_it == m_pAssocClassLine) {
                onLine = true;
                break;
            }
        }
    }
    DEBUG(DBG_SRC) << onLine;
    return onLine;
}

/**
 * Returns true if the given point is on the association line.
 * A circle (rectangle) around the point is used to obtain more tolerance.
 * @param point   the point to check
 * @return   flag whether point is on association line
 */
bool AssociationWidget::onAssociation(const QPointF& point)
{
    // check the path
    const qreal diameter(4.0);
    QPainterPath path = m_associationLine.shape();
    if (path.contains(point)) {
        DEBUG(DBG_SRC) << "on path";
        return true;
    }
    // check also the points
    if (m_associationLine.layout() == Uml::LayoutType::Spline) {
        if (m_associationLine.closestPointIndex(point, diameter) > -1) {
            DEBUG(DBG_SRC) << "on spline point";
            return true;
        }
    }
    return onAssocClassLine(point);
}

/**
 * Set all association points to x coordinate.
 */
void AssociationWidget::setXEntireAssoc(qreal x)
{
    for (int i = 0; i < m_associationLine.count(); ++i) {
        QPointF p = m_associationLine.point(i);
        p.setX(x);
        m_associationLine.setPoint(i, p);
    }
}

/**
 * Set all association points to y coordinate.
 */
void AssociationWidget::setYEntireAssoc(qreal y)
{
    for (int i = 0; i < m_associationLine.count(); ++i) {
        QPointF p = m_associationLine.point(i);
        p.setY(y);
        m_associationLine.setPoint(i, p);
    }
}

/**
 * Moves all the mid points (all except start /end) by the given amount.
 */
void AssociationWidget::moveMidPointsBy(qreal x, qreal y)
{
    int pos = m_associationLine.count() - 1;
    for (int i = 1; i < (int)pos; ++i) {
        QPointF p = m_associationLine.point( i );
        qreal newX = p.x() + x;
        qreal newY = p.y() + y;
        p.setX( newX );
        p.setY( newY );
        m_associationLine.setPoint( i, p );
    }
}

/**
 * Moves the entire association by the given offset.
 */
void AssociationWidget::moveEntireAssoc(qreal x, qreal y)
{
    //TODO: ADD SUPPORT FOR ASSOC. ON SEQ. DIAGRAMS WHEN NOTES BACK IN.
    moveMidPointsBy(x, y);
    // multi select
    if (umlScene()->selectedCount() > 1) {
        QPointF d(x, y);
        QPointF s = m_associationLine.startPoint() + d;
        QPointF e = m_associationLine.endPoint() + d;
        m_associationLine.setEndPoints(s, e);
    }
    calculateEndingPoints();
    calculateNameTextSegment();
    resetTextPositions();
}

/**
 * Returns the bounding rectangle of all segments of the association.
 */
QRectF AssociationWidget::boundingRect() const
{
    return m_associationLine.boundingRect();
}

/**
 * Returns the shape of all segments of the association.
 */
QPainterPath AssociationWidget::shape() const
{
    return m_associationLine.shape();
}

/**
 * Connected to UMLClassifier::attributeRemoved() or UMLEntity::constraintRemoved()
 * in case this AssociationWidget is linked to a classifier list item
 * (an attribute or a foreign key constraint)
 *
 * @param obj   The UMLClassifierListItem removed.
 */
void AssociationWidget::slotClassifierListItemRemoved(UMLClassifierListItem* obj)
{
    if (obj != m_umlObject) {
        DEBUG(DBG_SRC) << "obj=" << obj << ": m_umlObject=" << m_umlObject;
        return;
    }
    m_umlObject = 0;
    m_scene->removeWidgetCmd(this);
}

/**
 * Connected to UMLObject::modified() in case this
 * AssociationWidget is linked to a classifer's attribute type.
 */
void AssociationWidget::slotAttributeChanged()
{
    UMLAttribute *attr = attribute();
    if (attr == 0) {
        uError() << "attribute() returns NULL";
        return;
    }
    setVisibility(attr->visibility(), RoleType::B);
    setRoleName(attr->name(), RoleType::B);
}

void AssociationWidget::clipSize()
{
    if (m_nameWidget)
        m_nameWidget->clipSize();

    m_role[RoleType::A].clipSize();
    m_role[RoleType::B].clipSize();

    if (m_associationClass)
        m_associationClass->clipSize();
}

/**
 * Event handler for context menu events, called from the line segments.
 */
void AssociationWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();

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
    m_eventScenePos = event->scenePos();

    const Uml::AssociationType::Enum type = onAssocClassLine(event->scenePos()) ? Uml::AssociationType::Anchor : associationType();
    AssociationWidgetPopupMenu popup(parent, type, this);
    QAction *triggered = popup.exec(event->screenPos());
    slotMenuSelection(triggered);
}

/**
 * Reimplemented event handler for hover enter events.
 */
void AssociationWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_associationLine.hoverEnterEvent(event);
}

/**
 * Reimplemented event handler for hover leave events.
 */
void AssociationWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_associationLine.hoverLeaveEvent(event);
}

/**
 * Reimplemented event handler for hover move events.
 */
void AssociationWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    m_associationLine.hoverMoveEvent(event);
}

/**
 * Saves this widget to the "assocwidget" XMI element.
 */
void AssociationWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("assocwidget"));

    WidgetBase::saveToXMI(writer);
    LinkWidget::saveToXMI(writer);
    if (m_umlObject) {
        writer.writeAttribute(QLatin1String("xmi.id"), Uml::ID::toString(m_umlObject->id()));
    }
    writer.writeAttribute(QLatin1String("type"), QString::number(associationType()));
    if (!association()) {
        writer.writeAttribute(QLatin1String("visibilityA"), QString::number(visibility(RoleType::A)));
        writer.writeAttribute(QLatin1String("visibilityB"), QString::number(visibility(RoleType::B)));
        writer.writeAttribute(QLatin1String("changeabilityA"), QString::number(changeability(RoleType::A)));
        writer.writeAttribute(QLatin1String("changeabilityB"), QString::number(changeability(RoleType::B)));
        if (m_umlObject == 0) {
            writer.writeAttribute(QLatin1String("roleAdoc"), roleDocumentation(RoleType::A));
            writer.writeAttribute(QLatin1String("roleBdoc"), roleDocumentation(RoleType::B));
            writer.writeAttribute(QLatin1String("documentation"), documentation());
        }
    }
    writer.writeAttribute(QLatin1String("widgetaid"), Uml::ID::toString(widgetIDForRole(RoleType::A)));
    writer.writeAttribute(QLatin1String("widgetbid"), Uml::ID::toString(widgetIDForRole(RoleType::B)));

    if (m_associationClass) {
        QString acid = Uml::ID::toString(m_associationClass->id());
        writer.writeAttribute(QLatin1String("assocclass"), acid);
        writer.writeAttribute(QLatin1String("aclsegindex"), QString::number(m_nLinePathSegmentIndex));
    }

    // save attributes of m_role[A]
    const AssociationWidgetRole& roleA = m_role[RoleType::A];
    writer.writeAttribute(QLatin1String("indexa"), QString::number(roleA.m_nIndex));
    writer.writeAttribute(QLatin1String("totalcounta"), QString::number(roleA.m_nTotalCount));
    // save attributes of m_role[B]
    const AssociationWidgetRole& roleB = m_role[RoleType::B];
    writer.writeAttribute(QLatin1String("indexb"), QString::number(roleB.m_nIndex));
    writer.writeAttribute(QLatin1String("totalcountb"), QString::number(roleB.m_nTotalCount));

    // Save subelements of m_role[A] and m_role[B].
    m_role[RoleType::A].saveToXMI(writer);
    // This is separated from attributes because attributes may not follow
    // elements, i.e. all attributes must be written before the first subelement.
    m_role[RoleType::B].saveToXMI(writer);

    if (m_nameWidget) {
        m_nameWidget->saveToXMI(writer);
    }

    m_associationLine.saveToXMI(writer);

    writer.writeEndElement();  // assocwidget
}

/**
 * Uses the supplied widgetList for resolving
 * the role A and role B widgets. (The other loadFromXMI() queries
 * the UMLScene for these widgets.)
 * Required for clipboard operations.
 */
bool AssociationWidget::loadFromXMI(QDomElement& qElement,
                                    const UMLWidgetList& widgets,
                                    const MessageWidgetList* messages)
{
    if (!WidgetBase::loadFromXMI(qElement)) {
        return false;
    }

    if (!LinkWidget::loadFromXMI(qElement)) {
        return false;
    }

    // load child widgets first
    QString widgetaid = qElement.attribute(QLatin1String("widgetaid"), QLatin1String("-1"));
    QString widgetbid = qElement.attribute(QLatin1String("widgetbid"), QLatin1String("-1"));
    Uml::ID::Type aId = Uml::ID::fromString(widgetaid);
    Uml::ID::Type bId = Uml::ID::fromString(widgetbid);
    UMLWidget *pWidgetA = Widget_Utils::findWidget(aId, widgets, messages);
    if (!pWidgetA) {
        uError() << "cannot find widget for roleA id " << Uml::ID::toString(aId);
        return false;
    }
    UMLWidget *pWidgetB = Widget_Utils::findWidget(bId, widgets, messages);
    if (!pWidgetB) {
        uError() << "cannot find widget for roleB id " << Uml::ID::toString(bId);
        return false;
    }
    setWidgetForRole(pWidgetA, RoleType::A);
    setWidgetForRole(pWidgetB, RoleType::B);

    QString type = qElement.attribute(QLatin1String("type"), QLatin1String("-1"));
    Uml::AssociationType::Enum aType = Uml::AssociationType::fromInt(type.toInt());

    bool oldStyleLoad = false;
    if (m_nId == Uml::ID::None) {
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
            if (!m_umlObject && umlRoleA && umlRoleB) {
                oldStyleLoad = true; // flag for further special config below
                if (aType == AssociationType::Aggregation || aType == AssociationType::Composition) {
                    uWarning()<<" Old Style save file? swapping roles on association widget"<<this;
                    // We have to swap the A and B widgets to compensate
                    // for the long standing bug in AssociationLine of drawing
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
                    setWidgetForRole(pWidgetA, RoleType::A);
                    setWidgetForRole(pWidgetB, RoleType::B);
                    umlRoleA = pWidgetA->umlObject();
                    umlRoleB = pWidgetB->umlObject();
                }

                setUMLAssociation(umlDoc()->createUMLAssociation(umlRoleA, umlRoleB, aType));
            }
        }

        setDocumentation(qElement.attribute(QLatin1String("documentation")));
        setRoleDocumentation(qElement.attribute(QLatin1String("roleAdoc")), RoleType::A);
        setRoleDocumentation(qElement.attribute(QLatin1String("roleBdoc")), RoleType::B);

        // visibility defaults to Public if it cant set it here..
        QString visibilityA = qElement.attribute(QLatin1String("visibilityA"), QLatin1String("0"));
        int vis = visibilityA.toInt();
        if (vis >= 200) {  // bkwd compat.
            vis -= 200;
        }
        setVisibility((Uml::Visibility::Enum)vis, RoleType::A);

        QString visibilityB = qElement.attribute(QLatin1String("visibilityB"), QLatin1String("0"));
        vis = visibilityB.toInt();
        if (vis >= 200) { // bkwd compat.
            vis -= 200;
        }
        setVisibility((Uml::Visibility::Enum)vis, RoleType::B);

        // Changeability defaults to "Changeable" if it cant set it here..
        QString changeabilityA = qElement.attribute(QLatin1String("changeabilityA"), QLatin1String("0"));
        if (changeabilityA.toInt() > 0)
            setChangeability(Uml::Changeability::fromInt(changeabilityA.toInt()), RoleType::A);

        QString changeabilityB = qElement.attribute(QLatin1String("changeabilityB"), QLatin1String("0"));
        if (changeabilityB.toInt() > 0)
            setChangeability(Uml::Changeability::fromInt(changeabilityB.toInt()), RoleType::B);

    } else {

        // we should disconnect any prior association (can this happen??)
        if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association) {
            UMLAssociation *umla = association();
            umla->disconnect(this);
            umla->nrof_parent_widgets--;
        }

        // New style: The xmi.id is a reference to the UMLAssociation.
        // If the UMLObject is not found right now, we try again later
        // during the type resolution pass - see activate().
        UMLObject *myObj = umlDoc()->findObjectById(m_nId);
        if (myObj) {
            const UMLObject::ObjectType ot = myObj->baseType();
            if (ot != UMLObject::ot_Association) {
                setUMLObject(myObj);
            } else {
                UMLAssociation * myAssoc = myObj->asUMLAssociation();
                setUMLAssociation(myAssoc);
                if (type == QLatin1String("-1"))
                    aType = myAssoc->getAssocType();
            }
        }
    }

    setAssociationType(aType);

    m_role[RoleType::A].loadFromXMI(qElement, QLatin1String("a"));
    m_role[RoleType::B].loadFromXMI(qElement, QLatin1String("b"));

    QString assocclassid = qElement.attribute(QLatin1String("assocclass"));
    if (! assocclassid.isEmpty()) {
        Uml::ID::Type acid = Uml::ID::fromString(assocclassid);
        UMLWidget *w = Widget_Utils::findWidget(acid, widgets);
        if (w) {
            ClassifierWidget* aclWidget = static_cast<ClassifierWidget*>(w);
            QString aclSegIndex = qElement.attribute(QLatin1String("aclsegindex"), QLatin1String("0"));
            createAssocClassLine(aclWidget, aclSegIndex.toInt());
        } else {
            uError() << "cannot find assocclass " << assocclassid;
        }
    }

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    while (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QLatin1String("linepath")) {
            if (!m_associationLine.loadFromXMI(element)) {
                return false;
            }
        } else if (tag == QLatin1String("floatingtext") ||
                   tag == QLatin1String("UML:FloatingTextWidget")) {  // for bkwd compatibility
            QString r = element.attribute(QLatin1String("role"), QLatin1String("-1"));
            if (r == QLatin1String("-1"))
                return false;
            Uml::TextRole::Enum role = Uml::TextRole::fromInt(r.toInt());
            FloatingTextWidget *ft = new FloatingTextWidget(m_scene, role, QString(), Uml::ID::Reserved);
            if (! ft->loadFromXMI(element)) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete ft;
                node = element.nextSibling();
                element = node.toElement();
                continue;
            }
            // always need this
            ft->setParentItem(this);
            ft->setLink(this);
            ft->setSequenceNumber(m_SequenceNumber);
            ft->setFontCmd(ft->font());

            switch(role) {
            case Uml::TextRole::MultiA:
                m_role[RoleType::A].multiplicityWidget = ft;
                if (oldStyleLoad)
                    setMultiplicity(m_role[RoleType::A].multiplicityWidget->text(), RoleType::A);
                break;

            case Uml::TextRole::MultiB:
                m_role[RoleType::B].multiplicityWidget = ft;
                if (oldStyleLoad)
                    setMultiplicity(m_role[RoleType::B].multiplicityWidget->text(), RoleType::B);
                break;

            case Uml::TextRole::ChangeA:
                m_role[RoleType::A].changeabilityWidget = ft;
                break;

            case Uml::TextRole::ChangeB:
                m_role[RoleType::B].changeabilityWidget = ft;
                break;

            case Uml::TextRole::Name:
                m_nameWidget = ft;
                if (oldStyleLoad)
                    setName(m_nameWidget->text());
                break;

            case Uml::TextRole::Coll_Message:
            case Uml::TextRole::Coll_Message_Self:
                m_nameWidget = ft;
                ft->setLink(this);
                ft->setActivated();
                if (FloatingTextWidget::isTextValid(ft->text()))
                    ft->show();
                else
                    ft->hide();
                break;

            case Uml::TextRole::RoleAName:
                m_role[RoleType::A].roleWidget = ft;
                setRoleName(ft->text(), RoleType::A);
                break;
            case Uml::TextRole::RoleBName:
                m_role[RoleType::B].roleWidget = ft;
                setRoleName(ft->text(), RoleType::B);
                break;
            default:
                DEBUG(DBG_SRC) << "unexpected FloatingTextWidget (TextRole::Enum " << role << ")";
                delete ft;
                break;
            }
        }
        node = element.nextSibling();
        element = node.toElement();
    }

    return true;
}

/**
 * Queries the UMLView for resolving the role A and role B widgets.
 * ....
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
