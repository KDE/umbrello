/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLLISTVIEWITEMDATA_H
#define UMLLISTVIEWITEMDATA_H
#include "umlnamespace.h"
#include <qdom.h>

/**
 *	Contains all the uml data of
 *	a UMLListViewItem object
 *	@author Gustavo Madrigal
 */

class UMLListViewItem;
class QDataStream;
class UMLObject;

class UMLListViewItemData {
public:
	/**
	 *		Constructor
	 */
	UMLListViewItemData();

	/**
	 * 		Constructor
	 */
	UMLListViewItemData(UMLListViewItemData& Other);

	/**
	 * 		Deconstructor
	 */
	virtual ~UMLListViewItemData();

	/**
	 * sets the UMLListViewItem that contains the object
	 */
	void setListViewItem(UMLListViewItem* Item);

	/**
	 * Returns the number of children of the UMLListViewItem
	 *	containing this object
	 */
	int childCount();

	/**
	 * returns a pointer to the UMLListViewItem containing this object
	 */
	UMLListViewItem* getListViewItem();

	/**
	 *	 serializes this object to a QDataStream
	 */
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	 *	Serialize an instance of this class to a QDataStream that will be put in the clipboard
	 */
	virtual bool clipSerialize(QDataStream *s, const bool toClip);

	/**
	 *	 Returns the amount of bytes needed to serialize an instance object to the clipboard
	 */
	virtual long getClipSizeOf();

	/**
	 * Returns the text of the UMLListViewItem containing this object
	 */
	QString text(int index);

	/**
	 * Overload '==' operator
	 */
	bool operator==(const UMLListViewItemData& Other);

	/**
	 *	Overload '=' operator
	 */
	UMLListViewItemData& operator=(UMLListViewItemData & Other);

	/**
	 * 		Returns the UMLObject associated with this item.
	 */
	UMLObject * getUMLObject() {
		return object;
	}

	/**
	 *		Returns the type of item.
	 */
	Uml::ListView_Type getType() const {
		return m_Type;
	}

	/**
	 * 	Returns the id of the item being represented.
	 */
	int getID() {
		return m_nId;
	}

	/**
	 * 		Returns the label text.
	 */
	QString getLabel() {
		return label;
	}

	/**
	 * 	Returns the amount of childer the item has.
	 */
	int getChildren() {
		return m_nChildren;
	}

	/**
	 * 		Sets the UMLObject associated with this item.
	 */
	void setUMLObject( UMLObject * _object ) {
		object = _object;
	}

	/**
	 * Sets the type of the item being represented.
	 */
	void setType( Uml::ListView_Type type ) {
		m_Type = type;
	}

	/**
	 * 	Sets the id of the item being represented.
	 */
	void setID( int id ) {
		m_nId = id;
	}

	/**
	 * 		Sets the label text.
	 */
	void setLabel( QString _label ) {
		label = _label;
	}

	/**
	 * saves the listview item to a <listitem> tag
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement );

	/**
	 * Loads a <listitem> tag, this is only used by the clipboard currently
	 */
	bool loadFromXMI(QDomElement& qElement);
protected: // Private methods

	UMLListViewItem * m_pItem;
	int m_nId, m_nChildren;
	Uml::ListView_Type m_Type;
	UMLObject * object;
	QString label;
};

#endif
