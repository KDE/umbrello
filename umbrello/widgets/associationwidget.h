/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCIATIONWIDGET_H
#define ASSOCIATIONWIDGET_H

#include "associationwidgetlist.h"
#include "linkwidget.h"
#include "messagewidgetlist.h"
#include "umlwidgetlist.h"
#include "widgetbase.h"

class AssociationLine;
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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssociationWidget : public WidgetBase, public LinkWidget
{
    Q_OBJECT
public:
    static AssociationWidget* create(UMLScene *scene);
    static AssociationWidget* create
                     (UMLScene *scene, UMLWidget* WidgetA,
                      Uml::AssociationType::Enum assocType, UMLWidget* WidgetB,
                      UMLObject *umlobject = NULL);

    virtual ~AssociationWidget();

    virtual void setUMLObject(UMLObject *obj);

    //---------- LinkWidget Interface methods implemementation from now on.

    virtual void lwSetFont(QFont font);
    virtual UMLClassifier *operationOwner();

    virtual UMLOperation *operation();
    virtual void setOperation(UMLOperation *op);

    virtual QString customOpText();
    virtual void setCustomOpText(const QString &opText);

    virtual void resetTextPositions();

    virtual void setMessageText(FloatingTextWidget *ft);
    virtual void setText(FloatingTextWidget *ft, const QString &newText);

    virtual void showPropertiesDialog();

    virtual UMLClassifier* seqNumAndOp(QString& seqNum, QString& op);
    virtual void setSeqNumAndOp(const QString &seqNum, const QString &op);

    virtual void constrainTextPos(qreal &textX, qreal &textY,
                                  qreal textWidth, qreal textHeight,
                                  Uml::TextRole::Enum tr);

    virtual void calculateNameTextSegment();

    //---------- End LinkWidget Interface methods implemementation.

    UMLAssociation* association() const;
    UMLAttribute* attribute() const;

//    AssociationWidget& operator=(const AssociationWidget& other);
    bool operator==(const AssociationWidget& other) const;
    bool operator!=(AssociationWidget& other) const;

    FloatingTextWidget* textWidgetByRole(Uml::TextRole::Enum tr) const;

    FloatingTextWidget* nameWidget() const;
    QString name() const;
    void setName(const QString &strRole);

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
    UMLWidget* widgetForRole(Uml::RoleType::Enum role) const;
    void setWidgetForRole(UMLWidget* widget, Uml::RoleType::Enum role);

    bool setWidgets(UMLWidget* widgetA, Uml::AssociationType::Enum assocType, UMLWidget* widgetB);

//    bool checkAssoc(UMLWidget *widgetA, UMLWidget *widgetB);

    bool containsAsEndpoint(UMLWidget* widget);

    Uml::AssociationType::Enum associationType() const;
    void setAssociationType(Uml::AssociationType::Enum type);

    bool isCollaboration() const;
    bool isSelf() const;

    QString toString() const;

    bool isActivated() const;
    void setActivated(bool active /*=true*/);

    bool isSelected() const;
    void setSelected(bool _select = true);

    AssociationLine* associationLine() const;

    virtual bool activate();
    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;

    void widgetMoved(UMLWidget* widget, int x, int y);

    void saveIdealTextPositions();

    bool onAssociation(const QPointF& point);
    bool onAssocClassLine(const QPointF& point);

    void createAssocClassLine();
    void createAssocClassLine(ClassifierWidget* classifierWidget,
                              int linePathSegmentIndex);

    void selectAssocClassLine(bool sel = true);
    void removeAssocClassLine();
    void computeAssocClassLine();

    void moveMidPointsBy(int x, int y);
    void moveEntireAssoc(int x, int y);

    QFont font() const;

    virtual void setTextColor(const QColor &color);

//    void setIndex(int index, Uml::RoleType::Enum role);
//    int getIndex(Uml::RoleType::Enum role) const;

    void calculateEndingPoints();

    void clipSize();

    bool loadFromXMI(QDomElement& qElement, const UMLWidgetList& widgets,
                     const MessageWidgetList* messages);
    virtual bool loadFromXMI(QDomElement& qElement);
    virtual void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

    void cleanup();

    bool isPointAddable();
    bool isPointRemovable();

    //:TODO: the following four methods should be protected
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public Q_SLOTS:  //:TODO: all virtual?
    virtual void slotMenuSelection(QAction* action);
    void slotClearAllSelected();
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

    static Uml::Region::Enum findPointRegion(const QRectF& Rect, int PosX, int PosY);
    static int findInterceptOnEdge(const QRectF &rect, Uml::Region::Enum region, const QPointF &point);
    static QPointF findIntercept(const QRectF &rect, const QPointF &point);

    void moveEvent(QGraphicsSceneMouseEvent *me);

    Uml::TextRole::Enum calculateNameType(Uml::TextRole::Enum defaultRoleType);

//    bool isPointInsideBoundaries(int PosX, int PosY, QPointF & SPoint,
//                                 uint & StartSegmentIndex, uint & EndSegmentIndex);

    static QPointF swapXY(const QPointF &p);

//    float totalLength();

    static QPointF calculatePointAtDistance(const QPointF &P1, const QPointF &P2, float Distance);
    static QPointF calculatePointAtDistanceOnPerpendicular(const QPointF &P1, const QPointF &P2, float Distance);

    static float perpendicularProjection(const QPointF& P1, const QPointF& P2, const QPointF& P3, QPointF& ResultingPoint);

    static QPointF midPoint(const QPointF& p0, const QPointF& p1);

    void updatePointsException();

//    Uml::Region::Enum getWidgetRegion(AssociationWidget * widget) const;

    /**
     * The WidgetRole struct gathers all information pertaining to the role.
     * The AssociationWidget class contains two WidgetRole objects, one for each
     * side of the association (A and B).
     */
    struct WidgetRole {

        FloatingTextWidget* multiplicityWidget;   ///< information regarding multiplicity
        FloatingTextWidget* changeabilityWidget;  ///< information regarding changeability
        FloatingTextWidget* roleWidget;           ///< role's label of this association

        UMLWidget* umlWidget;    ///< UMLWidget at this role's side of this association

        Uml::Region::Enum     m_WidgetRegion;   ///< region of this role's widget

        int m_nIndex;        ///< the index of where the line is on the region for this role
        int m_nTotalCount;   ///< total amount of associations on the region this role's line is on

        // The following items are only used if m_pObject is not set.
        Uml::Visibility::Enum     visibility;
        Uml::Changeability::Enum  changeability;
        QString                   roleDocumentation;

    };

    void updateRegionLineCount(int index, int totalCount,
                               Uml::Region::Enum region, Uml::RoleType::Enum role);

    void updateAssociations(int totalCount, Uml::Region::Enum region, Uml::RoleType::Enum role);

    int getRegionCount(Uml::Region::Enum region, Uml::RoleType::Enum role);

    void doUpdates(int otherX, int otherY, Uml::RoleType::Enum role);

    void setChangeWidget(const QString &strChangeWidget, Uml::RoleType::Enum role);

    void checkPoints(const QPointF &p);
    bool checkAddPoint(const QPointF &scenePos);
    bool checkRemovePoint(const QPointF &scenePos);

    bool linePathStartsAt(const UMLWidget* widget);

    void insertIntoLists(int position, const AssociationWidget* assoc);

    int m_positions[100];             ///< auxiliary variable for updateAssociations()
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
    bool                m_selected;
    int                 m_nMovingPoint;

    QPointF m_oldNamePoint;    ///< Position of name floatingtext saved by saveIdealTextPositions()
    QPointF m_oldMultiAPoint;  ///< Position of role A multiplicity floatingtext saved by saveIdealTextPositions()
    QPointF m_oldMultiBPoint;  ///< Position of role B multiplicity floatingtext saved by saveIdealTextPositions()
    QPointF m_oldChangeAPoint; ///< Position of role A changeability floatingtext saved by saveIdealTextPositions()
    QPointF m_oldChangeBPoint; ///< Position of role B changeability floatingtext saved by saveIdealTextPositions()
    QPointF m_oldRoleAPoint;   ///< Position of role A name floatingtext saved by saveIdealTextPositions()
    QPointF m_oldRoleBPoint;   ///< Position of role B name floatingtext saved by saveIdealTextPositions()

    int m_nLinePathSegmentIndex; ///< anchor for m_pAssocClassLine
    QGraphicsLineItem *m_pAssocClassLine;  ///< used for connecting assoc. class
    /// selection adornment for the endpoints of the assoc. class connecting line
    QGraphicsRectItem *m_pAssocClassLineSel0;
    QGraphicsRectItem *m_pAssocClassLineSel1;

    AssociationLine *m_associationLine;      ///< the definition points for the association line
    ClassifierWidget *m_associationClass;    ///< used if we have an assoc. class
    Uml::AssociationType::Enum m_associationType;  ///< is only used if m_pObject is not set
    WidgetRole  m_role[2];
    FloatingTextWidget* m_nameWidget;  ///< displays the name of this association
    QPointF m_eventScenePos;           ///< holds scene pos of contextMenuEvent()

};

#endif
