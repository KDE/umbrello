/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TOOLBARSTATEOTHER_H
#define TOOLBARSTATEOTHER_H

#include "toolbarstatepool.h"


class QMouseEvent;
class UMLView;
class UMLWidget;

/** 
 * Almost every object is created by this state. Except assiociations and
 * messages. 
 */ 
class ToolBarStateOther : public ToolBarStatePool
{

public:
	ToolBarStateOther(UMLView *umlView);	
	virtual ~ToolBarStateOther();	

	virtual void mousePress(QMouseEvent* ome);
	virtual void mouseRelease(QMouseEvent* ome);
	virtual void mouseDoubleClick(QMouseEvent* ome);
	virtual void mouseMove(QMouseEvent* ome);
	
private:

	Uml::UMLObject_Type getObjectType(WorkToolBar::ToolBar_Buttons tbb);
	bool newWidget();

	void floatingTextSetup(UMLWidget* umlWidget);
	void activityWidgetSetup(UMLWidget* umlWidget);
	void stateWidgetSetup(UMLWidget* umlWidget);
};

#endif //TOOLBARSTATEOTHER_H
