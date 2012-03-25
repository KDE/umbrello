/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCIATIONWIDGET_H
#define ASSOCIATIONWIDGET_H

#include "associationline.h"
#include "associationwidgetlist.h"
#include "linkwidget.h"
#include "messagewidgetlist.h"
#include "umlwidgetlist.h"
#include "widgetbase.h"

class ListPopupMenu;
class ClassifierWidget;
class UMLDoc;
class UMLScene;
class UMLAssociation;
class UMLClassifierListItem;
class UMLAttribute;
class UMLOperation;

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
class AssociationWidget : public WidgetBase, public LinkWidget
{
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

    static AssociationWidget* create(UMLScene *scene);
    static AssociationWidget* create
                     (UMLScene *scene, UMLWidget* WidgetA,
                      Uml::AssociationType Type, UMLWidget* WidgetB,
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

    void constrainTextPos(int &textX, int &textY, int textWidth, int textHeight,
                          Uml::TextRole tr);

    virtual void calculateNameTextSegment();

    //---------- End LinkWidget Interface methods implemementation.

    UMLAssociation* association() const;
    UMLAttribute* attribute() const;

    //AssociationWidget& operator=(const AssociationWidget & Other);
    bool operator==(const AssociationWidget & Other) const;
    bool operator!=(AssociationWidget & Other) const;

    bool activate();

    void setWidget(UMLWidget* widget, Uml::Role_Type role);

    FloatingTextWidget* textWidgetByRole(Uml::TextRole tr) const;

    FloatingTextWidget* nameWidget() const;
    QString name() const;
    void setName(const QString &strRole);

    FloatingTextWidget* roleWidget(Uml::Role_Type role) const;
    QString roleName(Uml::Role_Type role) const;
    void setRoleName(const QString &strRole, Uml::Role_Type role);

    QString roleDocumentation(Uml::Role_Type role) const;
    void setRoleDocumentation(const QString& doc, Uml::Role_Type role);

    FloatingTextWidget* multiplicityWidget(Uml::Role_Type role) const;
    QString multiplicity(Uml::Role_Type role) const;
    void setMultiplicity(const QString &strMulti, Uml::Role_Type role);

    Uml::Visibility visibility(Uml::Role_Type role) const;
    void setVisibility(Uml::Visibility value, Uml::Role_Type role );

    FloatingTextWidget* changeabilityWidget(Uml::Role_Type role) const;
    Uml::Changeability changeability(Uml::Role_Type role) const;
    void setChangeability(Uml::Changeability value, Uml::Role_Type role);

    Uml::IDType widgetIDForRole(Uml::Role_Type role) const;
    UMLWidget* widgetForRole(Uml::Role_Type role) const;

    bool setWidgets(UMLWidget* widgetA, Uml::AssociationType assocType, UMLWidget* widgetB);

    bool checkAssoc(UMLWidget *widgetA, UMLWidget *widgetB);

    bool contains(UMLWidget* widget);

    bool isCollaboration();

    Uml::AssociationType associationType() const;
    void setAssociationType(Uml::AssociationType type);

    QString toString();

    bool isActivated();
    void setActivated(bool active /*=true*/);

    void setSelected(bool _select = true);

    /**
     * Returns the state of whether the widget is selected.
     *
     * @return  Returns the state of whether the widget is selected.
     */
    bool getSelected() const {
        return m_selected;
    }

    AssociationLine* associationLine() const;

    void widgetMoved(UMLWidget* widget, int x, int y);

    void updatePointsException ();

    void saveIdealTextPositions();

    void mouseDoubleClickEvent(QMouseEvent * me);
    void mousePressEvent(QMouseEvent * me);
    void mouseReleaseEvent(QMouseEvent * me);
    void mouseMoveEvent(QMouseEvent * me);

    bool onAssociation(const QPoint & point);
    bool onAssocClassLine(const QPoint & point);

    void createAssocClassLine();
    void createAssocClassLine(ClassifierWidget* classifierWidget,
                              int linePathSegmentIndex);

    void selectAssocClassLine(bool sel = true);

    void moveMidPointsBy( int x, int y );

    void moveEntireAssoc( int x, int y );

    QRect getAssocLineRectangle();

    QFont font() const;

    virtual void setTextColor(const QColor &color);
    virtual void setLineColor(const QColor &colour);
    void setLineWidth(uint width);

    void setIndex(int index, Uml::Role_Type role);
    int getIndex(Uml::Role_Type role) const;

    void setTotalCount(int count, Uml::Role_Type role);
    int getTotalCount(Uml::Role_Type role) const;

    void setTotalCount(int count);

    void calculateEndingPoints();

    void removeAssocClassLine();

    void computeAssocClassLine();

    void clipSize();

    bool loadFromXMI(QDomElement & qElement, const UMLWidgetList& widgets,
                     const MessageWidgetList* pMessages = NULL);
    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

    void cleanup();

public slots:
    void slotMenuSelection(QAction* action);
    void slotRemovePopupMenu();
    void slotClearAllSelected();
    void slotClassifierListItemRemoved(UMLClassifierListItem* obj);
    void slotAttributeChanged();

    void syncToModel();

private:

    /**
     * Constructor is made non accessible:
     * Users shall use the static create() methods for constructing AssociationWidgets.
     */
    AssociationWidget(UMLScene *scene);

    void setUMLAssociation (UMLAssociation * assoc);

    void mergeAssociationDataIntoUMLRepresentation();

    static Region findPointRegion(const QRect& Rect, int PosX, int PosY);

    static int findInterceptOnEdge(const QRect &rect, Region region, const QPoint &point);

    static QPoint findIntercept(const QRect &rect, const QPoint &point);

    void moveEvent(QMoveEvent *me);

    Uml::TextRole calculateNameType(Uml::TextRole defaultRoleType);

    bool isPointInsideBoundaries(int PosX, int PosY, QPoint & SPoint,
                                 uint & StartSegmentIndex, uint & EndSegmentIndex);

    static QPoint swapXY(const QPoint &p);

    float totalLength();

    static QPoint calculatePointAtDistance(const QPoint &P1, const QPoint &P2, float Distance);

    static QPoint calculatePointAtDistanceOnPerpendicular(const QPoint &P1, const QPoint &P2, float Distance);

    static float perpendicularProjection(const QPoint& P1, const QPoint& P2, const QPoint& P3, QPoint& ResultingPoint);

    static QPoint midPoint(const QPoint& p0, const QPoint& p1);

    QPoint calculateTextPosition(Uml::TextRole role);

    void setTextPosition(Uml::TextRole role);

    void setTextPositionRelatively(Uml::TextRole role, const QPoint &oldPosition);

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
         * This member holds a pointer to the UMLWidget at this role's side
         * of the association.
         */
        UMLWidget* m_pWidget;

        /**
         * This role's old top left corner before moving.
         */
        QPoint m_OldCorner;

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

        // The following items are only used if m_pObject is not set.
        Uml::Visibility m_Visibility;
        Uml::Changeability m_Changeability;
        QString m_RoleDoc;

    } m_role[2];

    void setFloatingText(Uml::TextRole tr, const QString &text, FloatingTextWidget* &ft);

    void updateRegionLineCount(int index, int totalCount,
                               AssociationWidget::Region region, Uml::Role_Type role);

    void updateAssociations(int totalCount, Region region, Uml::Role_Type role);

    int getRegionCount(Region region, Uml::Role_Type role);

    void init();

    void doUpdates(int otherX, int otherY, Uml::Role_Type role);

    void setChangeWidget(const QString &strChangeWidget, Uml::Role_Type role);

    void checkPoints(const QPoint &p);

    bool linePathStartsAt(const UMLWidget* widget);

    void insertIntoLists(int position, const AssociationWidget* assoc);

    int m_positions[100];             ///< auxiliary variable for updateAssociations()
    int m_positions_len;              ///< auxiliary variable for updateAssociations()
    AssociationWidgetList m_ordered;  ///< auxiliary variable for updateAssociations()

    /**
     * Flag which is true if the activate method has been called for this
     * class instance.
     */
    bool m_activated;

    /**
     * When the association has a Role Floating Text this text should move
     * when the AssociationLine moves but only if the closest segment to the
     * role text moves.
     * This segment is:
     * m_associationLine[m_unNameLineSegment] -- m_associationLine[m_unNameLineSegment+1]
     */
    uint                m_unNameLineSegment;
    UMLDoc * m_umldoc;  ///< just a shorthand for UMLApp::app()->getDocument()
    ListPopupMenu       *m_pMenu;
    bool                m_selected;
    int                 m_nMovingPoint;

    /**
     * Position of Name floatingtext saved by saveIdealTextPositions()
     */
    QPoint m_oldNamePoint;
    /**
     * Position of role A multiplicity floatingtext saved by
     * saveIdealTextPositions()
     */
    QPoint m_oldMultiAPoint;
    /**
     * Position of role B multiplicity floatingtext saved by
     * saveIdealTextPositions()
     */
    QPoint m_oldMultiBPoint;
    /**
     * Position of role A changeability floatingtext saved by
     * saveIdealTextPositions()
     */
    QPoint m_oldChangeAPoint;
    /**
     * Position of role B changeability floatingtext saved by
     * saveIdealTextPositions()
     */
    QPoint m_oldChangeBPoint;
    /**
     * Position of role A name floatingtext saved by
     * saveIdealTextPositions()
     */
    QPoint m_oldRoleAPoint;
    /**
     * Position of role B name floatingtext saved by
     * saveIdealTextPositions()
     */
    QPoint m_oldRoleBPoint;

    int         m_nLinePathSegmentIndex; ///< anchor for m_pAssocClassLine
    UMLSceneLineItem *m_pAssocClassLine;  ///< used for connecting assoc. class
    /// selection adornment for the endpoints of the assoc. class connecting line
    UMLSceneRectItem *m_pAssocClassLineSel0, *m_pAssocClassLineSel1;

    ClassifierWidget *m_pAssocClassWidget;  ///< used if we have an assoc. class

    /**
     * The definition points for the association line.
     */
    AssociationLine *m_associationLine;

    // The following items are only used if m_pObject is not set.
    Uml::AssociationType m_AssocType;

};

#endif
