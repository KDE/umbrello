/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MESSAGEWIDGETDATA_H
#define MESSAGEWIDGETDATA_H

#include <qstring.h>
#include "umlwidgetdata.h"
#include "dialogs/settingsdlg.h"

class QDataStream;

/**
 * This class holds all the MessageWidget's Information All this
 * information goes to a file or clipboard when an MessageWidget
 * object is serialized With this class we are trying to achieve
 * isolation between data and display layers.
 * @author Gustavo Madrigal
 */
class MessageWidgetData : public UMLWidgetData {
	friend class MessageWidget;
public:
	MessageWidgetData(SettingsDlg::OptionState optionState);
	MessageWidgetData(MessageWidgetData & Other);
	virtual ~MessageWidgetData();
	virtual MessageWidgetData & operator=(MessageWidgetData & Other);
	virtual bool operator==(MessageWidgetData & Other);
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	 * Returns the amount of bytes needed to serialize this object
	 * If the serialization method of this class is changed this function will have to be CHANGED TOO
	 * This function is used by the Copy and Paste Functionality
	 * The Size in bytes of a serialized QString Object is long sz:
	 *	if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
	 *	{
	 *		sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
	 *	}
	 * This calculation is valid only for QT 2.1.x or superior,
	 * this is totally incompatible with QT 2.0.x or QT 1.x or
	 * inferior
	 * That means the copy and paste functionality will work on with QT 2.1.x or superior
	 */
	virtual long getClipSizeOf();

	/**
	 * Write property of int m_nWidgetAID.
	 */
	virtual void setWidgetAID( int WidgetAID);

	/**
	 * Read property of int m_nWidgetAID.
	 */
	virtual int getWidgetAID();

	/**
	 * Write property of int m_nWidgetBID.
	 */
	virtual void setWidgetBID( int WidgetBI);

	/**
	 * Read property of int m_nWidgetBID.
	 */
	virtual int getWidgetBID();

	/**
	 * Write property of QString m_SequenceNumber.
	 */
	virtual void setSequenceNumber( QString SequenceNumber);

	/**
	 * Read property of QString m_SequenceNumber.
	 */
	virtual QString getSequenceNumber();

	/**
	 * Write property of QString m_Operation.
	 */
	virtual void setOperation( QString Operation);

	/**
	 * Read property of QString m_Operation.
	 */
	virtual QString getOperation();

	/**
	 * Write property of int m_nTextID.
	 */
	virtual void setTextID( int TextID);

	/**
	 * Read property of int m_nTextID.
	 */
	virtual int getTextID();

	virtual void print2cerr();

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );
protected:
	int m_nWidgetAID;
	int m_nWidgetBID;

	QString m_SequenceNumber;
	QString m_Operation;

	int m_nTextID;
};

#endif
