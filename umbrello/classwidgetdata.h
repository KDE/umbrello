/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSWIDGETDATA_H
#define CLASSWIDGETDATA_H

#include "umlwidgetdata.h"

/**
 * This class holds all the ClassWidget's Information All this
 * information goes to a file or clipboard when an ClassWidget
 * object is saved.  With this class we are trying to achieve
 * isolation between data and display layers.
 *
 * @author Gustavo Madrigal
 */
class ClassWidgetData : public UMLWidgetData {
	friend class ClassWidget;
public:
	ClassWidgetData(SettingsDlg::OptionState optionState);
	ClassWidgetData(ClassWidgetData & Other);

	virtual ~ClassWidgetData();

	virtual ClassWidgetData & operator=(ClassWidgetData & Other);

	virtual bool operator==(ClassWidgetData & Other);

	/**
	 * Write property of bool m_bShowAttributes.
	 */
	virtual void setShowAttributes(bool ShowAttributes);

	/**
	 * Read property of bool m_bShowAttributes.
	 */
	virtual bool getShowAttributes();

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
	 * Write property of ClassWidgetData::SigType m_ShowAttSigs.
	 */
	virtual void setShowAttSigs( Uml::Signature_Type ShowAttSigs);

	/**
	 * Read property of ClassWidgetData::SigType m_ShowAttSigs.
	 */
	virtual Uml::Signature_Type getShowAttSigs();

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
	 * Write property of bool m_bShowStereotype.
	 */
	virtual void setShowStereotype( bool ShowStereotype);

	/**
	 * Read property of bool m_bShowStereotype.
	 */
	virtual bool getShowStereotype();

	/**
	 * Debugging method
	 */
	virtual void print2cerr();

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );
protected:

	bool m_bShowAttributes;
	bool m_bShowOperations;
	bool m_bShowScope;
	bool m_bShowPackage;
	bool m_bShowStereotype;

	Uml::Signature_Type m_ShowOpSigs;
	Uml::Signature_Type m_ShowAttSigs;
};

#endif
