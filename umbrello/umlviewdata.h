/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLVIEWDATA_H
#define UMLVIEWDATA_H

#include "associationwidgetdatalist.h"
#include "dialogs/settingsdlg.h"
#include "umlwidgetlist.h"

class UMLWidgetData;

class QColor;
class QDataStream;

using namespace Uml;

/**
 * @author Paul Hensgen
 */
class UMLViewData {
public:

	/**
	 * 		Constructor
	 */
	UMLViewData();

	/**
	 * 		Copy Constructor
	 */
	UMLViewData( UMLViewData & other );

	/**
	 * 		Overload '=' operator
	 */
	UMLViewData & operator=( UMLViewData & rhs );

	/**
	 * 		Overloaded '==' operator
	 */
	bool operator==( UMLViewData & rhs );

	/**
	 * 		Deconstructor
	 */
	~UMLViewData();

	/**
	 *		Return the documentation of the diagram.
	 */
	QString getDoc();

	/**
	 * 		Set the documentation of the diagram.
	 */
	void setDoc( QString doc );

	/**
	 * 		Return the name of the diagram.
	 */
	QString getName();

	/**
	 * 		Set the name of the diagram.
	 */
	void setName( QString name );

	/**
	 * 		Returns the type of the diagram.
	 */
	Uml::Diagram_Type getType();

	/**
	 * 		Set the type of diagram.
	 */
	void setType( Uml::Diagram_Type type );

	/**
	 * 		Returns the fill color to use.
	 */
	QColor getFillColor();

	/**
	 * 		Sets the fill color to use.
	 */
	void setFillColor( QColor color );

	/**
	 * 		Returns the line color to use.
	 */
	QColor getLineColor();

	/**
	 * 		Sets the line color to use.
	 */
	void setLineColor( QColor color );

	/**
	 * 		Returns the ID of the diagram.
	 */
	int getID();

	/**
	 * 		Sets the ID of the diagram.
	 */
	void setID( int id );

	/**
	 * 		Returns the zoom of the diagram.
	 */
	int getZoom();

	/**
	 * 		Sets the zoom of the diagram.
	 */
	void setZoom(int zoom);

	/**
	 * 		Returns the height of the diagram.
	 */
	int getCanvasHeight();

	/**
	 * 		Sets the height of the diagram.
	 */
	void setCanvasHeight(int height);

	/**
	 * 		Returns the width of the diagram.
	 */
	int getCanvasWidth();

	/**
	 * 		Sets the height of the diagram.
	 */
	void setCanvasWidth(int width);

	/**
	 * 		Returns an ID unique to this diagram.  ObjectWidgets will
	 *		need one of these.
	 */
	int getUniqueID();

	/**
	 * 		Return whether to use snap to grid.
	 */
	bool getSnapToGrid() {
		return m_bUseSnapToGrid;
	}

	/**
	 * 		Return whether to use snap to grid for component size.
	 */
	bool getSnapComponentSizeToGrid() {
		return m_bUseSnapComponentSizeToGrid;
	}

	/**
	 *		Returns the x grid size.
	 */
	int getSnapX() {
		return m_nSnapX;
	}

	/**
	 *		Returns the y grid size.
	 */
	int getSnapY() {
		return m_nSnapY;
	}

	/**
	 * 		Returns whether to show snap grid or not.
	 */
	bool getShowSnapGrid() {
		return m_bShowSnapGrid;
	}

	/**
	 * 		Sets whether to show snap grid.
	 */
	void setShowSnapGrid( bool bShow ) {
		m_bShowSnapGrid = bShow;
	}

	/**
	 *		Sets the x grid size.
	 */
	void setSnapX( int x) {
		m_nSnapX = x;
	}

	/**
	 *		Sets the y grid size.
	 */
	void setSnapY( int y) {
		m_nSnapY = y;
	}

	/**
	 * 		Sets whether to snap to grid.
	 */
	void setSnapToGrid( bool bSnap ) {
		m_bUseSnapToGrid = bSnap;
	}

	/**
	 * 		Sets whether to snap to grid for component size.
	 */
	void setSnapComponentSizeToGrid( bool bSnap ) {
		m_bUseSnapComponentSizeToGrid = bSnap;
	}

	/**
	 * Sets whether to use the fill/background color
	 */

	void setUseFillColor( bool ufc ) {
		m_Options.uiState.useFillColor = ufc;
	}

	/**
	 * Returns whether to use the fill/background color
	 */
	bool getUseFillColor() {
		return m_Options.uiState.useFillColor;
	}

	/**
	 *		Sets the font to use.
	 */
	void setFont( QFont font ) {
		m_Options.uiState.font = font;
	}

	/**
	 *		Returns the font to use
	 */
	QFont getFont() {
		return m_Options.uiState.font;
	}

	/**
	 *   Returns the options being used.
	 */
	SettingsDlg::OptionState getOptionState() {
		return m_Options;
	}

	/**
	 *		Sets the options to be used.
	 */
	void setOptionState( SettingsDlg::OptionState options) {
		m_Options = options;
	}

	/**
	 *   Returns a reference to the association list.
	 */
	AssociationWidgetDataList& getAssociationList() {
		return m_AssociationList;
	}

	/**
	 *   Returns a reference to the widget list.
	 */
	UMLWidgetDataList& getWidgetList() {
		return m_WidgetList;
	}

	/**
	 *   Returns a reference to the message list.
	 */
	UMLWidgetDataList& getMessageList() {
		return m_MessageList;
	}

	/**
	 * creates the <diagram> tag, and fills it with the contents oft he diagram
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <diagram> tag
	 */
	bool loadFromXMI( QDomElement & qElement );

	/**
	 * Loads a <widget> element such as <UML:Class>, used by loadFromXMI() and the clipboard
	 */
	UMLWidgetData* loadWidgetFromXMI(QDomElement& widgetElement);

protected:

	bool loadWidgetsFromXMI( QDomElement & qElement );

	bool loadMessagesFromXMI( QDomElement & qElement );

	bool loadAssociationsFromXMI( QDomElement & qElement );

	/**
	 * 		Contains the unique IDs to allocate to and widget that that needs an
	 *		ID for the view.  @ref ObjectWidgets are an example of this.
	 */
	int m_nLocalID;

	/**
	 *		The ID of the view.  Allocated by @ref UMLDoc
	 */
	int m_nID;

	/**
	 * 		The type of diagram to represent.
	 */
	Diagram_Type m_Type;

	/**
	 * 		The name of the diagram.
	 */
	QString m_Name;

	/**
	 * 		The documentation of the diagram.
	 */
	QString m_Documentation;

	/**
	 *		Options used by view
	 */
	SettingsDlg::OptionState	m_Options;

	/**
	 * 		Contains all the data items for @ref MessageWidgets on the diagram.
	 */
	UMLWidgetDataList m_MessageList;

	/**
	 *		Contains all the data for @ref UMLWidgets on the diagram.
	 */
	UMLWidgetDataList m_WidgetList;

	/**
	 * 		Contains all the data for @ref AssociationWidgets on the diagram.
	 */
	AssociationWidgetDataList m_AssociationList;

	/**
	 *	The snap to grid x size.
	 */
	int m_nSnapX;

	/**
	 *	The snap to grid y size.
	 */
	int m_nSnapY;

	/**
	 * 		Determines whether to use snap to grid.  The default is off.
	 */
	bool m_bUseSnapToGrid;

	/**
	 * 		Determines whether to use snap to grid for component 
	 * 		size.  The default is off.
	 */
	bool m_bUseSnapComponentSizeToGrid;

	/**
	 * 		Determines whether to show the snap grid.  The default will be on if the grid is on.
	 */
	bool m_bShowSnapGrid;

	/**
	 * The zoom level in percent, default 100
	 */
	int m_nZoom;

	/**
	 * Width of canvas in pixels
	 */
	int m_nCanvasWidth;

	/**
	 * Height of canvas in pixels
	 */
	int m_nCanvasHeight;

};

#endif
