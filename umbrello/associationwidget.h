/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ASSOCIATIONWIDGET_H
#define ASSOCIATIONWIDGET_H

#include "umlnamespace.h"
#include "umlwidgetlist.h"
#include "messagewidgetlist.h"
#include "associationwidgetlist.h"
#include "linepath.h"

// qt includes
#include <qobject.h>

class FloatingText;
class IDChangeLog;
class ListPopupMenu;
class QBitmap;
class QPixmap;
class QDataStream;
class UMLView;
class UMLAssociation;

using namespace Uml;

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
 * @version $Revision$
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssociationWidget : public QObject {
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
	 * @param view		The parent view of this widget.
	 */
	AssociationWidget(UMLView *view);

	/**
	 * Constructor.
	 *
	 * @param view		The parent view of this widget.
	 * @param WidgetA	Pointer the role A widget for the association.
	 * @param Type		The Association_Type for this association.
	 * @param WidgetB	Pointer the role B widget for the association.
	 */
	AssociationWidget(UMLView *view, UMLWidget* WidgetA,
			  Association_Type Type, UMLWidget* WidgetB);

	/**
	 * Deconstructor.
	 */
	virtual ~AssociationWidget();

	/**
	 * Overrides the assignment operator.
	 */
	AssociationWidget& operator=(AssociationWidget & Other);

	/**
	 * Overrides the equality test operator.
	 */
	bool operator==(AssociationWidget & Other);

	/**
	 * Overrides the != operator.
	 */
	bool operator!=(AssociationWidget & Other);

	/**
	 * Activates the AssociationWidget after a load.
	 *
	 * @return	True if activation successful.
	 */
	bool activate();

	/**
	 * Write property of m_pWidgetA.
	 *
	 * @param widgetA	Pointer to the role A UMLWidget.
	 */
	void setWidgetA( UMLWidget* widgetA);

	/**
	 * Write property of m_pWidgetB.
	 *
	 * @param widgetB	Pointer to the role B UMLWidget.
	 */
	void setWidgetB( UMLWidget* widgetB);

	/**
	 * Read property of FloatingText* m_pMultiA.
	 *
	 * @return	Pointer to the multiA FloatingText.
	 */
	FloatingText* getMultiAWidget();

	/**
	 * Returns the m_pMultiA's text.
	 *
	 * @return	Text of the multiA widget.
	 */
	QString getMultiA() const;

	/**
	 * Read property of FloatingText* m_pMultiB.
	 *
	 * @return	Pointer to the multiB FloatingText.
	 */
	FloatingText* getMultiBWidget();

	/**
	 * Returns the m_pMultiB's text.
	 *
	 * @return	Text of the multiB widget.
	 */
	QString getMultiB() const;

	/**
	 * Read property of FloatingText* m_pName.
	 *
	 * @return	Pointer to the FloatingText name widget.
	 */
	FloatingText* getNameWidget();

	/**
	 * Returns the m_pName's text.
	 *
	 * @return	Text of the FloatingText name widget.
	 */
	QString getName() const;

	/**
	 * Read property of FloatingText* m_pRoleA.
	 *
	 * @return	Pointer to the role A FloatingText widget.
	 */
	FloatingText* getRoleAWidget();

	/**
	 * Read property of FloatingText* m_pRoleB.
	 *
	 * @return	Pointer to the role B FloatingText widget.
	 */
	FloatingText* getRoleBWidget();

	/**
	 * Returns the documentation about this association.
	 *
	 * @return	The AssociationWidget's documentation text.
	 */
	QString getDoc() const;

	/**
	 * Returns the m_pRoleA's text.
	 *
	 * @return	The name set at the roleA FloatingText.
	 */
	QString getRoleNameA() const;

	/**
	 * Returns the documentation about RoleA.
	 */
	QString getRoleADoc() const;

	/**
	 * Returns the m_pRoleB's text.
	 *
	 * @return	The name set at the roleB FloatingText.
	 */
	QString getRoleNameB() const;

	/**
	 * Returns the documentation about Role B.
	 */
	QString getRoleBDoc() const;

	/**
	 * Sets the text in the FloatingText widget representing the Name
	 * of this association.
	 */
	void setName (QString strRole);

	/**
	 * Sets the text in the FloatingText representing the Multiplicity
	 * at the ending side of the association.
	 */
	bool setMultiB(QString strMultiB);

	/**
	 * Sets the text in the FloatingText representing the Multiplicity
	 * at the starting side of the association.
	 */
	bool setMultiA(QString strMultiA);

	/**
	 * Gets the visibility on the rolename A end of the Association.
	 */
	Scope getVisibilityA () const;

	/**
	 * Gets the visibility on the rolename B end of the Association.
	 */
	Scope getVisibilityB () const;

	/**
	 * Sets the visibility on the rolename 'A' end of the Association.
	 */
	void setVisibilityA ( Scope visibility );

	/**
	 * Sets the visibility on the rolename 'B' end of the Association.
	 */
	void setVisibilityB (Scope visibility);

	/**
	 * Gets the changeability on the rolename A end of the Association.
	 */
	Changeability_Type getChangeabilityA() const;

	/**
	 * Gets the changeability on the rolename B end of the Association.
	 */
	Changeability_Type getChangeabilityB() const;

	/**
	 * Sets the changeability on the rolename 'B' end of the Association.
	 */
	void setChangeabilityA (Changeability_Type value);

	/**
	 * Sets the changeability on the rolename 'B' end of the Association.
	 */
	void setChangeabilityB (Changeability_Type value);

	/**
	 * Gets the ID of role A.
	 */
	int getWidgetAID() const;

	/**
	 * Gets the ID of role B.
	 */
	int getWidgetBID() const;

	/**
	 * Read property of UMLWidget* m_pWidgetA.
	 *
	 * @return	Pointer to the role A UMLWidget.
	 */
	UMLWidget* getWidgetA();

	/**
	 * Sets the associated widgets.
	 *
	 * @param widgetA	Pointer the role A widget for the association.
	 * @param assocType	The Association_Type for this association.
	 * @param widgetB	Pointer the role B widget for the association.
	 */
	bool setWidgets( UMLWidget* widgetA, Association_Type assocType, UMLWidget* widgetB);

	/**
	 * Read property of UMLWidget* m_pWidgetB.
	 *
	 * @return	Pointer to the role B UMLWidget.
	 */
	UMLWidget* getWidgetB();

	/**
	 * Returns true if this association associates widgetA to widgetB,
	 * otherwise it returns false.
	 *
	 * @param widgetA	Pointer the role A widget to check.
	 * @param widgetB	Pointer the role B widget to check.
	 * @return	True if widgetA and widgetB are associated.
	 */
	bool checkAssoc(UMLWidget * widgetA, UMLWidget *widgetB);

	/**
	 * Returns true if the Widget is either at the starting or ending side
	 * of the association.
	 *
	 * @return	True if widget plays role A or B in this assoc.
	 */
	bool contains(UMLWidget* widget);

	/**
	 * Gets the association's type.
	 *
	 * @return	This AssociationWidget's Association_Type.
	 */
	Association_Type getAssocType() const;

	/**
	 * Sets the association's type.
	 *
	 * @param type		The Association_Type to set.
	 */
	void setAssocType(Association_Type type);

	/**
	 * Returns a QString object representing this AssociationWidget.
	 *
	 * @return	Textual representation of the AssociationWidget.
	 */
	QString toString();

	/**
	 * Read property of bool m_bActivated.
	 *
	 * @return	True if this AssociationWidget has been activated.
	 */
	const bool isActivated();

	/**
	 * Set the m_bActivated flag of a widget but does not perform the
	 * activate method.
	 *
	 * @param active	The flag status to set.
	 */
	void setActivated(bool active /*=true*/);

	/**
	 * Sets the state of whether the widget is selected.
	 *
	 * @param _select	The state of whether the widget is selected.
	 */
	void setSelected(bool _select = true);

	/**
	 * Returns the state of whether the widget is selected.
	 *
	 * @return	Returns the state of whether the widget is selected.
	 */
	bool getSelected() const {
		return m_bSelected;
	}

	/**
	 * Returns a pointer to the association widget's line path.
	 */
	LinePath* getLinePath() {
		return &m_LinePath;
	}

	/**
	 * Adjusts the ending point of the association that connects to Widget
	 *
	 * @param widget	Pointer to the widget that was moved.
	 * @param x		New X coordinate of the widget.
	 * @param y		New Y coordinate of the widget.
	 */
	void widgetMoved(UMLWidget* widget, int x, int y);

	/**
	 * Calculates the m_unNameLineSegment value according to the new
	 * NameText topleft corner PT.
	 * It iterates through all LinePath's segments and for each one
	 * calculates the sum of PT's distance to the start point + PT's
	 * distance to the end point. The segment with the smallest sum will
	 * be the RoleTextSegment (if this segment moves then the RoleText
	 * will move with it). It sets m_unNameLineSegment to the start point
	 * of the chosen segment.
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
	 *  Returns true if the given point is on the Association.
	 */
	bool onAssociation(const QPoint & point);

	/**
	 * Moves all the mid points (all expcept start /end ) by the given amount.
	 */
	void moveMidPointsBy( int x, int y );

	/**
	 * Moves the entire association by the given offset.
	 */
	void moveEntireAssoc( int x, int y );

	/**
	 * Sets the new font for the text widgets used.
	 * Called by the text widgets when the menu was selected on one of them.
	 */
	void changeFont(QFont font);

	/**
	 * Returns the bounding rectangle of all segments of the association.
	 */
	QRect getAssocLineRectangle();

	/**
	 * Return the first font found being used by any child widget. (They
	 * could be different fonts, so this is a slightly misleading method.)
         */
	QFont getFont () const;

	/**
	 * Set all 'owned' child widgets to this font.
	 */
	void setFont (QFont font);


	/**
	 * Read property of FloatingText * m_pChangeWidgetA.
	 */
	FloatingText* getChangeWidgetA();

	/**
	 * Read property of FloatingText * m_pChangeWidgetB.
	 */
	FloatingText* getChangeWidgetB();


	/**
	 * Sets the text to the FloatingText that display the Role text of this
	 * association.
	 * For this funtion to work properly, the associated widgets
	 * (m_pWidgetA and m_pWidgetB) should already be set.
	 */
	bool setRoleNameA(QString strRole);
	bool setRoleNameB(QString strRole);

	/**
	 * Set the documentation on this association.
	 */
	void setDoc(QString doc);

	/**
	 * Set the documentation on role A.
	 */
	void setRoleADoc(QString doc);

	/**
	 * Set the documentation on role B.
	 */
	void setRoleBDoc(QString doc);

	/**
	 * Returns the UMLAssociation representation of this object.
	 *
	 * @return	Pointer to the UMLAssociation that is represented by
	 *		this AsociationWidget.
	 */
	UMLAssociation * getAssociation ();

	/**
	 * Calls @ref setTextPosition() on all the labels.
	 */
	void resetTextPositions();

	/**
	 * Sets the Association line index for widgetA.
	 */
	void setIndexA(int index);

	/**
	 * Returns the Association line index for widgetA.
	 */
	int getIndexA() const;

	/**
	 * Sets the Association line index for widgetB.
	 */
	void setIndexB(int index);

	/**
	 * Returns the Association line index for widgetB.
	 */
	int getIndexB() const;

	/**
	 * Sets the total count on the Association region for widgetA.
	 */
	void setTotalCountA(int count);

	/**
	 * Returns the total count on the Association region for widgetA.
	 */
	int getTotalCountA() const;

	/**
	 * Sets the total count on the Association region for widgetB.
	 */
	void setTotalCountB(int count);

	/**
	 * Region the total count on the Association region for widgetB
	 */
	int getTotalCountB() const;

	/**
	 * Calculates and sets the first and last point in the association's
	 * LinePath.
	 * Each point is a middle point of its respective UMLWidget's bounding
	 * rectangle.
	 * This method picks which sides to use for the association.
	 */
	void calculateEndingPoints();

	/**
	 * Saves this widget to the <UML:AssociationWidget> XMI element.
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads this widget from the <UML:AssociationWidget> XMI element.
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

private:

	/** set our internal umlAssociation */
	void setUMLAssociation (UMLAssociation * assoc);

        /**
         * Cleans up all the association's data in the related widgets.
         */
        void cleanup();

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
	 * @param Rect		The bounding rectangle to investigate.
	 * @param PosX		X coordinate of the point to seek.
	 * @param PosY		Y coordinate of the point to seek.
	 * @return	The region number of the region containing the point.
	 *		1 = Region 1
	 *		2 = Region 2
	 *		3 = Region 3
	 *		4 = Region 4
	 *		5 = On diagonal 2 between Region 1 and 2
	 *		6 = On diagonal 1 between Region 2 and 3
	 *		7 = On diagonal 2 between Region 3 and 4
	 *		8 = On diagonal 1 between Region4 and 1
	 *		9 = On diagonal 1 and On diagonal 2 (the center)
	 */
	static Region findPointRegion(QRect Rect, int PosX, int PosY);

	/**
	 * Given a rectangle and a point, findInterceptOnEdge computes the
	 * connecting line between the middle point of the rectangle and
	 * the point, and returns the intercept of this line with the
	 * the edge of the rectangle identified by `region'.
	 * When the region is North or South, the X value is returned (Y is
	 * constant.)
	 * When the region is East or West, the Y value is returned (X is
	 * constant.)
	 */
	static int findInterceptOnEdge(QRect rect, Region region, QPoint point);

	/**
	 * Overrides moveEvent.
	 */
	void moveEvent(QMoveEvent *me);

	/**
	 * This function calculates which role should be set for the m_pName
	 * FloatingText.
	 */
	Text_Role CalculateNameType(Text_Role defaultRoleType);

	/**
	 * Returns true if point (PosX, PosY) is close enough to any of the
	 * association's segments.
	 */
	bool isPointInsideBoundaries(int PosX, int PosY, QPoint & SPoint,
				     uint & StartSegmentIndex, uint & EndSegmentIndex);

	/**
	 * Returns a point with interchanged X and Y coordinates.
	 */
	static QPoint swapXY(QPoint p);

	/**
	 * Returns the total length of the association's LinePath:
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
	static QPoint calculatePointAtDistance(QPoint P1, QPoint P2, float Distance);

	/**
	 * Calculates which point of a perpendicular line to segment P1P2 that
	 * contains P2 has a distance equal to Distance from P2.
	 * Let's say such point is PX, the distance from P2 to PX must be equal
	 * to Distance.
	 */
	static QPoint calculatePointAtDistanceOnPerpendicular(QPoint P1, QPoint P2, float Distance);

	/**
	 * Calculates the intersection between line P1P2 and a perpendicular
	 * line containing P3, the result is returned in ResultingPoint.
	 * The result value represents the distance between ResultingPoint and
	 * P3. If this value is negative an error ocurred.
	 * This method is not currently used.
	 */
	static float perpendicularProjection(QPoint P1, QPoint P2, QPoint P3, QPoint& ResultingPoint);

	/**
	 * Calculates the position of the text widget depending on the role
	 * that widget is playing.
	 * Returns the point at which to put the widget.
	 */
	QPoint calculateTextPosition(Text_Role role);

	/**
	 * Puts the text widget with the given role at the given position.
	 * This method is usually called after calling @ref calculateTextPostion
	 * to get the needed postion.  It is a separate function in case there
	 * is a need to place the text without calculation.
	 * I.e. the line segment it is on has moved and it should move the same
	 * amount as the line.
	 */
	void setTextPosition(Text_Role role, QPoint pos);

	/**
	 * Moves the text widget with the given role by the difference between
	 * the two points.
	 */
	void setTextPositionRelatively(Text_Role role, QPoint pos, QPoint oldPosition);

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
	FloatingText* m_pName;

	/**
	 * The WidgetRole struct gathers all information pertaining to the role.
	 * The AssociationWidget class contains two WidgetRole objects, one for each
	 * side of the association (A and B).
	 */
	struct WidgetRole {

		/**
		 * This is a pointer to the Floating Text widget at the role's side
		 * of the association.
		 * This FloatingText displays the information regarding multiplicity.
		 */
		FloatingText* m_pMulti;

		/**
		 * This is a pointer to the Floating Text widget at the role's side
		 * of the association.
		 * This FloatingText displays the information regarding changeability.
		 */
		FloatingText* m_pChangeWidget;

		/**
		 * This member holds a pointer to the floating text that displays
		 * the role's label of this association.
		 */
		FloatingText* m_pRole;

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

		// The following items are only used if m_pAssociation is not set.
		Scope m_Visibility;
		Changeability_Type m_Changeability;
		QString m_RoleDoc;
		QString m_RoleName;
		QString m_Multi;

	} m_role[2];

	/**
	 * Constants used for indexing the roles. 
	 */
	enum Role_Type { A, B };

	/**
	 * Called to tell the association that another association has added
	 * a line to the region of one of its widgets. The widget is identified
	 * by its role (A or B).
	 *
	 * Called by @ref updateAssociations which is called by
	 * @ref calculateEndingPoints when required.
	 */
	void updateRegionLineCount(int index, int totalCount,
				   AssociationWidget::Region region, Role_Type role);

	/**
	 * Tells all the other view associations the new count for the
	 * given widget on a certain region. And also what index they should be.
	 */
	void updateAssociations(int totalCount, Region region, Role_Type role);

	/**
	 * Returns the number of lines there are on the given region for
	 * either widget A or B of the association.
	 */
	int getRegionCount(Region region, Role_Type role);

	/**
	 * Initialize attributes of this class at construction time.
	 */
	void init (UMLView *view);

	/**
	 * Auxiliary method for calculateEndingPoints().
	 */
	void doUpdates(int otherX, int otherY, Role_Type role);

	/**
	 * Auxiliary to setMulti{A,B}.
	 */
	void setMulti(QString strMulti, Role_Type role);

	/**
	 * Auxiliary to setRoleName{A,B}.
	 */
	bool setRoleName (QString strRole, Role_Type role);

	/**
	 * Auxiliary to setVisibility{A,B}.
	 */
	void setVisibility (Scope value, Role_Type role);

	/**
	 * Auxiliary to setChangeability{A,B}.
	 */
	void setChangeability (Changeability_Type value, Role_Type role);

	/**
	 * For internal purposes only.
	 * Other classes/users should use setChangeability[AB] instead.
	 */
	void setChangeWidget(QString strChangeWidget, Role_Type role);

	/**
	 * Auxiliary to getWidget{A,B}ID.
	 */
	int getWidgetID(Role_Type role) const;

	/**
	 * Checks to see if the given point is one of the points of the line.
	 * If so will try and get the view to flag the point for moving.
	 * This is only valid if no other point id being moved and only
	 * while the left mouse button is down.
	 */
	void checkPoints(QPoint p);

	/**
	 * Returns true if the line path starts at the given widget.
	 */
	bool linePathStartsAt(const UMLWidget* widget);

	/**
	 * Utility: Find the widget identified by the given ID in the given
	 * widget or message list.
	 * It's somewhat arbitrary that this utility is here as it has no
	 * dependencies whatsoever on AssociationWidget.  It just happens
	 * to be needed here.  We should pull it out from here if it also
	 * becomes needed somewhere else.
	 */
	static UMLWidget* findWidget(int id, const UMLWidgetList& widgets,
					     const MessageWidgetList* pMessages = NULL);

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

	/*QPixmap 	*m_pPix;
	QBitmap 	*m_pMask;*/

	UMLView 	* m_pView;

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
	uint 		m_unNameLineSegment;

	bool 		m_bFocus;
	ListPopupMenu 	*m_pMenu;
	UMLAssociation  *m_pAssociation;
	bool 		m_bSelected;
	int 		m_nMovingPoint;

	/**
	 * The definition points for the association line.
	 */
	LinePath m_LinePath;

	// The following items are only used if m_pAssociation is not set.
	QString m_Doc;
	Uml::Association_Type m_AssocType;

public slots:
	/**
	 * Handles the selection from the popup menu.
	 */
	void slotMenuSelection(int sel);

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
	 * Merge together the UMLAssociation representation and this widget
	 */
	void mergeUMLRepresentationIntoAssociationData();
};
#endif
