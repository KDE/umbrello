/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INTERFACEWIDGETDATA_H
#define INTERFACEWIDGETDATA_H

#include "umlwidgetdata.h"

/**
 * This class holds all the InterfaceWidget's information. All this
 * information goes to a file or clipboard when an InterfaceWidget
 * object is saved.  With this class we are trying to achieve
 * isolation between data and display layers.
 *
 * @author Jonathan Riddell
 */
class InterfaceWidgetData : public UMLWidgetData {
public:
	/**
	 * creates an InterfaceWidgetData object
	 */
	InterfaceWidgetData(SettingsDlg::OptionState optionState);

	/**
	 * creates a copy of an InterfaceWidgetData object
	 */
	InterfaceWidgetData(InterfaceWidgetData& Other);

	/**
	 * destructor, empty
	 */
	virtual ~InterfaceWidgetData();

	/**
	 * Make the same as another InterfaceWidgetData
	 */
	virtual InterfaceWidgetData& operator=(InterfaceWidgetData& Other);

	/**
	 * Compare two InterfaceWidgetData.
	 */
	virtual bool operator==(InterfaceWidgetData& Other);

	/**
	 * Write property of bool m_bShowOperations.
	 */
	virtual void setShowOperations( bool ShowOperations);

	/**
	 * Read property of bool m_bShowOperations.
	 */
	virtual bool getShowOperations();

	/**
	 * Write property of UMLObject::SigType m_ShowOpSigs.
	 */
	virtual void setShowOpSigs( Uml::Signature_Type ShowOpSigs);

	/**
	 * Read property of UMLObject::SigType m_ShowOpSigs.
	 */
	virtual Uml::Signature_Type getShowOpSigs();

	/**
	 * Write property of bool m_bShowScope.
	 */
	virtual void setShowScope( bool ShowScope);

	/**
	 * Read property of bool m_bShowScope.
	 */
	virtual bool getShowScope();

	/**
	 * Write property of bool m_bShowPackage.
	 */
	virtual void setShowPackage( bool ShowPackage);

	/**
	 * Read property of bool m_bShowPackage.
	 */
	virtual bool getShowPackage();

	/**
	 * Write property of bool m_bDrawAsCircle.
	 */
	virtual void setDrawAsCircle(bool drawAsCircle);

	/**
	 * Read property of bool m_bDrawAsCircle.
	 */
	virtual bool getDrawAsCircle();

	/**
	 * Debugging method
	 */
	virtual void print2cerr();

	/**
	 * Saves to the <interfacewidget> element
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads from an <interfacewidget> element
	 */
	bool loadFromXMI( QDomElement & qElement );
protected:

	bool m_bShowOperations;
	bool m_bShowScope;
	bool m_bShowPackage;
	bool m_bDrawAsCircle;

	Uml::Signature_Type m_ShowOpSigs;
};

#endif
