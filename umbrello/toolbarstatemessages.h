/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TOOLBARSTATEMESSAGES_H
#define TOOLBARSTATEMESSAGES_H

#include "toolbarstatepool.h"

#include "umlnamespace.h"
#include "worktoolbar.h"

class QMouseEvent;
class UMLView;
class UMLWidget;
class QCanvasLine;

class ToolBarStateMessages : public ToolBarStatePool
{

public:
	ToolBarStateMessages(UMLView *umlView);	
	virtual ~ToolBarStateMessages();	

	virtual void mousePress(QMouseEvent* ome);
	virtual void mouseRelease(QMouseEvent* ome);
	virtual void mouseDoubleClick(QMouseEvent* ome);
	virtual void mouseMove(QMouseEvent* ome);
	
	virtual void init();

protected:
	Uml::Sequence_Message_Type getMessageType ();
	virtual bool setSelectedWidget(QMouseEvent * me);
	void removeLine();

protected:
	UMLWidget* m_pSelectedWidget;
	QCanvasLine* m_pLine;
};

#endif //TOOLBARSTATEMESSAGES_H
