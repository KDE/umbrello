 /*
  *  copyright (C) 2003-2004
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

#ifndef CLASSWIDGET_H
#define CLASSWIDGET_H

#include "classifierwidget.h"
#include <qsize.h>

class UMLView;
class UMLClass;
class AssociationWidget;

/**
 * Defines a graphical version of the Class.  Most of the functionality
 * comes from its ancestors, @ref ClassifierWidget and @ref UMLWidget.
 *
 * @short A graphical version of a Class.
 * @author Paul Hensgen	<phensgen@techie.com>
 * @see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassWidget : public ClassifierWidget {
public:

	/**
	 * Constructs a ClassWidget.
	 *
	 * @param view	The parent of this ClassWidget.
	 * @param o	The UMLObject to represent.
	 */
	ClassWidget(UMLView * view, UMLClass * o);

	/**
	 * destructor
	 */
	virtual ~ClassWidget();

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
	 * Set the status of whether to show StereoType
	 *
	 * @param _show		True if stereotype shall be shown.
	 */
	void setShowStereotype(bool _status);

	/**
	 * Toggles the status of whether to show StereoType.
	 */
	void toggleShowStereotype();

	/**
	 * Return the number of displayed attributes.
	 */
	int displayedAttributes();

	/**
	 * Set the AssociationWidget when this ClassWidget acts as
	 * an association class.
	 */
	void setClassAssocWidget(AssociationWidget *assocwidget);

	/**
	 * Return the AssociationWidget when this ClassWidget acts as
	 * an association class (else return NULL.)
	 */
	AssociationWidget *getClassAssocWidget();

	/**
	 * Overrides standard method.
	 */
	void draw(QPainter & p, int offsetX, int offsetY);

	/**
	 * Creates the <classwidget> XML element.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <classwidget> XML element.
	 */
	bool loadFromXMI( QDomElement & qElement );

public slots:
	/**
	 * Called when a menu selection has been made from the popup menu.
	 *
	 * @param sel		The selection id that has been selected.
	 */
	void slotMenuSelection(int sel);	

private:
	/**
	 * Initializes key variables of the class.
	 */
	void init();

	/**
	 * Automatically calculates the size of the object.
	 */
	void calculateSize();

	/**
	 * Updates m_ShowOpSigs and m_ShowAttSigs to match m_bShowScope.
	 */
	void updateSigs();

	bool m_bShowAttributes;             ///< Loaded/saved item.
	bool m_bShowStereotype;             ///< Loaded/saved item.
	Uml::Signature_Type m_ShowAttSigs;  ///< Loaded/saved item.

	AssociationWidget *m_pAssocWidget;
};

#endif
