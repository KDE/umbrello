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

#include "associationwidgetdata.h"
#include "umlnamespace.h"

class FloatingText;
class IDChangeLog;
class ListPopupMenu;
class QBitmap;
class QPixmap;
class QDataStream;
class UMLView;
class UMLWidget;

using namespace Uml;

/** This class represents an association
 * inside a diagram. It is the equivalent
 * to the original Association class
 * plus support for non straight lines
 *
 * @author Gustavo Madrigal
 * @short This class represents an association inside a diagram.
 */
class AssociationWidget : public QObject {
	Q_OBJECT
public:
	/**
	*   enumeration used for stating where a line is on a widget.
	*/
	enum Region{ Error = 0, West , North, East, South, NorthWest, NorthEast, SouthEast, SouthWest, Center };

	/**
	* Constructor
	*/
	AssociationWidget(QWidget *parent, AssociationWidgetData * pData = 0);

	/**
	* Constructor
	*/
	AssociationWidget(QWidget *parent, UMLWidget* WidgetA, Association_Type Type, UMLWidget* WidgetB);

	/**
	* Deconstructor
	*/
	virtual ~AssociationWidget();

	/**
	* Overrides = operator
	*/
	AssociationWidget& operator=(AssociationWidget & Other);

	/**
	*   Overrides == operator
	*/
	bool operator==(AssociationWidget & Other);

	/**
	* Overrides != operator
	*/
	bool operator!=(AssociationWidget & Other);

	/**
	*  Write property of FloatingText* m_pMultiA.
	*/
	virtual bool setMultiA( FloatingText* pMultiA);

	/**

	*   activates an associationwidget after a serialize function
	*/
	bool activate();

	/**
	* Read property of FloatingText* m_pMultiA.
	*/
	virtual  FloatingText* getMultiAWidget();

	/**
	 * Returns the m_pMultiA's text.
	 */
	virtual  QString getMultiA();

	/**
	* Write property of FloatingText* m_pMultiB.
	*/
	virtual bool setMultiB( FloatingText* pMultiB);

	/**
	* Read property of FloatingText* m_pMultiB.
	*/
	virtual  FloatingText* getMultiBWidget();

	/**
	* Returns the m_pMultiB's text.
	*/
	virtual  QString getMultiB();

	/**
	*  Write property of CFloatingText* m_pRole.
	*/
	virtual bool setRole( FloatingText* pRole);

	/**
	* Read property of CFloatingText* m_pRole.
	*/
	virtual  FloatingText* getRoleWidget();

	/**
	* Returns the m_pRole's text.
	*/
	virtual  QString getRole();

	/**
	* Sets the text to the FloatingText representing the Multiplicity at the ending side
	* of the association
	*/
	bool setMultiB(QString strMultiB);


	/**
	* Sets the text to the FloatingText representing the Multiplicity at the starting side
	* of the association
	*/
	bool setMultiA(QString strMultiA);

	/**
	 * Sets the text to the FloatingText that display the Role text of this association
	 * For this funtion to work properly, the associated widgets (m_pWidgetA and m_pWidgetB)
	 * should be already set
	 */
	bool setRole(QString strRole);

	/**
	 * Read property of UMLWidget* m_pWidgetA.
	 */
	virtual UMLWidget* getWidgetA();

	/**
	 * Set the associated widgets.
	 */
	virtual bool setWidgets( UMLWidget* WidgetA, Association_Type AssocType, UMLWidget* WidgetB);

	/**
	 * Read property of UMLWidget* m_pWidgetB.
	 */
	virtual UMLWidget* getWidgetB();

	/**
	 * Synchronizes the Widget's m_pData member with its display properties, for example:
	 * the X and Y positions of the widget, etc
	 */
	void synchronizeData();

	/**
	 * Returns true if this association associates WidgetA to WidgetB, otherwise it returns
	 *false
	 */
	bool checkAssoc(UMLWidget * WidgetA, UMLWidget *WidgetB);

	/**
	 * CleansUp all the association's data in the related widgets
	 */
	void cleanup();

	/**
	 * Returns true if the Widget is either at the starting or ending side of the association
	 */
	bool contains(UMLWidget* Widget);

	/**
	 * Returns the Association's Type
	 */
	Association_Type getAssocType();

	/**
	 * Sets the association's type
	 */
	void setAssocType(Association_Type Type);

	/**
	 * Returns a QString Object representing this AssociationWidget
	 */
	QString toString();

	/**
	 * Read property of bool m_bActivated.
	 */
	const bool isActivated();

	/**
	 * Set the m_bActivated flag of a widget but does not perform the Activate method
	 */
	void setActivated(bool Active /*=true*/);

	/**
	 *  Save/load the widget
	 *  @param	s	
	 *  @param	archive	if true will the classes information, else will load the information
	 *  @param	fileversion	the version of the serialize format
	 *  @return	true if successful else false
	 */
	virtual bool serialize(QDataStream *s, bool archive, int fileversion );

	/**
	 *Returns the amount of bytes needed to serialize this object
	 */
	long getClipSizeOf();

	/**
	 *	Sets the state of whether the widget is selected.
	 *
	 *	@param _select The state of whether the widget is selected.
	 */
	void setSelected(bool _select);

	/**
	 *	Returns the state of whether the widget is selected.
	 *
	 *	@return Returns the state of whether the widget is selected.
	 */
	bool getSelected() {
		return m_bSelected;
	}

	/**
	 * Return the widget data instance.
	 */
	AssociationWidgetData* getData();

	/**
	 * Adjusts the endding point of the association that connects to Widget
	 */
	void widgetMoved(UMLWidget* Widget, int x, int y);

	/**
	 * calculates the m_unRoleLineSegment value according to the new RoleText topleft
	 *  corner PT.
	 * It iterates through all LinePath's segments for each one it calculates the sum of
	 * PT's distance to the start point + PT's distance to the end point. The segment with
	 * the smallest sum will be the RoleTextSegment (if this segment moves then the
	 * RoleText will move with it). It sets m_unRoleLineSegment to the start point of the
	 * chosen segment
	 */
	void calculateRoleTextSegment();

	/**
	 * Overrides standard event method
	 */
	void mouseDoubleClickEvent(QMouseEvent * me);

	/**
	 *  Overrides standard event method
	 */
	void mousePressEvent(QMouseEvent * me);

	/**
	 *  Overrides standard event method
	 */
	void mouseReleaseEvent(QMouseEvent * me);

	/**
	 *  Overrides standard event method
	 */
	void mouseMoveEvent(QMouseEvent * me);


	/**
	 *   Returns true if the given point is ob the Association.
	 */
	bool onAssociation(const QPoint & point);

	/**
	 *  	Returns the associations documentation.
	 */
	QString getDoc() {
		return m_pData -> getDoc();
	}

	/**
	 *	Sets the associations documentation.
	 */
	void setDoc( QString doc) {
		m_pData -> setDoc( doc );
	}

	/**
	* 	Moves all the mid points (all expcept start /end ) by the given amount
	*/
	void moveMidPointsBy( int x, int y );

	/**
	* 	Moves the entire association by the given offset.
	*/
	void moveEntireAssoc( int x, int y );

	/**
	 *     Sets the new font for the text widgets used.  Called by the text widgets
	 *    when the menu was selected on one of them.
	 */
	void changeFont(QFont font);

	/**
	 * returns a rectangle where all segments of the assocication are in
	 */
	QRect getAssocLineRectangle();


private:
	/**
	 * Finds out in which region contains the Point (PosX, PosY) and returns the region
	 * number
	 */
	AssociationWidget::Region findPointRegion(QRect Rect, int PosX, int PosY);

	/**
	 *  Overrides moveEvent
	 */
	void moveEvent(QMoveEvent *me);

	/**
	 * This function calculates which role should be set for the m_pRole FloatingText
	 */
	Text_Role CalculateRoleType();

	/**
	 * Calculates and sets the first and last point in the Association's LinePath
	 * Each point is a middle point of its respecting UMLWidget's Bounding rectangle
	 * This method picks which sides to use for the association
	 */
	void calculateEndingPoints();

	/**
	 * Returns true if point (PosX, PosY) is close enough to any of association's segments
	 */
	bool isPointInsideBoundaries(int PosX, int PosY, QPoint & SPoint,
				     uint & StartSegmentIndex, uint & EndSegmentIndex);

	/**
	 * Returns the intersection point between line P1P2 and the bounding rectangle of pWidget
	 */
	QPoint findRectIntersectionPoint(UMLWidget* pWidget, QPoint P1, QPoint P2);

	/**
	 * Returns the intersection point between lines P1P2 and P3P4
	 */
	QPoint findIntersection(QPoint P1, QPoint P2, QPoint P3, QPoint P4);

	/**
	 * Returns the total lenght of the assocition's LinePath:
	 * result = segment_1_lenght + segment_2_lenght + ..... + segment_n_lenght
	 */
	float totalLength();

	/**
	 * Calculates which point of segment P1P2 has a distance equal to Distance from P1,
	 * Lets say such point is PX,  the distance from P1 to PX must be equal to Distance
	 * and if PX is not a point of the segment P1P2 then the function returns (-1,-1)
	 */
	QPoint calculatePointAtDistance(QPoint P1, QPoint P2, float Distance);

	/**
	 * Calculates which point of a perpendicular line to segment P1P2 that contains P2
	 * has a distance equal to Distance from P2,
	 * Lets say such point is PX,  the distance from P2 to PX must be equal to Distance
	 */
	QPoint calculatePointAtDistanceOnPerpendicular(QPoint P1, QPoint P2, float Distance);

	/**
	 * Calculates the intersection between line P1P2 and a perpendicular line containing
	 * P3, the result is returned in ResultingPoint. and result value represents the distance
	 * between ResultingPoint and P3; if this value is negative an error ocurred.
	 */
	float perpendicularProjection(QPoint P1, QPoint P2, QPoint P3, QPoint ResultingPoint);

	/**
	 * Calculates the position of the text widget depending on the role
	 * that widget is under-taking.  Returns the point at which to put the widget.
	 */
	QPoint calculateTextPosition(Text_Role role);

	/**
	 * Puts the text widget with the given role at the given position.
	 * This method is usually called after calling @ref calculateTextPostion
	 * to get the needed postion.  It is a seperate function incase there is a need
	 * to place the text without calculation.  i.e.  The line segment it is on has moved
	 * and it should move the same amount as the line.
	 */
	void setTextPosition(Text_Role role, QPoint pos);

	/**
	 * Shows the select operation dialog.
	 */
	void showOpDlg();

	/**
	 * Returns the Region the widget to line intersection is for the given widget
	 * in this Association.  If the given widget is not in the Association then
	 * Region::Error is returned.
	 * Used by @ref calculateEndingPoints to work these positions out for another
	 * Association, Since the number of Associations on the same region for the
	 * same widget will mean the lines will need to be spread out across the region.
	 */
	Region getWidgetRegion(AssociationWidget * widget);

	/**
	 * This is a pointer to the Floating Text widget at the starting side of the association.
	 * This FloatingText displays the information regarding multiplicity
	 */
	FloatingText* m_pMultiA;

	/**
	 * This is a pointer to the Floating Text widget at the ending side of the association.
	 * This FloatingText displays the information regarding multiplicity
	 */
	FloatingText* m_pMultiB;

	/**
	 * This member holds a pointer to the floating text that displays the role of this
	 * association
	 */
	FloatingText* m_pRole;

	/**
	 * This member holds a pointer to the UMLWidget at the starting side of the association
	 */
	UMLWidget* m_pWidgetA;

	/**
	 * This member holds a pointer to the UMLWidget at the ending side of the association
	 */
	UMLWidget* m_pWidgetB;

	/**
	 * WidgetA's Old TopLeft Corner before moving
	 */
	QPoint m_OldCornerA;

	/**
	 * WidgetB's Old TopLeft Corner before moving
	 */
	QPoint m_OldCornerB;

	/**
	 * Old WidgetA's TopLeft Region according to WidgetB's bounding rectangle
	 */
	int m_nCornerARegion;

	/**
	 * Old WidgetA's TopLeft Region according to WidgetB's bounding rectangle
	 */
	int m_nCornerBRegion;

	/**
	 * Returns the region the line is on for widgetA.
	 */
	Region getWidgetARegion() {
		return m_WidgetARegion;
	}

	/**
	 * Returns the region the line is on for widgetB.
	 */
	Region getWidgetBRegion() {
		return m_WidgetBRegion;
	}

	/**
	 * Called to tell the association that another association has added
	 * a line to the region of one of its' widgets. true - widgetA
	 * false - widgetB.
	 *
	 * Called by @ref updateAssociations which is called by
	 * @ref calculateEndingPoints when required.
	 */
	void updateRegionLineCount(int index, int totalCount, AssociationWidget::Region region , bool widgetA);

protected:

	/**
	 * Tells all the other view associations the new count for the
	 * given widget on a certain region. And also what index they should be.
	 */
	void updateAssociations(int totalCount, Region region, bool widgetA);

	/**
	 * Returns the number of lines there are on the given region for
	 * either widget a or b of the association.  Setting the boolean value
	 * to true will give a count for widgetA and false for widgetB.
	 */
	int getRegionCount(Region region, bool widgetA);

private:
	/**
	 *  Checks to see if the given point is one of the points of the line.
	 *  If so will try and get the view to flag the point for moving.
	 *  This is only valid if no other point id being moved and only
	 *  while the left mouse button is down.
	 */
	void checkPoints(QPoint p);

	/*QPixmap 	*m_pPix;
	QBitmap 	*m_pMask;*/

	UMLView 	* m_pView;
	AssociationWidgetData *m_pData;
	Region 		m_WidgetARegion, 
			m_WidgetBRegion;

	/**
	 * It is true if the Activate Function has been called for this
	 * class instance
	 */
	bool m_bActivated;

	/**
	 * When the association has a Role Floating Text this text should move when the
	 * LinePath moves but only if the closest segment to the role text moves.
	 * this segment is m_LinePath[m_unRoleLineSegment] -- m_LinePath[m_unRoleLineSegment + 1]
	 */
	uint 		m_unRoleLineSegment;
	bool 		m_bFocus;
	ListPopupMenu 	*m_pMenu;
	bool 		m_bSelected;
	int 		m_nMovingPoint;

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
	virtual void slotRemovePopupMenu();

	/**
	 * Handles any signals that tells everyone not to be selected.
	 */
	void slotClearAllSelected();
};
#endif
