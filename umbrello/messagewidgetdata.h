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
 * object is saved.  With this class we are trying to achieve
 * isolation between data and display layers.
 * @author Gustavo Madrigal
 */
class MessageWidgetData : public UMLWidgetData {
public:
	/**
	 * Constructor.  Sets message type to sequenceMessageType.
	 */
	MessageWidgetData(SettingsDlg::OptionState optionState, Sequence_Message_Type sequenceMessageType);

	/**
	 * Copy constructor
	 */
	MessageWidgetData(MessageWidgetData& Other);

	/**
	 * Destructor, empty
	 */
	virtual ~MessageWidgetData();

	/**
	 * Assignment
	 */
	virtual MessageWidgetData& operator=(MessageWidgetData& Other);

	/**
	 * Comparason
	 */
	virtual bool operator==(MessageWidgetData & Other);

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

	/**
	 * Returns whether the message is synchronous or asynchronous
	 */
	Sequence_Message_Type getSequenceMessageType();

	/**
	 * Debugging method
	 */
	virtual void print2cerr();

	/**
	 * saves properties for the supplied <UML:MessageWidget> tag
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * loads properties from a supplied <UML:MessageWidget> tag
	 */
	bool loadFromXMI( QDomElement & qElement );
protected:
	int m_nWidgetAID;
	int m_nWidgetBID;

	QString m_SequenceNumber;
	QString m_Operation;

	int m_nTextID;

	/**
	 *	Whether the message is synchronous or asynchronous
	 */
        Sequence_Message_Type m_sequenceMessageType;
};

#endif
