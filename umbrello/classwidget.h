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

#include "umlwidget.h"
#include <qsize.h>

class UMLView;

#define MARGIN 5

/**
 * Defines a graphical version of the Class.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits.
 *
 * @short A graphical version of a Class.
 * @author Paul Hensgen	<phensgen@techie.com>
 * @see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassWidget : public UMLWidget {
public:

	/**
	 * Constructs a ClassWidget.
	 *
	 * @param view	The parent of this ClassWidget.
	 * @param o	The UMLObject to represent.
	 */
	ClassWidget(UMLView * view, UMLObject * o);

	/**
	 * Constructs a ClassWidget.
	 *
	 * @param view	The parent of this ClassWidget.
	 */
	ClassWidget(UMLView * view);

	/**
	 * Standard deconstructor.
	 */
	~ClassWidget();

	/**
	 * Initializes key variables of the class.
	 */
	void init();

	/**
	 * Returns the status of showing attributes.
	 *
	 * @return	True if attributes are shown.
	 */
	bool getShowAtts() const {
		return m_bShowAttributes;
	}

	/**
	 * Toggles the status of showing attributes.
	 */
	void toggleShowAtts();

	/**
	 * Return the status of showing operations.
	 *
	 * @return	True if operations are shown.
	 */
	bool getShowOps() const {
		return m_bShowOperations;
	}

	/**
	 * Toggles the status of showing operations.
	 */
	void toggleShowOps();

	/**
	 * Return the status of showing operation signatures.
	 *
	 * @return	Status of how operation signatures are shown.
	 */
	Uml::Signature_Type getShowOpSigs() const {
		return m_ShowOpSigs;
	}

	/**
	 * Toggles the status of showing operation signatures.
	 */
	void toggleShowOpSigs();

	/**
	 * Returns the status of showing attribute signatures.
	 *
	 * @return	Status of how attribute signatures are shown.
	 */
	Uml::Signature_Type getShowAttSigs() {
		return m_ShowAttSigs;
	}

	/**
	 * toggles the status of showing attribute signatures
	 */
	void toggleShowAttSigs();

	/**
	 * Returns the status of whether to show scope.
	 *
	 * @return	True if scope is shown.
	 */
	bool getShowScope() const {
		return m_bShowScope;
	}

	/**
	 * Toggles the status of whether to show scope.
	 */
	void toggleShowScope();

	/**
	 * Set the status of whether to show scope
	 *
	 * @param _scope	True if scope shall be shown.
	 */
	void setShowScope(bool _scope);

	/**
	 * Set the status of whether to show Attributes
	 *
	 * @param _show		True if attributes shall be shown.
	 */
	void setShowAtts(bool _show);

	/**
	 * Set the status of whether to show Attribute signature
	 *
	 * @param _show		True if attribute signatures shall be shown.
	 */
	void setShowAttSigs(bool _show);

	/**
	 * Set the status of whether to show Operation signature
	 *
	 * @param _show		True if operation signatures shall be shown.
	 */
	void setShowOpSigs(bool _show);

	/**
	 * Set the status of whether to show Operations
	 *
	 * @param _show		True if operations shall be shown.
	 */
	void setShowOps(bool _show);

	/**
	 * Set the type of signature to display for an Operation
	 *
	 * @param sig	Type of signature to display for an operation.
	 */
	void setOpSignature(Uml::Signature_Type sig);

	/**
	 * Set the type of signature to display for an Attribute
	 *
	 * @param sig	Type of signature to display for an attribute.
	 */
	void setAttSignature(Uml::Signature_Type sig);

	/**
	 * Returns the status of whether to show StereoType.
	 *
	 * @return	True if stereotype is shown.
	 */
	bool getShowStereotype() const {
		return m_bShowStereotype;
	}

	/**
	 * Returns the status of whether to show Package.
	 *
	 * @return  True if package is shown.
	 */
	bool getShowPackage() const {
		return m_bShowPackage;
	}

	/**
	 * Toggles the status of whether to show package.
	 */
	void toggleShowPackage();

	/**
	 * Set the status of whether to show StereoType
	 *
	 * @param _show		True if stereotype shall be shown.
	 */
	void setShowStereotype(bool _status);

	/*
	 * Toggles the status of whether to show StereoType.
	 */
	void toggleShowStereotype();

	/**
	 * Set the status of whether to show Package
	 *
	 * @param _show  True if package shall be shown.
	 */
	void setShowPackage(bool _status);

	/**
	 * Activate the object after serializing it from a QDataStream.
	 *
	 * @param ChangeLog	Pointer to the IDChangeLog (optional.)
	 */
	virtual bool activate(IDChangeLog* ChangeLog  = 0 );

	/**
	 * Overrides standard method.
	 */
	void draw(QPainter & p, int offsetX, int offsetY);

	/**
	 * Creates the <UML:ClassWidget> XML element.
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <UML:ClassWidget> XML element.
	 */
	bool loadFromXMI( QDomElement & qElement );

private:

	/**
	 * Automatically calculates the size of the object.
	 */
	void calculateSize();

	/**
	 * Calculcates the size of the templates box in the top left
	 * if it exists, returns QSize(0,0) if it doesn't.
	 *
	 * @return	QSize of the templates flap.
	 */
	QSize calculateTemplatesBoxSize();

	/**
	 * Updates m_ShowOpSigs and m_ShowAttSigs to match m_bShowScope.
	 */
	void updateSigs();

	/**
	 * The right mouse button menu.
	 */
	ListPopupMenu* m_pMenu;

	// Data members loaded/saved

	bool m_bShowAttributes;
	bool m_bShowOperations;
	bool m_bShowScope;
	bool m_bShowPackage;
	bool m_bShowStereotype;
	Uml::Signature_Type m_ShowOpSigs;
	Uml::Signature_Type m_ShowAttSigs;

public slots:
	/**
	 * Called when a menu selection has been made from the popup menu.
	 *
	 * @param sel		The selection id that has been selected.
	 */
	void slotMenuSelection(int sel);
};

#endif
