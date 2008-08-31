/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCIATIONWIDGET_H
#define ASSOCIATIONWIDGET_H

#include "widgetbase.h"
#include "linkwidget.h"
#include "umlwidgetlist.h"
#include "messagewidgetlist.h"
#include "associationwidgetlist.h"
#include "linepath.h"

class ListPopupMenu;
class ClassifierWidget;
class UMLDoc;
class UMLView;
class UMLAssociation;
class UMLClassifierListItem;
class UMLAttribute;
class UMLOperation;
class QMoveEvent;
class QGraphicsSceneMouseEvent;

/**
 * This class represents an association inside a diagram.
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
 *
 * @author Gustavo Madrigal
 * @short This class represents an association inside a diagram.
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssociationWidget : public NewUMLWidget, public LinkWidget {
    Q_OBJECT
public:

    /**
     * Enumeration used for stating where a line is on a widget.
     */
    enum Region {
        Error = 0,
        West, North, East, South,
        NorthWest, NorthEast, SouthEast, SouthWest,
        Center
    };

    /**
     * Constructor.
     *
     * @param scene              The scene to which this association belongs.
     */
    AssociationWidget(UMLScene *scene);

    /**
     * Constructor.
     *
     * @param view      The parent view of this widget.
     * @param WidgetA   Pointer to the role A widget for the association.
     * @param Type      The Association_Type for this association.
     * @param WidgetB   Pointer to the role B widget for the association.
     * @param umlobject Pointer to the underlying UMLObject (if applicable.)
     */
    AssociationWidget(UMLScene *scene, NewUMLRectWidget* WidgetA,
                      Uml::Association_Type Type, NewUMLRectWidget* WidgetB,
                      UMLObject *umlobject = NULL);

    /**
     * Deconstructor.
     */
    virtual ~AssociationWidget();

    /**
     * Overrides the assignment operator.
     */
    AssociationWidget& operator=(const AssociationWidget & Other);

    /**
     * Overrides the equality test operator.
     */
    bool operator==(const AssociationWidget & Other);

    /**
     * Overrides the != operator.
     */
    bool operator!=(AssociationWidget & Other);

    bool activate();

    void setWidget(NewUMLRectWidget* widget, Uml::Role_Type role);

    FloatingTextWidget* getMultiWidget(Uml::Role_Type role);

    QString getMulti(Uml::Role_Type role) const;

    FloatingTextWidget* getNameWidget();

    QString getName() const;

    FloatingTextWidget* getRoleWidget(Uml::Role_Type role);

    FloatingTextWidget* getTextWidgetByRole(Uml::Text_Role tr);

    QString getRoleName(Uml::Role_Type role) const;

    QString getRoleDoc(Uml::Role_Type role) const;

    void setName (const QString &strRole);

    void setMulti(const QString &strMulti, Uml::Role_Type role);

    Uml::Visibility getVisibility (Uml::Role_Type role) const;

    void setVisibility (Uml::Visibility visibility, Uml::Role_Type role );

    Uml::Changeability_Type getChangeability(Uml::Role_Type role) const;

    void setChangeability (Uml::Changeability_Type value, Uml::Role_Type role);

    Uml::IDType getWidgetID(Uml::Role_Type role) const;

    NewUMLRectWidget* getWidget(Uml::Role_Type role) const;

    bool setWidgets( NewUMLRectWidget* widgetA, Uml::Association_Type assocType, NewUMLRectWidget* widgetB);

    bool checkAssoc(NewUMLRectWidget * widgetA, NewUMLRectWidget *widgetB);

    bool contains(NewUMLRectWidget* widget);

    bool contains(const QPointF& point) const {
        return NewUMLWidget::contains(point);
    }

    bool isCollaboration();

    Uml::Association_Type getAssocType() const;

    void setAssocType(Uml::Association_Type type);

    QString toString();

    bool isActivated();

    void setActivated(bool active /*=true*/);

    void setSelected(bool _select = true);

    bool getSelected() const {
        return m_bSelected;
    }

    LinePath* getLinePath() {
        return &m_LinePath;
    }

    void widgetMoved(NewUMLRectWidget* widget, qreal x, qreal y);


    void updatePointsException ();


    void saveIdealTextPositions();

    void calculateNameTextSegment();

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * me);

    void mousePressEvent(QGraphicsSceneMouseEvent * me);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent * me);

    void mouseMoveEvent(QGraphicsSceneMouseEvent * me);

    bool onAssociation(const QPointF & point);

    bool onAssocClassLine(const QPointF & point);

    void createAssocClassLine();

    void createAssocClassLine(ClassifierWidget* classifierWidget,
                              int linePathSegmentIndex);

    void selectAssocClassLine(bool sel = true);

    void moveMidPointsBy( qreal x, qreal y );

    void moveEntireAssoc( qreal x, qreal y );

    QRectF getAssocLineRectangle();

    QFont getFont () const;

    void setLineColor(const QColor &colour);

    void setLineWidth(uint width);

    void lwSetFont (QFont font);

    FloatingTextWidget* getChangeWidget(Uml::Role_Type role);

    void setRoleName(const QString &strRole, Uml::Role_Type role);

    void setRoleDoc(const QString &doc, Uml::Role_Type role);

    UMLClassifier *getOperationOwner();

    UMLOperation *getOperation();

    void setOperation(UMLOperation *op);

    QString getCustomOpText();

    void setCustomOpText(const QString &opText);

    void setMessageText(FloatingTextWidget *ft);

    UMLAssociation * getAssociation() const;

    UMLAttribute * getAttribute() const;

    void setText(FloatingTextWidget *ft, const QString &text);

    void resetTextPositions();

    void constrainTextPos(qreal &textX, qreal &textY, qreal textWidth, qreal textHeight,
                          Uml::Text_Role tr);

    bool showDialog();

    void setIndex(int index, Uml::Role_Type role);

    int getIndex(Uml::Role_Type role) const;

    void setTotalCount(int count, Uml::Role_Type role);

    int getTotalCount(Uml::Role_Type role) const;

    void setTotalCount(int count);

    void setSeqNumAndOp(const QString &seqNum, const QString &op);

    UMLClassifier * getSeqNumAndOp(QString& seqNum, QString& op);

    void calculateEndingPoints();

    void removeAssocClassLine();

    void computeAssocClassLine();

    void setUMLObject(UMLObject *obj);

    void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget*) {}

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    bool loadFromXMI( QDomElement & qElement );

    bool loadFromXMI( QDomElement & qElement, const UMLWidgetList& widgets,
                      const MessageWidgetList* pMessages = NULL);

    void cleanup();

private:

    void setUMLAssociation (UMLAssociation * assoc);

    void mergeAssociationDataIntoUMLRepresentation();

    static Region findPointRegion(const QRectF& Rect, qreal PosX, qreal PosY);

    static qreal findInterceptOnEdge(const QRectF &rect, Region region, const QPointF &point);

    static QPointF findIntercept(const QRectF &rect, const QPointF &point);

    void moveEvent(QMoveEvent *me);

    Uml::Text_Role CalculateNameType(Uml::Text_Role defaultRoleType);

    // [PORT] This isn't used, remove this.
#if 0

    bool isPointInsideBoundaries(qreal PosX, qreal PosY, QPointF & SPoint,
                                 uint & StartSegmentIndex, uint & EndSegmentIndex);
#endif

    static QPointF swapXY(const QPointF &p);

    qreal totalLength();

    static QPointF calculatePointAtDistance(const QPointF &P1, const QPointF &P2, qreal Distance);

    static QPointF calculatePointAtDistanceOnPerpendicular(const QPointF &P1, const QPointF &P2, qreal Distance);

    static qreal perpendicularProjection(const QPointF& P1, const QPointF& P2, const QPointF& P3, QPointF& ResultingPoint);

    static QPointF midPoint(const QPointF& p0, const QPointF& p1);

    QPointF calculateTextPosition(Uml::Text_Role role);

    void setTextPosition(Uml::Text_Role role);

    void setTextPositionRelatively(Uml::Text_Role role, const QPointF &oldPosition);

    Region getWidgetRegion(AssociationWidget * widget) const;

    /**
     * This is a pointer to the Floating Text widget which displays the
     * name of this association.
     */
    FloatingTextWidget* m_pName;

    /**
     * The WidgetRole struct gathers all information pertaining to the role.
     * The AssociationWidget class contains two WidgetRole objects, one for each
     * side of the association (A and B).
     */
    struct WidgetRole {

        /**
         * This is a pointer to the Floating Text widget at the role's side
         * of the association.
         * This FloatingTextWidget displays the information regarding multiplicity.
         */
        FloatingTextWidget* m_pMulti;

        /**
         * This is a pointer to the Floating Text widget at the role's side
         * of the association.
         * This FloatingTextWidget displays the information regarding changeability.
         */
        FloatingTextWidget* m_pChangeWidget;

        /**
         * This member holds a pointer to the floating text that displays
         * the role's label of this association.
         */
        FloatingTextWidget* m_pRole;

        /**
         * This member holds a pointer to the NewUMLRectWidget at this role's side
         * of the association.
         */
        NewUMLRectWidget* m_pWidget;

        /**
         * This role's old top left corner before moving.
         */
        QPointF m_OldCorner;

        /**
         * The region of this role's widget.
         */
        Region m_WidgetRegion;

        /**
         * The index of where the line is on the region for this role.
         */
        int m_nIndex;

        /**
         * The total amount of associations on the region this role's line is on.
         */
        int m_nTotalCount;

        // The following items are only used if umlObject() is not set.
        Uml::Visibility m_Visibility;
        Uml::Changeability_Type m_Changeability;
        QString m_RoleDoc;

    } m_role[2];

    void setFloatingText(Uml::Text_Role tr, const QString &text, FloatingTextWidget* &ft);

    void updateRegionLineCount(int index, int totalCount,
                               AssociationWidget::Region region, Uml::Role_Type role);

    void updateAssociations(int totalCount, Region region, Uml::Role_Type role);

    int getRegionCount(Region region, Uml::Role_Type role);

    void init (UMLScene *scene);

    void doUpdates(qreal otherX, qreal otherY, Uml::Role_Type role);

    void setChangeWidget(const QString &strChangeWidget, Uml::Role_Type role);

    void checkPoints(const QPointF &p);

    bool linePathStartsAt(const NewUMLRectWidget* widget);

    void insertIntoLists(int position, const AssociationWidget* assoc);

    int m_positions[100];             ///< auxiliary variable for updateAssociations()
    int m_positions_len;              ///< auxiliary variable for updateAssociations()
    AssociationWidgetList m_ordered;  ///< auxiliary variable for updateAssociations()

    /**
     * Flag which is true if the activate method has been called for this
     * class instance.
     */
    bool m_bActivated;

    /**
     * When the association has a Role Floating Text this text should move
     * when the LinePath moves but only if the closest segment to the
     * role text moves.
     * This segment is:
     * m_LinePath[m_unNameLineSegment] -- m_LinePath[m_unNameLineSegment+1]
     */
    uint m_unNameLineSegment;
    UMLDoc * m_umldoc;  ///< just a shorthand for UMLApp::app()->getDocument()
    ListPopupMenu *m_pMenu;
    bool m_bSelected;
    int m_nMovingPoint;

    /**
     * Position of Name floatingtext saved by saveIdealTextPositions()
     */
    QPointF m_oldNamePoint;
    /**
     * Position of role A multiplicity floatingtext saved by
     * saveIdealTextPositions()
     */
    QPointF m_oldMultiAPoint;
    /**
     * Position of role B multiplicity floatingtext saved by
     * saveIdealTextPositions()
     */
    QPointF m_oldMultiBPoint;
    /**
     * Position of role A changeability floatingtext saved by
     * saveIdealTextPositions()
     */
    QPointF m_oldChangeAPoint;
    /**
     * Position of role B changeability floatingtext saved by
     * saveIdealTextPositions()
     */
    QPointF m_oldChangeBPoint;
    /**
     * Position of role A name floatingtext saved by
     * saveIdealTextPositions()
     */
    QPointF m_oldRoleAPoint;
    /**
     * Position of role B name floatingtext saved by
     * saveIdealTextPositions()
     */
    QPointF m_oldRoleBPoint;

    int m_nLinePathSegmentIndex; ///< anchor for m_pAssocClassLine
    QGraphicsLineItem *m_pAssocClassLine;  ///< used for connecting assoc. class
    /// selection adornment for the endpoints of the assoc. class connecting line
    QGraphicsRectItem *m_pAssocClassLineSel0, *m_pAssocClassLineSel1;

    ClassifierWidget *m_pAssocClassWidget;  ///< used if we have an assoc. class

    /**
     * The definition points for the association line.
     */
    LinePath m_LinePath;

    // The following items are only used if umlObject() is not set.
    Uml::Association_Type m_AssocType;

public slots:

    void slotMenuSelection(QAction* action);

    void slotRemovePopupMenu();

    void slotClearAllSelected();

    void slotClassifierListItemRemoved(UMLClassifierListItem* obj);


    void slotAttributeChanged();

    void syncToModel();
};
#endif
