/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NOTEWIDGETDATA_H
#define NOTEWIDGETDATA_H

#include <qstring.h>
#include "umlwidgetdata.h"
#include "dialogs/settingsdlg.h"

class QDataStream;

/**	This class holds all the NoteWidget's Information
 *	All this information goes to a file or clipboard
 *	when an NoteWidget object is serialized
 *	With this class we are trying to achieve isolation
 *	between data and display layers.
 *	@author Gustavo Madrigal
 */

class NoteWidgetData : public UMLWidgetData {
	friend class NoteWidget;
public:
	/**
	*
	*/
	NoteWidgetData(SettingsDlg::OptionState optionState);

	/**
	*
	*/
	NoteWidgetData(NoteWidgetData & Other);

	/**
	*
	*/
	~NoteWidgetData();

	/**
	*
	*/
	virtual NoteWidgetData & operator=(NoteWidgetData & Other);

	/**
	*
	*/
	virtual bool operator==(NoteWidgetData & Other);

	/**
	*	Write property of QString m_Text.
	*/
	virtual void setText( QString Text);

	/**
	*	Read property of QString m_Text.
	*/
	virtual QString getText();

	/**
	*	Returns the amount of bytes needed to serialize this object
	*/
	virtual long getClipSizeOf();

	/**
	*	Write property of bool m_bLinkDocumentation.
	*/
	virtual void setLinkDocumentation( bool LinkDocumentation);

	/**
	*	Read property of bool m_bLinkDocumentation.
	*/
	virtual bool getLinkDocumentation();

	/**
	*	No descriptions
	*/
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	*
	*/
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	*
	*/
	bool loadFromXMI( QDomElement & qElement );
protected:
	/**
	*
	*/
	QString m_Text;

	/**
	*
	*/
	bool m_bLinkDocumentation;
};

#endif
