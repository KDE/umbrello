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
 * object is serialized.  With this class we are trying to achieve
 * isolation between data and display layers.
 *
 * @author Jonathan Riddell
 */
class InterfaceWidgetData : public UMLWidgetData {
	friend class InterfaceWidget;
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
	 * Save/Restore the object.
	 *	@param	s	Pointer to the data stream (file) to save/load from.
	 *	@param	archive	If true will save the object information else will load the information
	 *	@param	fileversion	the version of the serialize format
	 *	@return	true if successful else false
	 */
	virtual bool serialize(QDataStream* s, bool archive, int fileversion);

	/**
	 * Returns the amount of bytes needed to serialize this object
	 * If the serialization method of this class is changed this function will have to be CHANGED TOO
	 * This function is used by the Copy and Paste Functionality
	 * The Size in bytes of a serialized QString Object is long sz:
	 *  if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() ) {
	 *      sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
	 *  }
	 * This calculation is valid only for QT 2.1.x or superior,
	 * this is totally incompatible with QT 2.0.x or QT 1.x or
	 * inferior
	 * That means the copy and paste functionality will work on with QT 2.1.x or superior
	 */
	virtual long getClipSizeOf();

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
