/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLWIDGETDATA_H
#define UMLWIDGETDATA_H

#include "associationwidgetlist.h"
#include "dialogs/settingsdlg.h"
#include "umlnamespace.h"

#include <qdom.h>
#include <qfont.h>

class QDataStream;
class AssociationWidget;

using namespace Uml;

/**	This class holds all the UMLWidget's Information All this
 *	information goes to a file or clipboard when a UMLWidget
 *	object is saved With this class we are trying to achieve
 *	isolation between data and display layers.
 *
 *      @author Gustavo Madrigal
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLWidgetData {
public:
	/**
	 * Constructor
	 * this constructor does not take the option state for the diagram and does not setup
	 * the colours.  It is used only by FloatingText
	 */
	UMLWidgetData();

	/**
	 *   Constructor 
	 *   @param optionState the optionState of the diagram.  Used to set the initial colours.
	 */
	UMLWidgetData(SettingsDlg::OptionState optionState);

	/**
	 *		Copy constructor
	 */
	UMLWidgetData(UMLWidgetData &Other);

	/**
	 *  	Deconstructor
	 */
	virtual ~UMLWidgetData();

	/**
	 * 	Overload '=' operator
	 */
	virtual UMLWidgetData & operator=(const UMLWidgetData & Other);

	/**
	 * 	Overload '==' operator
	 */
	virtual bool		operator==(const UMLWidgetData & Other);

	/**
	*	 Write property of m_Type.
	*/
	virtual void setType( UMLWidget_Type Type);

	/**
	*	 Read property of m_Type.
	*/
	virtual UMLWidget_Type getType();

	/**
	*	 Write property of m_nId.
	*/
	virtual void setID( int id ) { m_nId = id; }

	/**
	*	 Read property of m_nId.
	*/
	virtual int getID() { return m_nId; }

	/**
	*	 Write property of int m_nX.
	*/
	virtual void setX( int X);

	/**
	 * Read property of int m_nX.
	 */
	virtual int getX();

	/**
	 * Write property of int m_nY.
	 */
	virtual void setY( int Y);

	/**
	 * Read property of int m_nY.
	 */
	virtual int getY();

	/**
	 *		Returns the height of widget.
	 *		Only valid after load.
	 */
	virtual int getHeight() {
		return m_nHeight;
	}

	/**
	 *		Returns the width of the widget.
	 *		Only valid after load.
	 */
	virtual int getWidth() {
		return m_nWidth;
	}

	/**
	 *		Sets the width of the widget.
	 *		Sets before save/copy.
	 */
	virtual void setWidth( int width ) {
		m_nWidth = width;
	}

	/**
	 *		Sets the height of the widget.
	 *		Sets before save/copy.
	 */
	virtual void setHeight( int height ) {
		m_nHeight = height;
	}

	/**
	 * Write property of int m_Id.
	 */
	virtual void setId( int Id);

	/**
	 * Read property of int m_Id.
	 */
	virtual int getId();

	/**
	 * Write property of bool m_bUseFillColor.
	 */
	virtual void setUseFillColor( bool UseFillcolor);

	/**
	 * Read property of bool m_bUseFillColor.
	 */
	virtual bool getUseFillColor();

	/**
	 * Write property of QColor m_LineColour.
	 */
	virtual void setLineColour(QColor colour);

	/**
	 * Read property of QColor m_LineColour.
	 */
	virtual QColor getLineColour();

	/**
	 * Write property of QColor m_FillColor.
	 */
	virtual void setFillColour(QColor colour);

	/**
	 * Read property of QColor m_FillColor.
	 */
	virtual QColor getFillColour();

	/**
	 * Read property of int m_nNumAssoc.
	 */
	virtual uint getNumAssoc();

	/**
	 * Prints the data members to standard error
	 */
	virtual void print2cerr();

	/**
	 * Removes an already created association from the list of
	 * associations that include this UMLWidget
	 */
	void removeAssoc(AssociationWidget* pAssoc);

	/**
	 * Adds an already created association to the list of
	 * associations that include this UMLWidget
	 */
	void addAssoc(AssociationWidget* pAssoc);

	/**
	 * 		Returns the list of associations connected to this widget.
	 */
	AssociationWidgetList & getAssocList() {
		return m_Assocs;
	}

	/**
	 *  	Sets the font the widget is to use.
	 */
	virtual void setFont( QFont font ) {
		m_Font = font;
	}

	/**
	 * 		Returns the font the widget is too use.
	 */
	virtual QFont getFont() {
		return m_Font;
	}

	/**
	 * Returns m_bUsesDiagramFillColour
	 */
	bool getUsesDiagramFillColour();

	/**
	 * Returns m_bUsesDiagramLineColour
	 */
	bool getUsesDiagramLineColour();

	/**
	 * Returns m_bUsesDiagramUseFillColour
	 */
	bool getUsesDiagramUseFillColour();

	/**
	 * Sets m_bUsesDiagramFillColour
	 */
	void setUsesDiagramFillColour(bool usesDiagramFillColour);

	/**
	 * Sets m_bUsesDiagramLineColour
	 */
	void setUsesDiagramLineColour(bool usesDiagramLineColour);

	/**
	 * Sets m_bUsesDiagramUseFillColour
	 */
	void setUsesDiagramUseFillColour(bool usesDiagramUseFillColour);

	/**
	 * Write property of bool m_bIsInstance
	 */
	virtual void setIsInstance(bool isInstance);

	/**
	 * Read property of bool m_bIsInstance
	 */
	virtual bool getIsInstance();

	/**
	 * Write property of bool m_bIsInstance
	 */
	virtual void setInstanceName(QString instanceName);

	/**
	 * Read property of bool m_bIsInstance
	 */
	virtual QString getInstanceName();

	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	virtual bool loadFromXMI( QDomElement & qElement );
protected:
	/**
	 *		Type of widget.
	 */
	UMLWidget_Type m_Type;

	/**
	 * X Position of the UMLWidget that holds this instance
	 */
	int m_nX;

	/**
	 * Y Position of the widge
	 */
	int m_nY;

	/**
	 *		Width of width
	 */
	int m_nWidth;

	/**
	 *		Height of width
	 */
	int m_nHeight;

	/**
	 * This UMLObjectID of the UMLObject related to the UMLWidget holding
	 *	this UMLWidgetData Instance
	 */
	int m_nId;

	/**
	 * This flag indicates if the UMLWidget holding this UMLWidgetData instance
	 *	uses the Diagram FillColor
	 */
	bool m_bUseFillColor;

	/**
	 *  true by default, false if the colours have
	 *  been explicity set for this widget
	 */
	bool m_bUsesDiagramFillColour, m_bUsesDiagramLineColour, m_bUsesDiagramUseFillColour;

	/**
	 * Colour of the lines of the widget
	 */
	QColor m_LineColour;

	/**
	 * Colour of the background of the widget
	 */
	QColor m_FillColour;

	/**
	 *	 A list of AssociationWidgets between the UMLWidget holding this UMLWidgetData
	 *	instance and other UMLWidgets in the diagram
	 */
	AssociationWidgetList m_Assocs;

	/**
	 * 	The font the widget will use.
	 */
	QFont m_Font;

	/**
	 * Holds whether this widget is a component instance (i.e. on a deployment diagram)
	 */
	bool m_bIsInstance;

	/**
	 * The instance name (used if on a deployment diagram)
	 */
	QString m_instanceName;
};

#endif
