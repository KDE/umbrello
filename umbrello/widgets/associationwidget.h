/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCIATIONWIDGET_H
#define ASSOCIATIONWIDGET_H

#include "widgetbase.h"
#include "linkwidget.h"
#include "umlwidgetlist.h"
#include "messagewidgetlist.h"
#include "associationwidgetlist.h"
#include "associationline.h"

#include <QMouseEvent>
#include <QMoveEvent>

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

    /**
     * Constructor.
     *
     * @param scene              The parent view of this widget.
     */
    AssociationWidget(UMLScene *scene);

    /**
     * Constructor.
     *
     * @param scene      The parent view of this widget.
     * @param WidgetA   Pointer to the role A widget for the association.
     * @param Type      The AssociationType for this association.
     * @param WidgetB   Pointer to the role B widget for the association.
     * @param umlobject Pointer to the underlying UMLObject (if applicable.)
     */
    AssociationWidget(UMLScene *scene, UMLWidget* WidgetA,
                      Uml::AssociationType Type, UMLWidget* WidgetB,
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
    bool operator==(const AssociationWidget & Other) const;

    /**
     * Overrides the != operator.
     */
    bool operator!=(AssociationWidget & Other) const;

    /**
     * Activates the AssociationWidget after a load.
     *
     * @return  true for success
     */
    bool activate();

    /**
     * Set the widget of the given role.
     *
     * @param widget    Pointer to the UMLWidget.
     * @param role      Role for which to set the widget.
     */
    void setWidget(UMLWidget* widget, Uml::Role_Type role);

    /**
     * Return the multiplicity FloatingTextWidget widget of the given role.
     *
     * @return  Pointer to the multiplicity FloatingTextWidget object.
     */
    FloatingTextWidget* getMultiWidget(Uml::Role_Type role);

    /**
     * Read property of FloatingTextWidget* m_pName.
     *
     * @return  Pointer to the FloatingTextWidget name widget.
     */
    FloatingTextWidget* getNameWidget();

    /**
     * Returns the m_pName's text.
     *
     * @return  Text of the FloatingTextWidget name widget.
     */
    QString getName() const;

    /**
     * Return the given role's FloatingTextWidget object.
     *
     * @return  Pointer to the role's FloatingTextWidget widget.
     */
    FloatingTextWidget* getRoleWidget(Uml::Role_Type role);

    /**
     * Return the FloatingTextWidget object indicated by the given Text_Role.
     *
     * @return  Pointer to the text role's FloatingTextWidget widget.
     */
    FloatingTextWidget* getTextWidgetByRole(Uml::TextRole tr);

    QString roleName(Uml::Role_Type role) const;
    QString roleDocumentation(Uml::Role_Type role) const;

    /**
     * Sets the text in the FloatingTextWidget widget representing the Name
     * of this association.
     */
    void setName(const QString &strRole);

    QString multiplicity(Uml::Role_Type role) const;
    void setMultiplicity(const QString &strMulti, Uml::Role_Type role);

    Uml::Visibility visibility(Uml::Role_Type role) const;
    void setVisibility(Uml::Visibility visibility, Uml::Role_Type role );

    Uml::Changeability changeability(Uml::Role_Type role) const;
    void setChangeability (Uml::Changeability value, Uml::Role_Type role);

    /**
     * Gets the ID of the given role widget.
     */
    Uml::IDType getWidgetID(Uml::Role_Type role) const;

    /**
     * Gets the given role widget.
     *
     * @return  Pointer to the role's UMLWidget.
     */
    UMLWidget* widgetForRole(Uml::Role_Type role) const;

    /**
     * Sets the associated widgets.
     *
     * @param widgetA   Pointer the role A widget for the association.
     * @param assocType The AssociationType for this association.
     * @param widgetB   Pointer the role B widget for the association.
     */
    bool setWidgets(UMLWidget* widgetA, Uml::AssociationType assocType, UMLWidget* widgetB);

    /**
     * Returns true if this association associates widgetA to widgetB,
     * otherwise it returns false.
     *
     * @param widgetA   Pointer the role A widget to check.
     * @param widgetB   Pointer the role B widget to check.
     * @return  True if widgetA and widgetB are associated.
     */
    bool checkAssoc(UMLWidget *widgetA, UMLWidget *widgetB);

    /**
     * Returns true if the Widget is either at the starting or ending side
     * of the association.
     *
     * @return  True if widget plays role A or B in this assoc.
     */
    bool contains(UMLWidget* widget);

    /**
     * Returns true if this AssociationWidget represents a collaboration message.
     */
    bool isCollaboration();

    /**
     * Gets the association's type.
     *
     * @return  This AssociationWidget's AssociationType.
     */
    Uml::AssociationType associationType() const;

    /**
     * Sets the association's type.
     *
     * @param type   The AssociationType to set.
     */
    void setAssociationType(Uml::AssociationType type);

    /**
     * Returns a QString object representing this AssociationWidget.
     *
     * @return  Textual representation of the AssociationWidget.
     */
    QString toString();

    /**
     * Read property of bool m_activated.
     *
     * @return  True if this AssociationWidget has been activated.
     */
    bool isActivated();

    /**
     * Set the m_activated flag of a widget but does not perform the
     * activate method.
     *
     * @param active    The flag status to set.
     */
    void setActivated(bool active /*=true*/);

    /**
     * Sets the state of whether the widget is selected.
     *
     * @param _select   The state of whether the widget is selected.
     */
    void setSelected(bool _select = true);

    /**
     * Returns the state of whether the widget is selected.
     *
     * @return  Returns the state of whether the widget is selected.
     */
    bool getSelected() const {
        return m_selected;
    }

    /**
     * Returns a pointer to the association widget's line path.
     */
    AssociationLine* getLinePath() {
        return &m_LinePath;
    }

    /**
     * Adjusts the ending point of the association that connects to Widget
     *
     * @param widget    Pointer to the widget that was moved.
     * @param x         New X coordinate of the widget.
     * @param y         New Y coordinate of the widget.
     */
    void widgetMoved(UMLWidget* widget, int x, int y);

    /**
     * Adjusts the points of the association exception.
     * Method called when a widget was moved by widgetMoved(widget,x,y)
     */
    void updatePointsException ();

    /**
     * Auxiliary method for widgetMoved():
     * Saves all ideally computed floatingtext positions before doing any
     * kind of change.  This is necessary because a single invocation of
     * calculateEndingPoints() modifies the AssociationLine ending points on ALL
     * AssociationWidgets.  This means that if we don't save the old ideal
     * positions then they are irretrievably lost as soon as
     * calculateEndingPoints() is invoked.
     */
    void saveIdealTextPositions();

    /**
     * Calculates the m_unNameLineSegment value according to the new
     * NameText topleft corner PT.
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
    void calculateNameTextSegment();

    /**
     * Adds a break point (if left mouse button).
     */
    void mouseDoubleClickEvent(QMouseEvent * me);

    /**
     * Sets the association to be selected.
     */
    void mousePressEvent(QMouseEvent * me);

    /**
     * Displays the right mouse buttom menu if right button is pressed.
     */
    void mouseReleaseEvent(QMouseEvent * me);

    /**
     * Moves the break point being dragged.
     */
    void mouseMoveEvent(QMouseEvent * me);

    /**
     * Returns true if the given point is on the Association.
     */
    bool onAssociation(const QPoint & point);

    /**
     * Returns true if the given point is on the connecting line to
     * the association class. Returns false if there is no association
     * class attached, or if the given point is not on the connecting
     * line.
     */
    bool onAssocClassLine(const QPoint & point);

    /**
     * Creates the association class connecting line.
     */
    void createAssocClassLine();

    /**
     * Creates the association class connecting line using the specified
     * ClassifierWidget.
     *
     * @param classifierWidget The ClassifierWidget to use.
     * @param linePathSegmentIndex The index of the segment where the
     *        association class is created.
     */
    void createAssocClassLine(ClassifierWidget* classifierWidget,
                              int linePathSegmentIndex);

    /**
     * Renders the association class connecting line selected.
     */
    void selectAssocClassLine(bool sel = true);

    /**
     * Moves all the mid points (all expcept start /end ) by the given amount.
     */
    void moveMidPointsBy( int x, int y );

    /**
     * Moves the entire association by the given offset.
     */
    void moveEntireAssoc( int x, int y );

    /**
     * Returns the bounding rectangle of all segments of the association.
     */
    QRect getAssocLineRectangle();

    /**
     * Return the first font found being used by any child widget. (They
     * could be different fonts, so this is a slightly misleading method.)
     */
    QFont font() const;

    /**
     * Overrides the method from WidgetBase.
     */
    virtual void setTextColor(const QColor &color);

    /**
     * Overrides the method from WidgetBase.
     */
    virtual void setLineColor(const QColor &colour);

    /**
     * Overrides the method from WidgetBase.
     */
    void setLineWidth(uint width);

    /**
     * Set all 'owned' child widgets to this font.
     */
    void lwSetFont (QFont font);

    /**
     * Return the given role's changeability FloatingTextWidget widget.
     */
    FloatingTextWidget* getChangeWidget(Uml::Role_Type role);

    /**
     * Sets the text to the FloatingTextWidget that display the Role text of this
     * association.
     * For this function to work properly, the associated widget
     *  should already be set.
     */
    void setRoleName(const QString &strRole, Uml::Role_Type role);

    void setRoleDocumentation(const QString &doc, Uml::Role_Type role);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     * @todo Move to LinkWidget.
     */
    UMLClassifier *getOperationOwner();

    UMLOperation *operation();
    void setOperation(UMLOperation *op);

    QString customOpText();
    void setCustomOpText(const QString &opText);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     *
     * @param ft   The text widget which to update.
     */
    void setMessageText(FloatingTextWidget *ft);

    /**
     * Returns the UMLAssociation representation of this object.
     *
     * @return  Pointer to the UMLAssociation that is represented by
     *          this AsociationWidget.
     */
    UMLAssociation * getAssociation() const;

    /**
     * Returns the UMLAttribute representation of this object.
     *
     * @return  Pointer to the UMLAttribute that is represented by
     *          this AsociationWidget.
     */
    UMLAttribute * getAttribute() const;

    /**
     * Sets the text of the given FloatingTextWidget.
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     */
    void setText(FloatingTextWidget *ft, const QString &text);

    /**
     * Calls setTextPosition on all the labels.
     * Overrides operation from LinkWidget.
     */
    void resetTextPositions();

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
    void constrainTextPos(int &textX, int &textY, int textWidth, int textHeight,
                          Uml::TextRole tr);

    void showPropertiesDialog();

    /**
     * Sets the Association line index for the given role.
     */
    void setIndex(int index, Uml::Role_Type role);

    /**
     * Returns the Association line index for the given role.
     */
    int getIndex(Uml::Role_Type role) const;

    /**
     * Sets the total count on the Association region.
     */
    void setTotalCount(int count, Uml::Role_Type role);

    /**
     * Returns the total count on the Association region.
     */
    int getTotalCount(Uml::Role_Type role) const;

    /**
     * Sets the total count on the Association region for widgetB.
     */
    void setTotalCount(int count);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     *
     * @param seqNum    The new sequence number string to set.
     * @param op        The new operation string to set.
     */
    void setSeqNumAndOp(const QString &seqNum, const QString &op);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     *
     * @param seqNum    Return this AssociationWidget's sequence number string.
     * @param op        Return this AssociationWidget's operation string.
     */
    UMLClassifier * seqNumAndOp(QString& seqNum, QString& op);

    /**
     * Calculates and sets the first and last point in the association's
     * AssociationLine.
     * Each point is a middle point of its respective UMLWidget's bounding
     * rectangle.
     * This method picks which sides to use for the association.
     */
    void calculateEndingPoints();

    /**
     * Remove dashed connecting line for association class.
     */
    void removeAssocClassLine();

    /**
     * Compute the end points of m_pAssocClassLine in case this
     * association has an attached association class.
     */
    void computeAssocClassLine();

    /**
     * Overriding the method from WidgetBase because we need to do
     * something extra in case this AssociationWidget represents
     * an attribute of a classifier.
     */
    void setUMLObject(UMLObject *obj);

    void clipSize();
    /**
     * Saves this widget to the "assocwidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads this widget from the "assocwidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

    /**
     * Same as above, but uses the supplied widgetList for resolving
     * the role A and role B widgets. (The other loadFromXMI() queries
     * the UMLView for these widgets.)
     * Required for clipboard operations.
     */
    bool loadFromXMI( QDomElement & qElement, const UMLWidgetList& widgets,
                      const MessageWidgetList* pMessages = NULL);

    /**
     * Cleans up all the association's data in the related widgets.
     */
    void cleanup();

private:

    /** set our internal umlAssociation */
    void setUMLAssociation (UMLAssociation * assoc);

    /**
     * Merges/syncs the association widget data into UML object
     * representation.
     * CHECK: Can we get rid of this.
     */
    void mergeAssociationDataIntoUMLRepresentation();

    /**
     * Finds out which region contains the point (PosX, PosY).
     *
     * The diagram is divided into four regions by its diagonals :
     *
     *                         Region 2
     *                      \            /
     *                       \          /
     *                        +--------+
     *                        | \    / |
     *           Region 1     |   ><   |    Region 3
     *                        | /    \ |
     *                        +--------+
     *                       /          \
     *                      /            \
     *                         Region 4
     *
     *
     * @param Rect              The bounding rectangle to investigate.
     * @param PosX              X coordinate of the point to seek.
     * @param PosY              Y coordinate of the point to seek.
     * @return  The region number of the region containing the point.
     *          1 = Region 1
     *          2 = Region 2
     *          3 = Region 3
     *          4 = Region 4
     *          5 = On diagonal 2 between Region 1 and 2
     *          6 = On diagonal 1 between Region 2 and 3
     *          7 = On diagonal 2 between Region 3 and 4
     *          8 = On diagonal 1 between Region4 and 1
     *          9 = On diagonal 1 and On diagonal 2 (the center)
     */
    static Region findPointRegion(const QRect& Rect, int PosX, int PosY);

    /**
     * Given a rectangle and a point, findInterceptOnEdge computes the
     * connecting line between the middle point of the rectangle and
     * the point, and returns the intercept of this line with the
     * the edge of the rectangle identified by `region'.
     * When the region is North or South, the X value is returned (Y is
     * constant.)
     * When the region is East or West, the Y value is returned (X is
     * constant.)
     * @todo This is buggy. Try replacing by findIntercept()
     */
    static int findInterceptOnEdge(const QRect &rect, Region region, const QPoint &point);

    static QPoint findIntercept(const QRect &rect, const QPoint &point);

    /**
     * Overrides moveEvent.
     */
    void moveEvent(QMoveEvent *me);

    /**
     * This function calculates which role should be set for the m_pName
     * FloatingTextWidget.
     */
    Uml::TextRole CalculateNameType(Uml::TextRole defaultRoleType);

    /**
     * Returns true if point (PosX, PosY) is close enough to any of the
     * association's segments.
     */
    bool isPointInsideBoundaries(int PosX, int PosY, QPoint & SPoint,
                                 uint & StartSegmentIndex, uint & EndSegmentIndex);

    /**
     * Returns a point with interchanged X and Y coordinates.
     */
    static QPoint swapXY(const QPoint &p);

    /**
     * Returns the total length of the association's AssociationLine:
     * result = segment_1_length + segment_2_length + ... + segment_n_length
     */
    float totalLength();

    /**
     * Calculates which point of segment P1P2 has a distance equal to
     * Distance from P1.
     * Let's say such point is PX, the distance from P1 to PX must be equal
     * to Distance and if PX is not a point of the segment P1P2 then the
     * function returns (-1,-1).
     */
    static QPoint calculatePointAtDistance(const QPoint &P1, const QPoint &P2, float Distance);

    /**
     * Calculates which point of a perpendicular line to segment P1P2 that
     * contains P2 has a distance equal to Distance from P2.
     * Let's say such point is PX, the distance from P2 to PX must be equal
     * to Distance.
     */
    static QPoint calculatePointAtDistanceOnPerpendicular(const QPoint &P1, const QPoint &P2, float Distance);

    /**
     * Calculates the intersection between line P1P2 and a perpendicular
     * line containing P3, the result is returned in ResultingPoint.
     * The result value represents the distance between ResultingPoint and
     * P3. If this value is negative an error ocurred.
     * This method is not currently used.
     */
    static float perpendicularProjection(const QPoint& P1, const QPoint& P2, const QPoint& P3, QPoint& ResultingPoint);

    /**
     * Return the mid point between p0 and p1
     */
    static QPoint midPoint(const QPoint& p0, const QPoint& p1);

    /**
     * Calculates the position of the text widget depending on the role
     * that widget is playing.
     * Returns the point at which to put the widget.
     */
    QPoint calculateTextPosition(Uml::TextRole role);

    /**
     * Puts the text widget with the given role at the given position.
     * This method calls @ref calculateTextPostion to get the needed position.
     * I.e. the line segment it is on has moved and it should move the same
     * amount as the line.
     */
    void setTextPosition(Uml::TextRole role);

    /**
     * Moves the text widget with the given role by the difference between
     * the two points.
     */
    void setTextPositionRelatively(Uml::TextRole role, const QPoint &oldPosition);

    /**
     * Returns the Region the widget to line intersection is for the given
     * widget in this Association.  If the given widget is not in the
     * Association then Region::Error is returned.
     * Used by @ref calculateEndingPoints to work these positions out for
     * another Association - since the number of Associations on the same
     * region for the same widget will mean the lines will need to be
     * spread out across the region.
     */
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

    /**
     * Change, create, or delete the FloatingTextWidget indicated by the given Text_Role.
     *
     * @param tr    Text_Role of the FloatingTextWidget to change or create.
     * @param text  Text string that controls the action:
     *              If empty and ft is NULL then setFloatingText() is a no-op.
     *              If empty and ft is non-NULL then the existing ft is deleted.
     *              If non-empty and ft is NULL then a new FloatingTextWidget is created
     *              and returned in ft with the text set.
     *              If non-empty and ft is non-NULL then the existing ft text is modified.
     * @param ft    Reference to the pointer to FloatingTextWidget to change or create.
     *              On creation/deletion, the pointer value will be changed.
     */
    void setFloatingText(Uml::TextRole tr, const QString &text, FloatingTextWidget* &ft);

    /**
     * Called to tell the association that another association has added
     * a line to the region of one of its widgets. The widget is identified
     * by its role (A or B).
     *
     * Called by @ref updateAssociations which is called by
     * @ref calculateEndingPoints when required.
     */
    void updateRegionLineCount(int index, int totalCount,
                               AssociationWidget::Region region, Uml::Role_Type role);

    /**
     * Tells all the other view associations the new count for the
     * given widget on a certain region. And also what index they should be.
     */
    void updateAssociations(int totalCount, Region region, Uml::Role_Type role);

    /**
     * Returns the number of lines there are on the given region for
     * either widget A or B of the association.
     */
    int getRegionCount(Region region, Uml::Role_Type role);

    /**
     * Initialize attributes of this class at construction time.
     */
    void init();

    /**
     * Auxiliary method for calculateEndingPoints().
     */
    void doUpdates(int otherX, int otherY, Uml::Role_Type role);

    /**
     * For internal purposes only.
     * Other classes/users should use setChangeability() instead.
     */
    void setChangeWidget(const QString &strChangeWidget, Uml::Role_Type role);

    /**
     * Checks to see if the given point is one of the points of the line.
     * If so will try and get the view to flag the point for moving.
     * This is only valid if no other point id being moved and only
     * while the left mouse button is down.
     */
    void checkPoints(const QPoint &p);

    /**
     * Returns true if the line path starts at the given widget.
     */
    bool linePathStartsAt(const UMLWidget* widget);

    /**
     * Auxiliary method for updateAssociations():
     * Put position into m_positions and assoc into m_ordered at the
     * correct index.
     * m_positions and m_ordered move in parallel and are sorted by
     * ascending position.
     */
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
     * m_LinePath[m_unNameLineSegment] -- m_LinePath[m_unNameLineSegment+1]
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
    UMLSceneLine *m_pAssocClassLine;  ///< used for connecting assoc. class
    /// selection adornment for the endpoints of the assoc. class connecting line
    UMLSceneRectangle *m_pAssocClassLineSel0, *m_pAssocClassLineSel1;

    ClassifierWidget *m_pAssocClassWidget;  ///< used if we have an assoc. class

    /**
     * The definition points for the association line.
     */
    AssociationLine m_LinePath;

    // The following items are only used if m_pObject is not set.
    Uml::AssociationType m_AssocType;

public slots:
    /**
     * Handles the selection from the popup menu.
     */
    void slotMenuSelection(QAction* action);

    /**
     * This slot is entered when an event has occurred on the views display,
     * most likely a mouse event.  Before it sends out that mouse event all
     * children should make sure that they don't have a menu active or there
     * could be more than one popup menu displayed.
     */
    void slotRemovePopupMenu();

    /**
     * Handles any signals that tells everyone not to be selected.
     */
    void slotClearAllSelected();

    /**
     * Connected to UMLClassifier::attributeRemoved() or UMLEntity::constraintRemoved()
     * in case this AssociationWidget is linked to a clasifier list item
     * ( an attribute or a foreign key constraint )
     *
     * @param obj               The UMLClassifierListItem removed.
     */
    void slotClassifierListItemRemoved(UMLClassifierListItem* obj);


    /**
     * Connected to UMLObject::modified() in case this
     * AssociationWidget is linked to a classifer's attribute type.
     */
    void slotAttributeChanged();

    /**
     * Synchronize this widget from the UMLAssociation.
     */
    void syncToModel();
};
#endif
