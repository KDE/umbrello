/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USECASEWIDGETDATA_H
#define USECASEWIDGETDATA_H

#include "umlwidgetdata.h"
#include "dialogs/settingsdlg.h"

/**This class holds all the UseCaseWidget's Information All this
 * information goes to a file or clipboard when an UseCaseWidget
 * object is saved With this class we are trying to achieve
 * isolation between data and display layers.
 * @author Gustavo Madrigal
 */
class UseCaseWidgetData : public UMLWidgetData {
public:
	UseCaseWidgetData(SettingsDlg::OptionState optionState);

	UseCaseWidgetData(UseCaseWidgetData & Other);

	virtual ~UseCaseWidgetData();

	virtual UseCaseWidgetData & operator=(UseCaseWidgetData & Other);

	virtual bool operator==(UseCaseWidgetData & Other);

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );
};

#endif
