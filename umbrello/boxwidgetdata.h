/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BOXWIDGETDATA_H
#define BOXWIDGETDATA_H

#include <qstring.h>
#include "umlwidgetdata.h"
#include "dialogs/settingsdlg.h"

class QDataStream;

/**
 *	This class holds all the BoxWidget's Information
 *	All this information goes to a file or clipboard
 *	when an BoxWidget object is saved.
 *
 *	@author Jonathan Riddell
 */

class BoxWidgetData : public UMLWidgetData {
public:
	/**
	 *  Standard constructor sets the base type
	 */
	BoxWidgetData();

	/**
	 * Copy constructor
	 */
	BoxWidgetData(BoxWidgetData & Other);

	/**
	 *  Empty destructor
	 */
	~BoxWidgetData();

	/**
	 *	Saves the box to XMI using <boxwidget>
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 *	Loads the <boxwidget> element
	 */
	bool loadFromXMI(QDomElement& qElement );

};

#endif
