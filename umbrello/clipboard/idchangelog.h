/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IDCHANGELOG_H
#define IDCHANGELOG_H


/**
 *	This class contains all the ID translations done for each
 *	UMLObject pasted. It contains for each old id its new
 *	assigned id.
 *
 *	@author Gustavo Madrigal
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

#include <qpointarray.h>

class IDChangeLog {
public:
	/**
	 * 	Constructor
	 */
	IDChangeLog();

	/**
	 *	Copy constructor
	 */
	IDChangeLog(const IDChangeLog& Other);

	/**
	 * 		Deconstructor
	 */
	~IDChangeLog();

	/**
	 *  	Overloaded '=' operator
	 */
	IDChangeLog& operator=(const IDChangeLog& Other);

	/**
	 *  	Overloaded '==' operator
	 */
	bool operator==(const IDChangeLog& Other);

	/**
	 * Adds a new ID Change to the log
	 */
	void addIDChange(int OldID, int NewID);

	/**
	 *	Appends another IDChangeLog to this instance of IDChangeLog and
	 *	returns a reference to itself
	 */
	IDChangeLog& operator+=(const IDChangeLog& Other);

	/**
	 * Returns the new assigned ID of the object that had OldID as its previous id
	 */
	int findNewID(int OldID);

	/**
	 *	Returns the old ID of an UMLobject given its new one
	 */
	int findOldID(int NewID);

	/**
	 *	 Removes a change giving an New ID
	 */
	void removeChangeByNewID( int OldID);

	enum SpecialIDs
	{
	    NullID = -1000 //A not posible id
	};

public:
	/**
	 * Each change is a QPoint (x=newID, y=oldID)
	 */
	QPointArray m_LogArray;

private:
	/**
	 * Finds a specific change in the log
	 */
	bool findIDChange(int OldID, int NewID, uint& pos);
};

#endif
