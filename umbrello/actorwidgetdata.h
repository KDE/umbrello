/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ACTORWIDGETDATA_H
#define ACTORWIDGETDATA_H

#include "umlwidgetdata.h"

/**
 * This class holds all the ActorWidget's Information All this
 * information goes to a file or clipboard when an ActorWidget object
 * is saved. With this class we are trying to achieve isolation
 * between data and display layers.
 * @author Gustavo Madrigal
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ActorWidgetData : public UMLWidgetData {
public:
	ActorWidgetData(SettingsDlg::OptionState optionState);
	ActorWidgetData(ActorWidgetData & Other);
	virtual ~ActorWidgetData();
	virtual ActorWidgetData & operator=(ActorWidgetData & Other);
	virtual bool operator==(ActorWidgetData & Other);

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );
};

#endif
