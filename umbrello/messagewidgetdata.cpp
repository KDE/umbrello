/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatastream.h>
#include <kdebug.h>

#include "messagewidgetdata.h"

MessageWidgetData::MessageWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_nWidgetAID = -1;
	m_nWidgetBID = -1;
	m_SequenceNumber = "";
	m_Operation = "";
	m_nTextID = -1;
	m_Type = Uml::wt_Message;
}

MessageWidgetData::MessageWidgetData(MessageWidgetData & Other) : UMLWidgetData(Other) {
	*this = Other;
}

MessageWidgetData::~MessageWidgetData() {
}

MessageWidgetData & MessageWidgetData::operator=(MessageWidgetData & Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;
	m_nWidgetAID = Other.m_nWidgetAID;
	m_nWidgetBID = Other.m_nWidgetBID;
	m_SequenceNumber = Other.m_SequenceNumber;
	m_Operation = Other.m_Operation;
	m_nTextID = Other.m_nTextID;

	return *this;
}

bool MessageWidgetData::operator==(MessageWidgetData & Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}
	if(m_nWidgetAID != Other.m_nWidgetAID) {
		return false;
	}
	if(m_nWidgetBID != Other.m_nWidgetBID) {
		return false;
	}
	if(m_SequenceNumber != Other.m_SequenceNumber) {
		return false;
	}
	if(m_Operation != Other.m_Operation) {
		return false;
	}
	if(m_nTextID != Other.m_nTextID) {
		return false;
	}

	return true;
}

/** Read property of int m_nWidgetAID. */
int MessageWidgetData::getWidgetAID() {
	return m_nWidgetAID;
}

/** Write property of int m_nWidgetAID. */
void MessageWidgetData::setWidgetAID( int WidgetAID) {
	m_nWidgetAID = WidgetAID;
}

/** Read property of int m_nWidgetBID. */
int MessageWidgetData::getWidgetBID() {
	return m_nWidgetBID;
}

/** Write property of int m_nWidgetBID. */
void MessageWidgetData::setWidgetBID( int WidgetBID) {
	m_nWidgetBID = WidgetBID;
}

/** Read property of QString m_SequenceNumber. */

QString MessageWidgetData::getSequenceNumber() {
	return m_SequenceNumber;
}

/** Write property of QString m_SequenceNumber. */
void MessageWidgetData::setSequenceNumber( QString SequenceNumber) {
	m_SequenceNumber = SequenceNumber;
}

/** Read property of QString m_Operation. */
QString MessageWidgetData::getOperation() {
	return m_Operation;
}

/** Write property of QString m_Operation. */
void MessageWidgetData::setOperation( QString Operation) {
	m_Operation = Operation;

}

/** Read property of int m_nTextID. */
int MessageWidgetData::getTextID() {
	return m_nTextID;
}

/** Write property of int m_nTextID. */
void MessageWidgetData::setTextID( int TextID) {
	m_nTextID = TextID;
}

/** No descriptions */
void MessageWidgetData::print2cerr() {
	UMLWidgetData::print2cerr();
	kdDebug() << "m_Operation = " << m_Operation << endl;
	kdDebug() << "m_SequenceNumber = " << m_SequenceNumber << endl;
	kdDebug() << "m_nTextID = " << m_nTextID << endl;
	kdDebug() << "m_WidgetAID = " << m_nWidgetAID << endl;
	kdDebug() << "m_WidgetBID = " << m_nWidgetBID << endl;
}

bool MessageWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement messageElement = qDoc.createElement( "UML:MessageWidget" );
	bool status = UMLWidgetData::saveToXMI( qDoc, messageElement );
	messageElement.setAttribute( "textid", m_nTextID );
	messageElement.setAttribute( "widgetaid", m_nWidgetAID );
	messageElement.setAttribute( "widgetbid", m_nWidgetBID );
	messageElement.setAttribute( "operation", m_Operation );
	messageElement.setAttribute( "seqnum", m_SequenceNumber );
	qElement.appendChild( messageElement );
	return status;
}

bool MessageWidgetData::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidgetData::loadFromXMI( qElement ) )
		return false;
	QString textid = qElement.attribute( "textid", "-1" );
	QString widgetaid = qElement.attribute( "widgetaid", "-1" );
	QString widgetbid = qElement.attribute( "widgetbid", "-1" );
	m_Operation = qElement.attribute( "operation", "" );
	m_SequenceNumber = qElement.attribute( "seqnum", "" );

	m_nTextID = textid.toInt();
	m_nWidgetAID = widgetaid.toInt();
	m_nWidgetBID = widgetbid.toInt();
	return true;
}
