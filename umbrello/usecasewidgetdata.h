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
 * object is serialized With this class we are trying to achieve
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

	/** Returns the amount of bytes needed to serialize this object
	 * If the serialization method of this class is changed this function will have to be CHANGED TOO
	 * This function is used by the Copy and Paste Functionality
	 * The Size in bytes of a serialized QString Object is long sz:
	 * if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
	 * {
	 *	sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
	 * }
	 * This calculation is valid only for QT 2.1.x or superior,
	 * this is totally incompatible with QT 2.0.x or QT 1.x or
	 * inferior That means the copy and paste functionality will
	 * work on with QT 2.1.x or superior
	 */
	virtual long getClipSizeOf();

	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );
};

#endif
