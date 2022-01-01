/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ASSOCIATIONWIDGET_H
#define ASSOCIATIONWIDGET_H

#include "associationline.h"
#include "associationwidgetlist.h"
#include "associationwidgetrole.h"
#include "linkwidget.h"
#include "messagewidgetlist.h"
#include "umlwidgetlist.h"
#include "widgetbase.h"

class ClassifierWidget;
class UMLScene;
class UMLAssociation;
class UMLAttribute;
class UMLClassifierListItem;
class UMLOperation;

/**
 * This class represents an association inside a diagram.
 *
 * Constructor is made non accessible:
 * Users shall use the static create() methods for constructing AssociationWidgets.
 *
 * Associations exist not only between UML objects. For example, when a Note is
 * attached to a UML object, the Note itself is not a UML object.
 * This class supports both kinds of associations. An association where one or
 * both roles are not a UML object is called a "pure widget association".
 *
 * An AssociationWidget where both roles are UML objects has a corresponding
 * UMLAssociation. The UMLAssociation can be retrieved using the getAssociation
 * method.
 * A pure widget association does not have a corresponding UMLAssociation.
 * The getAssociation method returns NULL in this case.
 *
 * @author Gustavo Madrigal
 * @author Gopala Krishna
 * @short This class represents an association inside a diagram.
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class AssociationWidget : public WidgetBase, public LinkWidget
{
    Q_OBJECT
public:
    static AssociationWidget* create(UMLScene *scene);
    static AssociationWidget* create
                     (UMLScene *scene, UMLWidget* WidgetA,
                      Uml::AssociationType::Enum assocType, UMLWidget* WidgetB,
                      UMLObject *umlobject = 0);

    virtual ~AssociationWidget();

    virtual void setUMLObject(UMLObject *obj);

    //---------- LinkWidget Interface methods implementation from now on.

    virtual void lwSetFont(QFont font);
    virtual UMLClassifier *operationOwner();

    virtual UMLOperation *operation();
    virtual void setOperation(UMLOperation *op);

    virtual QString customOpText();
    virtual void setCustomOpText(const QString &opText);

    virtual void resetTextPositions();

    virtual void setMessageText(FloatingTextWidget *ft);
    virtual void setText(FloatingTextWidget *ft, const QString &newText);

    virtual bool showPropertiesDialog();

    virtual QString lwOperationText();
    virtual UMLClassifier *lwClassifier();
    virtual void setOperationText(const QString &op);

    virtual void constrainTextPos(qreal &textX, qreal &textY,
                                  qreal textWidth, qreal textHeight,
                                  Uml::TextRole::Enum tr);

    virtual void calculateNameTextSegment();

    //---------- End LinkWidget Interface methods implementation.

    UMLAssociation* association() const;
    UMLAttribute* attribute() const;

//    AssociationWidget& operator=(const AssociationWidget& other);
    bool operator==(const AssociationWidget& other) const;
    bool operator!=(AssociationWidget& other) const;

    FloatingTextWidget* textWidgetByRole(Uml::TextRole::Enum tr) const;

    FloatingTextWidget* nameWidget() const;
    QString name() const;
    void setName(const QString &strRole);
    void setStereotype(const QString &stereo);

    FloatingTextWidget* roleWidget(Uml::RoleType::Enum role) const;
    QString roleName(Uml::RoleType::Enum role) const;
    void setRoleName(const QString &strRole, Uml::RoleType::Enum role);

    QString roleDocumentation(Uml::RoleType::Enum role) const;
    void setRoleDocumentation(const QString& doc, Uml::RoleType::Enum role);

    FloatingTextWidget* multiplicityWidget(Uml::RoleType::Enum role) const;
    QString multiplicity(Uml::RoleType::Enum role) const;
    void setMultiplicity(const QString& text, Uml::RoleType::Enum role);

    Uml::Visibility::Enum visibility(Uml::RoleType::Enum role) const;
    void setVisibility(Uml::Visibility::Enum value, Uml::RoleType::Enum role);

    FloatingTextWidget* changeabilityWidget(Uml::RoleType::Enum role) const;
    Uml::Changeability::Enum changeability(Uml::RoleType::Enum role) const;
    void setChangeability(Uml::Changeability::Enum value, Uml::RoleType::Enum role);

    Uml::ID::Type widgetIDForRole(Uml::RoleType::Enum role) const;
    Uml::ID::Type widgetLocalIDForRole(Uml::RoleType::Enum role) const;
    UMLWidget* widgetForRole(Uml::RoleType::Enum role) const;
    void setWidgetForRole(UMLWidget* widget, Uml::RoleType::Enum role);

    bool containsAsEndpoint(UMLWidget* widget);

    Uml::AssociationType::Enum associationType() const;
    void setAssociationType(Uml::AssociationType::Enum type);

    bool isCollaboration() const;
    bool isSelf() const;

    QString toString() const;

    bool isActivated() const;
    void setActivated(bool active);

    const AssociationLine& associationLine() const;
    AssociationLine& associationLine();

    virtual bool activate(IDChangeLog *changeLog = 0);
    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;

    void widgetMoved(UMLWidget* widget, qreal x, qreal y);

    void saveIdealTextPositions();

    UMLWidget* onWidget(const QPointF &p);
    bool onAssociation(const QPointF& point);
    bool onAssocClassLine(const QPointF& point);

    void createAssocClassLine();
    void createAssocClassLine(ClassifierWidget* classifierWidget,
                              int linePathSegmentIndex);

    void selectAssocClassLine(bool sel = true);
    void removeAssocClassLine();
    void computeAssocClassLine();

    void setXEntireAssoc(qreal x);
    void setYEntireAssoc(qreal y);

    void moveMidPointsBy(qreal x, qreal y);
    void moveEntireAssoc(qreal x, qreal y);

    QFont font() const;

    virtual void setTextColor(const QColor &color);
    virtual void setLineColor(const QColor &color);
    virtual void setLineWidth(uint width);

    void calculateEndingPoints();

    void clipSize();

    bool loadFromXMI(QDomElement& qElement, const UMLWidgetList& widgets,
                     const MessageWidgetList* messages);
    virtual bool loadFromXMI(QDomElement& qElement);
    virtual void saveToXMI(QXmlStreamWriter& writer);

    void cleanup();

    bool isPointAddable();
    bool isPointRemovable();
    bool isAutoLayouted();
    bool isLayoutChangeable();

    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void setSelected(bool _select);

public Q_SLOTS:  //:TODO: all virtual?
    virtual void slotMenuSelection(QAction* action);
    void slotClassifierListItemRemoved(UMLClassifierListItem* obj);
    void slotAttributeChanged();

    void syncToModel();

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

private:
    QPointF calculateTextPosition(Uml::TextRole::Enum role);
    void setTextPosition(Uml::TextRole::Enum role);
    void setTextPositionRelatively(Uml::TextRole::Enum role, const QPointF &oldPosition);
    void setFloatingText(Uml::TextRole::Enum role, const QString& text, FloatingTextWidget* &ft);

    AssociationWidget(UMLScene *scene);

    void setUMLAssociation(UMLAssociation * assoc);

    void mergeAssociationDataIntoUMLRepresentation();

    static Uml::Region::Enum findPointRegion(const QRectF& rect, const QPointF& pos);
    static qreal findInterceptOnEdge(const QRectF &rect, Uml::Region::Enum region, const QPointF &point);
    static QLineF::IntersectType intersect(const QRectF &rect, const QLineF &line,
                                           QPointF* intersectionPoint);

    void moveEvent(QGraphicsSceneMouseEvent *me);

    Uml::TextRole::Enum calculateNameType(Uml::TextRole::Enum defaultRoleType);

    static QPointF swapXY(const QPointF &p);

    // not used at the moment
    // static QPointF calculatePointAtDistance(const QPointF &P1, const QPointF &P2, float Distance);
    // static QPointF calculatePointAtDistanceOnPerpendicular(const QPointF &P1, const QPointF &P2, float Distance);
    // static float perpendicularProjection(const QPointF& P1, const QPointF& P2, const QPointF& P3, QPointF& ResultingPoint);

    static QPointF midPoint(const QPointF& p0, const QPointF& p1);

    void createPointsSelfAssociation();
    void updatePointsSelfAssociation();
    void createPointsException();
    void updatePointsException();

    void updateRegionLineCount(int index, int totalCount,
                               Uml::Region::Enum region, Uml::RoleType::Enum role);

    void updateAssociations(int totalCount, Uml::Region::Enum region, Uml::RoleType::Enum role);

    int getRegionCount(Uml::Region::Enum region, Uml::RoleType::Enum role);

    void doUpdates(const QPointF &otherP, Uml::RoleType::Enum role);

    void setChangeWidget(const QString &strChangeWidget, Uml::RoleType::Enum role);

    bool checkAddPoint(const QPointF &scenePos);
    bool checkRemovePoint(const QPointF &scenePos);
    bool checkAutoLayoutSpline();

    bool linePathStartsAt(const UMLWidget* widget);

    void insertIntoLists(qreal position, const AssociationWidget* assoc);

    qreal m_positions[100];           ///< auxiliary variable for updateAssociations()
    int m_positions_len;              ///< auxiliary variable for updateAssociations()
    AssociationWidgetList m_ordered;  ///< auxiliary variable for updateAssociations()

    bool m_activated;   ///< flag which is true if the activate method has been called for this class instance

    /**
     * When the association has a Role Floating Text this text should move
     * when the AssociationLine moves but only if the closest segment to the
     * role text moves.
     * This segment is:
     * m_associationLine[m_unNameLineSegment] -- m_associationLine[m_unNameLineSegment+1]
     */
    int                 m_unNameLineSegment;

    QPointF m_oldNamePoint;    ///< Position of name floatingtext saved by saveIdealTextPositions()
    QPointF m_oldMultiAPoint;  ///< Position of role A multiplicity floatingtext saved by saveIdealTextPositions()
    QPointF m_oldMultiBPoint;  ///< Position of role B multiplicity floatingtext saved by saveIdealTextPositions()
    QPointF m_oldChangeAPoint; ///< Position of role A changeability floatingtext saved by saveIdealTextPositions()
    QPointF m_oldChangeBPoint; ///< Position of role B changeability floatingtext saved by saveIdealTextPositions()
    QPointF m_oldRoleAPoint;   ///< Position of role A name floatingtext saved by saveIdealTextPositions()
    QPointF m_oldRoleBPoint;   ///< Position of role B name floatingtext saved by saveIdealTextPositions()

    int m_nLinePathSegmentIndex;               ///< anchor for m_pAssocClassLine
    QGraphicsLineItem *m_pAssocClassLine;      ///< used for connecting assoc. class
    QGraphicsRectItem *m_pAssocClassLineSel0;  ///< selection decoration for the start point of the assoc. class line
    QGraphicsRectItem *m_pAssocClassLineSel1;  ///< selection decoration for the end point of the assoc. class line

    AssociationLine   m_associationLine;     ///< the definition points for the association line
    ClassifierWidget *m_associationClass;    ///< used if we have an assoc. class
    Uml::AssociationType::Enum m_associationType;  ///< is only used if m_pObject is not set
    AssociationWidgetRole  m_role[2];
    FloatingTextWidget* m_nameWidget;  ///< displays the name of this association
    QPointF m_eventScenePos;           ///< holds scene pos of contextMenuEvent()

};

#endif
