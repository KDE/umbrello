/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSWIDGET_H
#define CLASSWIDGET_H

#include "classwidgetdata.h"
#include "umlwidget.h"
#include <qsize.h>

class UMLView;

#define MARGIN 5

/**
 *	Defines a graphical version of the Class.  Most of the functionality
 *	will come from the @ref UMLWidget class from which class inherits from.
 *
 *	@short	A graphical version of a Class.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version	1.0
 *	@see	UMLWidget
 */
class ClassWidget : public UMLWidget {
public:
	/**
	 *	Constructs a ClassWidget.
	 *
	 *	@param	view		The parent of this ClassWidget.
	 *	@param	pData		The ClassWidgetData to represent.
	 */
	ClassWidget(UMLView * view, UMLObject *o, UMLWidgetData *pData);

	/**
	 *	Constructs a ClassWidget.
	 *
	 *	@param	view		The parent of this ClassWidget.
	 *	@param	o				The UMLObject is will be representing.
	 */
	ClassWidget(UMLView * view, UMLObject *o);

	/**
	 *	Constructs a ClassWidget.
	 *
	 *	@param	view	The parent of this ClassWidget.
	 */
	ClassWidget(UMLView * view);

	/**
	 *	Standard deconstructor.
	 */
	~ClassWidget();

	/**
	 *	Initializes key variables of the class.
	 */
	void init();

	/**
	 *	Returns the status of showing attributes.
	 *
	 *	@return	Returns the status of showing attributes.
	 */
	bool getShowAtts() {
		return ((ClassWidgetData*)m_pData)->m_bShowAttributes;
	}

	/**
	 *	Return the status of showing operations.
	 *
	 *	@return	Return the status of showing operations.
	 */
	bool getShowOps() {
		return ((ClassWidgetData*)m_pData)->m_bShowOperations;
	}

	/**
	 *	Return the status of showing operation signatures.
	 *
	 *	@return Returns the status of showing operation signatures.
	 */
	Uml::Signature_Type getShowOpSigs() {
		return ((ClassWidgetData*)m_pData)->m_ShowOpSigs;
	}

	/**
	 *	Returns the status of showing attribute signatures.
	 *
	 *	@return	Returns the status of showing attribute signatures.
	 */
	Uml::Signature_Type getShowAttSigs() {
		return ((ClassWidgetData*)m_pData)->m_ShowAttSigs;
	}

	/**
	 *	Returns the status of whether to show scope.
	 *
	 *	@return  Returns the status of whether to show scope.
	 */
	bool getShowScope() {
		return ((ClassWidgetData*)m_pData)->m_bShowScope;
	}

	/**
	 *	Set the status of whether to show scope
	 *
	 *	@param	_scope Status to whether to show scope.
	 */
	void setShowScope(bool _scope);

	/**
	 * 	Set the status of whether to show Attributes
	 *
	 *	@param _show  The status of whether to show Attributes
	 */
	void setShowAtts(bool _show);

	/**
	 * 	Set the status of whether to show Attribute signature
	 *
	 *	@param _show  The status of whether to show Attribute signature
	 */
	void setShowAttSigs(bool _show);

	/**
	 * 	Set the status of whether to show Operation signature
	 *
	 *	@param _show  The status of whether to show Operation signature
	 */
	void setShowOpSigs(bool _show);

	/**
	 * 	Set the status of whether to show Operations
	 *
	 *	@param _show  The status of whether to show Operations
	 */
	void setShowOps(bool _show);

	/**
	 *	Set the type of signature to display for an Operation
	 *
	 *	@param	sig	the type of signature to display for an Operation
	 */
	void setOpSignature(Uml::Signature_Type sig);

	/**
	 *	Set the type of signature to display for an Attribute
	 *
	 *	@param	sig	the type of signature to display for an Attribute
	 */
	void setAttSignature(Uml::Signature_Type sig);

	/**
	 *	Returns the status of whether to show StereoType.
	 *
	 *	@return  Returns the status of whether to show StereoType.
	 */
	bool getShowStereotype() {
		return ((ClassWidgetData*)m_pData)->m_bShowStereotype;
	}

	/**
	 *	Returns the status of whether to show Package.
	 *
	 *	@return  Returns the status of whether to show Package.
	 */
	bool getShowPackage() {
		return ((ClassWidgetData*)m_pData)->m_bShowPackage;
	}

	/**
	 * 	Set the status of whether to show StereoType
	 *
	 *	@param _show  The status of whether to show StereoType
	 */
	void setShowStereotype(bool _status);

	/**
	 * 	Set the status of whether to show Package
	 *
	 *	@param _show  The status of whether to show Package
	 */
	void setShowPackage(bool _status);

	/**
	 * Activate the object after serializing it from a QDataStream
	 */
	virtual bool activate(IDChangeLog* ChangeLog  = 0 );

	/**
	 * Synchronizes the Widget's m_pData member with its display properties, for example:
	 * the X and Y positions of the widget, etc
	 */
	virtual void synchronizeData();

	/**
	 *	Overrides standards method
	 */
	void draw(QPainter & p, int offsetX, int offsetY);
private:

	/**
	 *	Automatically calculates the size of the object.
	 */
	void calculateSize();

	/**
	 * Calculcates the size of the templates box in the top left
	 * if it exists, returns QSize(0,0) if it doesn't.
	 */
	QSize calculateTemplatesBoxSize();

	/**
	 * updates m_ShowOpSigs and m_ShowAttSigs to match m_bShowScope
	 */
	void updateSigs();

	/**
	 * The right mouse button menu
	 */
	ListPopupMenu* m_pMenu;

public slots:
	/**
	 *	Will be called when a menu selection has been made from the popup menu.
	 *
	 *	@param	sel	The selection id that has been selected.
	 */
	void slotMenuSelection(int sel);
};

#endif
