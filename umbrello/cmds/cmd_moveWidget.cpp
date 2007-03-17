/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "cmd_moveWidget.h"

#include "uml.h"

// app includes
#include "umlwidgetcontroller.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlnamespace.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "listpopupmenu.h"
#include "classifierwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"

#include <klocale.h>

namespace Uml
{

	cmdMoveWidget::cmdMoveWidget(UMLWidgetController* _UMLwc):QUndoCommand(),UMLwc(_UMLwc),already(false)
	{	
		UMLWidget * w = _UMLwc->getWidget();
		setText(i18n("Move widget :") + w->getName());
		X = w->getX();
		Y = w->getY();
		oldX = _UMLwc->getOldX();
		oldY = _UMLwc->getOldY();
	}
	
	cmdMoveWidget::~cmdMoveWidget()
	{

	}
	
	void cmdMoveWidget::redo()
	{

		UMLwc->insertSaveValues(oldX,oldY,X,Y);
		  UMLwc->widgetMoved();

		//UMLWidget * w = 
		//UMLwc->reverseOldNewValues();
		//UMLwc->widgetMoved();
		//UMLwc->moveWidget(diffX,diffY);
		//UMLwc->moveWidget(10,10);
		//widget->getWidgetController()->moveWidgetBy(diffX, diffY);
	}
	
	void cmdMoveWidget::undo()
	{	
		int tempX;
		int tempY;

		//UMLWidget * w = 
		UMLwc->insertSaveValues(X,Y,oldX,oldY);
		UMLwc->widgetMoved();
    		//w->setX(oldX);
    		//w->setY(oldY);
		
		//UMLwc->moveWidget(w->getX() - oldX,w->getY() - oldY);

	//UMLwc->moveWidget(-10,-10);

		//UMLwc->moveWidget(-diffX,-diffY);
		//widget->getWidgetController()->moveWidgetBy(-diffX, -diffY);
	}

	bool cmdMoveWidget::mergeWith(const QUndoCommand* other) {
		const cmdMoveWidget* otherCmd = static_cast<const cmdMoveWidget*>(other);
		if (UMLwc != otherCmd->UMLwc)
			return false;
		X = otherCmd->X;
		Y = otherCmd->Y;
		return true;
	}
}
