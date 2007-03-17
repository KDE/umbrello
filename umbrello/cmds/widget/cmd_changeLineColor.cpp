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



#include "cmd_changeLineColor.h"

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

	/*cmdChangeLineColor::cmdChangeLineColor(UMLView *view, QColor col)
	{
		setText(i18n("Change Line Color"));
		UMLWidget * widget = view->getFirstMultiSelectedWidget();
		pView=view;
		color = col;
		oldColor=widget -> getLineColor() ;
	}*/

cmdChangeLineColor::cmdChangeLineColor(UMLWidget *w, QColor col):UMLw(w),color(col)
{	
	setText(i18n("Change Line Color") + w->getName());
	oldColor= w -> getLineColor() ;

}
	cmdChangeLineColor::~cmdChangeLineColor()
	{
	}
	void cmdChangeLineColor::redo()
	{

		UMLw -> setLineColorcmd( color );
	}
	
	void cmdChangeLineColor::undo()
	{	
		UMLw -> setLineColorcmd( oldColor );
	}
}
