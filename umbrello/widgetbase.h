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

#ifndef WIDGETBASE_H
#define WIDGETBASE_H

#include <qobject.h>

#include "umlnamespace.h"

// forward declarations
class UMLView;
class UMLObject;

/**
 * @short	Common base class for UMLWidget and AssociationWidget
 * @author 	Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class WidgetBase : public QObject {
	Q_OBJECT
public:
	/**
	 * Creates a WidgetBase object.
	 *
	 * @param view	The view to be displayed on.
	 */
	WidgetBase(UMLView * view);

	/**
	 * Standard deconstructor
	 */
	virtual ~WidgetBase() {}

	/**
	 * Write property of m_Type.
	 */
	void setBaseType(Uml::Widget_Type type);

	/**
	 * Read property of m_Type.
	 */
	Uml::Widget_Type getBaseType() const;

	/**
	 * Returns the @ref UMLObject set to represent.
	 *
	 * @return the UMLObject to represent.
	 */
	UMLObject *getUMLObject();

	/**
	 * Deliver a const pointer to the connected UMLView
	 * ( needed esp. by event handling of LinePath )
	 */
	const UMLView *getUMLView() const { return m_pView; };

	/**
	 * Sets the @ref UMLObject to represent.
	 *
	 * @param o	The object to represent.
	 */
	virtual void setUMLObject(UMLObject * o);

	/**
	 * Used by some child classes to get documentation.
	 *
	 * @return	The documentation from the UMLObject (if m_pObject is set.)
	 */
	virtual QString getDoc() const;

	/**
	 * Used by some child classes to set documentation.
	 *
	 * @param doc	The documentation to be set in the UMLObject
	 *		(if m_pObject is set.)
	 */
	virtual void setDoc( const QString &doc );

protected:
	/**
	 * Type of widget.
	 */
	Uml::Widget_Type m_Type;

	UMLView   *m_pView;
	UMLObject *m_pObject;
};

#endif
