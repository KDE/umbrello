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

#include "notewidgetdata.h"

NoteWidgetData::NoteWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_bLinkDocumentation = false;
	m_Text  = "";
	m_Type = Uml::wt_Note;
}

NoteWidgetData::NoteWidgetData(NoteWidgetData & Other) : UMLWidgetData(Other) {
	*this = Other;
}

NoteWidgetData::~NoteWidgetData() {
}

NoteWidgetData & NoteWidgetData::operator=(NoteWidgetData & Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;
	m_bLinkDocumentation = Other.m_bLinkDocumentation;
	m_Text  = Other.m_Text;
	return *this;
}

bool NoteWidgetData::operator==(NoteWidgetData & Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}
	if(m_bLinkDocumentation != Other.m_bLinkDocumentation) {
		return false;
	}
	if(m_Text  != Other.m_Text) {
		return false;
	}
	return true;
}

QString NoteWidgetData::getText() {
	return m_Text;
}

void NoteWidgetData::setText( QString Text) {
	m_Text = Text;
}

bool NoteWidgetData::getLinkDocumentation() {
	return m_bLinkDocumentation;
}

void NoteWidgetData::setLinkDocumentation( bool LinkDocumentation) {
	m_bLinkDocumentation = LinkDocumentation;
}

bool NoteWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement noteElement = qDoc.createElement( "UML:NoteWidget" );
	bool status = UMLWidgetData::saveToXMI( qDoc, noteElement );
	noteElement.setAttribute( "text", m_Text );
	qElement.appendChild( noteElement );
	return status;
}

bool NoteWidgetData::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidgetData::loadFromXMI( qElement ) )
		return false;
	m_Text = qElement.attribute( "text", "" );
	return true;
}




