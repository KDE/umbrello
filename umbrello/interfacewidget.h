/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INTERFACEWIDGET_H
#define INTERFACEWIDGET_H

#include "umlwidget.h"

class UMLView;

#define INTERFACE_MARGIN 5

/**
 *	Defines a graphical version of the interface.  Most of the functionality
 *	will come from the @ref UMLWidget class from which class inherits from.
 *
 *	@short	A graphical version of an interface.
 *	@author Jonathan Riddell
 *	@see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class InterfaceWidget : public UMLWidget {
public:

	/**
	 *	Constructs a InterfaceWidget.
	 *
	 *	@param	view	The parent of this InterfaceWidget.
	 *	@param	o	The UMLObject this will be representing.
	 */
	InterfaceWidget(UMLView * view, UMLObject * o);

	/**
	 *	Constructs a InterfaceWidget.
	 *
	 *	@param	view	The parent of this InterfaceWidget.
	 */
	InterfaceWidget(UMLView * view);

	/**
	 *	Standard deconstructor.
	 */
	~InterfaceWidget();

	/**
	 *	Initializes key variables of the class.
	 */
	void init();

	/**
	 *	Return the status of showing operations.
	 *
	 *	@return	Return the status of showing operations.
	 */
	bool getShowOps() const;

	/**
	 * toggles the status of showing operations
	 */
	void toggleShowOps();

	/**
	 *	Return the status of showing operation signatures.
	 *
	 *	@return Returns the status of showing operation signatures.
	 */
	Uml::Signature_Type getShowOpSigs() const;

	/**
	 * toggles the status of showing operation signatures
	 */
	void toggleShowOpSigs();

	/**
	 *	Set the status of whether to show scope
	 *
	 *	@param	_scope Status to whether to show scope.
	 */
	void setShowScope(bool _scope);

	/**
	 *	Returns the status of whether to show scope.
	 *
	 *	@return  Returns the status of whether to show scope.
	 */
	bool getShowScope() const;

	/**
	 * toggles the status of wether to show scope
	 */
	void toggleShowScope();

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
	 *	Returns the status of whether to show Package.
	 *
	 *	@return  Returns the status of whether to show Package.
	 */
	bool getShowPackage() const;

	/**
	 * toggles the status of wether to show package.
	 */
	void toggleShowPackage();

	/**
	 * 	Set the status of whether to show Package
	 *
	 *	@param _show  The status of whether to show Package
	 */
	void setShowPackage(bool _status);

	/**
	 *	Returns the status of whether to draw as circle.
	 *
	 *	@return  Returns the status of whether to draw as circle.
	 */
	bool getDrawAsCircle() const;

	/**
	 * toggles the status of wether to draw as circle
	 */
	void toggleDrawAsCircle();

	/**
	 * 	Set the status of whether to draw as circle.
	 *
	 *	@param _show  The status of whether to draw as circle.
	 */
	void setDrawAsCircle(bool drawAsCircle);

	/**
	 * Activate the object after serializing it from a QDataStream
	 */
	virtual bool activate(IDChangeLog* ChangeLog  = 0 );

	/**
	 *	Overrides standards method
	 */
	void draw(QPainter& p, int offsetX, int offsetY);

	/**
	 * Overrides default impl. to set e.g. the abstract attribute.
	 */
	virtual void setUMLObject(UMLObject* object);

	/**
	 * Saves to the <interfacewidget> element
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads from an <interfacewidget> element
	 */
	bool loadFromXMI( QDomElement & qElement );

protected:
	// Data loaded/saved
	bool m_bShowOperations;
	bool m_bShowScope;
	bool m_bShowPackage;
	bool m_bDrawAsCircle;
	Uml::Signature_Type m_ShowOpSigs;

private:
	/**
	 * Sets e.g. the abstract attribute.
	 */
	void initUMLObject(UMLObject* object);

	/**
	 * draws the interface as a circle with name underneith
	 */
	void drawAsCircle(QPainter& p, int offsetX, int offsetY);

	/**
	 * draws the interface as a concept, in a box with operations
	 */
	void drawAsConcept(QPainter& p, int offsetX, int offsetY);

	/**
	 *	Automatically calculates the size of the object.
	 */
	void calculateSize();

	/**
	 * Automatically calculates the size of the object when drawn as a circle
	 */
	void calculateAsCircleSize();

	/**
	 * Automatically calculates the size of the object when drawn as a concept
	 */
	void calculateAsConceptSize();

	/**
	 * updates m_ShowOpSigs to match m_bShowScope
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
