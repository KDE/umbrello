/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSIFIERLISTITEM_H
#define CLASSIFIERLISTITEM_H

#include <qptrlist.h>
#include "umlobject.h"

/**
 * Classifiers (classes, interfaces) have lists of operations,
 * attributes, templates and others.  This is a base class for
 * the items in this list.  This abstraction should remove
 * duplication of dialogues and allow for stereotypes in lists.
 *
 * @short A base class for classifier list items (e.g. attributes)
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLClassifierListItem : public UMLObject {
	Q_OBJECT
public:
	/**
	 * Constructor.  Empty.
	 *
	 * @param parent	The parent to this operation.
	 * @param name		The name of the operation.
	 * @param id		The id of the operation.
	 */
	UMLClassifierListItem(UMLObject *parent, QString Name, int id);

	/**
	 * Constructor.  Empty.
	 *
	 * @param parent	The parent to this operation.
	 */
	UMLClassifierListItem(UMLObject *parent);

	/**
	 * Destructor.  Empty.
	 */
	virtual ~UMLClassifierListItem();

	/**
	 * Returns a string representation of the operation.
	 *
	 * @param sig		What type of operation string to show.
	 * @return	The string representation of the operation.
	 */
	virtual QString toString(Signature_Type sig = st_NoSig) = 0;

	/**
	 * Display the properties configuration dialogue for the list item.
	 *
	 * @param parent	The parent widget.
	 * @return	True for success of this operation.
	 */
	virtual bool showPropertiesDialogue(QWidget* parent) = 0;

	/**
	 * Return a short name, usually just calls getName().
	 *
	 * @return	Short name of this UMLClassifierListItem.
	 */
	virtual QString getShortName();

};

#endif
