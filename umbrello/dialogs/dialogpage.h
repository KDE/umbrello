/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIALOG_PAGE
#define DIALOG_PAGE


/** 
 * @short A basic interface for all Dialog Pages in Umbrello. 
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class DialogPage
{
public:
	DialogPage( bool autoApply) 
		: m_autoApply(autoApply) {}
	
	/** 
	 * If auto apply is true, the observed object will be modified
	 * inmediatly when the user changes something in the dialog 
	 */	
	inline void setAutoApply( bool a ) {m_autoApply = a;}
	

// the following signals and slots are expected to be implemented:
//
//public slots: 
// 	void cancel();
// 	void apply();
// signals:
// 	void pageModified( );

protected:
	bool m_autoApply;

};



#endif

