 /*
  *  copyright (C) 2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSIFIERWIDGET_H
#define CLASSIFIERWIDGET_H

#include "umlwidget.h"

class QPainter;
class UMLClassifier;

/**
 * Intermediate class that serves as the base class for ClassWidget
 * and InterfaceWidget.  Does not itself appear on diagrams.
 *
 * @short Common base class for ClassWidget and InterfaceWidget
 * @author Oliver Kellogg
 * @see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassifierWidget : public UMLWidget {
public:

	/**
	 * Constructs a ClassifierWidget.
	 *
	 * @param view	The parent of this ClassifierWidget.
	 * @param o	The UMLObject to represent.
	 */
	ClassifierWidget(UMLView * view, UMLClassifier * o, Uml::Widget_Type wt);

	/**
	 * Destructor.
	 */
	virtual ~ClassifierWidget();

	/**
	 * Return the status of showing operations.
	 *
	 * @return	Return the status of showing operations.
	 */
	virtual bool getShowOps() const;

	/**
	 *  Set the status of whether to show Operations
	 *
	 * @param _show		True if operations shall be shown.
	 */
	virtual void setShowOps(bool _show);

	/**
	 * Toggles the status of showing operations.
	 */
	virtual void toggleShowOps();

	/**
	 * Return true if public operations/attributes are shown only.
	 */
	virtual bool getShowPublicOnly() const;

	/**
	 * Set whether to show public operations/attributes only.
	 */
	virtual void setShowPublicOnly(bool _status);

	/**
	 * Toggle whether to show public operations/attributes only.
	 */
	virtual void toggleShowPublicOnly();

	/**
	 * Returns the status of whether to show scope.
	 *
	 * @return	True if scope is shown.
	 */
	virtual bool getShowScope() const;

	/**
	 * Set the status of whether to show scope
	 *
	 * @param _scope	True if scope shall be shown.
	 */
	virtual void setShowScope(bool _scope);

	/**
	 * Toggles the status of whether to show scope
	 */
	virtual void toggleShowScope();

	/**
	 * Return the status of showing operation signatures.
	 *
	 * @return	Status of showing operation signatures.
	 */
	virtual Uml::Signature_Type getShowOpSigs() const;

	/**
	 * Set the status of whether to show Operation signature
	 *
	 * @param _show		True if operation signatures shall be shown.
	 */
	virtual void setShowOpSigs(bool _show);

	/**
	 * Toggles the status of showing operation signatures.
	 */
	virtual void toggleShowOpSigs();

	/**
	 * Returns the status of whether to show Package.
	 *
	 * @return	True if package is shown.
	 */
	virtual bool getShowPackage() const;

	/**
	 * Set the status of whether to show Package.
	 *
	 * @param _show		True if package shall be shown.
	 */
	virtual void setShowPackage(bool _status);

	/**
	 * Toggles the status of whether to show package.
	 */
	virtual void toggleShowPackage();

	/**
	 * Set the type of signature to display for an Operation
	 *
	 * @param sig	Type of signature to display for an operation.
	 */
	virtual void setOpSignature(Uml::Signature_Type sig);

	/**
	 * Return the number of displayed operations.
	 */
	int displayedOperations();

	/**
	 * Activate the object after serializing it from a QDataStream.
	 */
	virtual bool activate(IDChangeLog* ChangeLog  = 0 );

	/**
	 * Overrides standard method.
	 * Auxiliary to reimplementations in the derived classes.
	 */
	virtual void draw(QPainter & p, int offsetX, int offsetY);

protected:

	/**
	 * Initializes key variables of the class.
	 */
	virtual void init(Uml::Widget_Type wt);

	/**
	 * Calculcates the size of the templates box in the top left
	 * if it exists, returns QSize(0,0) if it doesn't.
	 *
	 * @return	QSize of the templates flap.
	 */
	QSize calculateTemplatesBoxSize();

	/**
	 * Abstract method for automatically computing the size of the object.
	 */
	virtual void calculateSize() = 0;

	/**
	 * Auxiliary method for calculateSize():
	 * Computes the width and height of the classifier widget
	 * taking into account the name, stereotype, and operations.
	 *
	 * @param width		Return value: the computed width.
	 * @param height	Return value: the computed height.
	 * @param showStereotype True to include the stereotype.
	 */
	virtual void computeBasicSize(int &width, int &height,
				      bool showStereotype = true);

	/**
	 * Updates m_ShowOpSigs to match m_bShowScope.
	 */
	virtual void updateSigs();

	/**
	 * Return the number of displayed members of the given Object_Type.
	 * Takes into consideration m_bShowPublicOnly but not other settings,
	 */
	int displayedMembers(Uml::Object_Type ot);

	/**
	 * Auxiliary method for draw() of child classes:
	 * Draw the attributes or operations.
	 *
	 * @param p		QPainter to paint to.
	 * @param ot		Object type to draw, either ot_Attribute or ot_Operation.
	 * @param sigType	Governs details of the member display.
	 * @param x		X coordinate at which to draw the texts.
	 * @param y		Y coordinate at which text drawing commences.
	 * @param fontHeight	The font height.
	 */
	void drawMembers(QPainter & p, Uml::Object_Type ot, Uml::Signature_Type sigType,
			 int x, int y, int fontHeight);

	bool m_bShowOperations;            ///< Loaded/saved item.
	bool m_bShowPublicOnly;            ///< Loaded/saved item.
	bool m_bShowScope;                 ///< Loaded/saved item.
	bool m_bShowPackage;               ///< Loaded/saved item.
	Uml::Signature_Type m_ShowOpSigs;  ///< Loaded/saved item.

	/**
	 * Text width margin
	 */
	static const int MARGIN;

	// Auxiliary variables for size calculations and drawing
	int m_bodyOffsetY, m_w, m_h;
};

#endif
