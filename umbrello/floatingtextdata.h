/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FLOATINGTEXTDATA_H
#define FLOATINGTEXTDATA_H

#include "umlwidgetdata.h"
#include "umlnamespace.h"
#include <qstring.h>

/**
 *  This class holds all the FloatingText's Information
 *	All this information goes to a file or clipboard
 *	when an FloatingText object is serialized
 *	With this class we are trying to achieve isolation
 *	between data and display layers.
 *
 *	@author Gustavo Madrigal
 */
class FloatingTextData : public UMLWidgetData {
	friend class FloatingText;
public:
	/**
	* 		Constructor
	*/
	FloatingTextData();

	/**
	* 		Constructor
	*/
	FloatingTextData(FloatingTextData & Other);

	/**
	* 		Deconstructor
	*/
	virtual ~FloatingTextData();

	/**
	* 		Overloaded '=' operator
	*/
	virtual FloatingTextData & operator=(FloatingTextData & Other);

	/**
	* 		OverLoaded '==' operator
	*/
	virtual bool operator==(FloatingTextData & Other);

	/**
	* Write property of QString m_Text.
	*/
	virtual void setText( QString _newVal);


	/**
	 * Write property of QString m_PreText
	 */
	virtual void setPreText( QString _newVal);

	/**
	 * Write property of QString m_PostText
	 */
	virtual void setPostText( QString _newVal);

	/**
	 * Read property of QString m_Text.
	 */
	virtual QString getText();

	/**
	* Read property of QString m_PreText.
	*/
	virtual QString getPreText();

	/**
	 * Read property of QString m_PostText.
	 */
	virtual QString getPostText();

	/**
	 * Get the text that should be displayed
	 */
	virtual QString getDisplayText();

	/**
	*	Save load the classes information
	*/
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	 *  Returns the amount of bytes needed to serialize this object
	 *	If the serialization method of this class is changed this function will have to be CHANGED TOO
	 *	This function is used by the Copy and Paste Functionality
	 *	The Size in bytes of a serialized QString Object is long sz:
	 *		if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
	 *		{
	 *		sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
	 *		}
	 *	This calculation is valid only for QT 2.1.x or
	 *	superior, this is totally incompatible with QT 2.0.x
	 *	or QT 1.x or inferior
	 *	That means the copy and paste functionality will work on with QT 2.1.x or superior
	 */
	virtual long getClipSizeOf();

	/**
	* 	Returns the sequence number of the operation/message.
	*/
	QString getSeqNum() {
		return m_SeqNum;
	}

	/**
	*  	Sets the sequence number of the operation/message.
	*	Will set the @ref m_Text variable with the correct
	*	seq num + operation combination.
	*/
	void setSeqNum( QString number );

	/**
	*   Returns the operation string.
	*/
	QString getOperation() {
		return m_Operation;
	}

	/**
	*  	Sets the operation string.
	*	Will set the @ref m_Text variable with the correct
	*	seq num + operation combination.
	*/
	void setOperation( QString operation );

	/**
	* Sets the role type of this FloatingText
	*/
	void setRole(Uml::Text_Role Role);

	/**
	*  Return the role of the text widget
	*/
	Uml::Text_Role getRole();

	/**
	 *	Debugging method.
	 */
	virtual void print2cerr();

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );

protected:

	/** 
	 * initialize some string attributes 
	 */
	void init ();

	/**
	 *  These Strings describe the text that the widget will display.
	 */
	QString m_PreText; // prepended text (such as for scope of association Role or method)
	QString m_Text; // main body 
	QString m_PostText; // ending text (such as bracket on changability notation for association Role )

	/**
	 * 		The sequence number of an operation or message.
	 *		Used when representing a @ref MessageWidget or an operation/message
	 *		on a collaboration diagram.
	 *
	 *		We keep this value so we can let the user set it seperatly from the other text.
	 *		When set the @ref setSeqNumber() method will add it to the main text.
	 */
	QString m_SeqNum;

	/**
	 * 		Like the sequence number above, but is used to represent the
	 *		message/operation being displayed.
	 */
	QString m_Operation;

	/**
	 * 		The role the text widget will under take.
	 */
	Uml::Text_Role m_Role;
};

#endif
