
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
 
 /* Created By Krzywda Stanislas and Bouchikhi Mohamed-Amine ;) */

#include "cmd_changeFillColor.h"
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
	cmdChangeFillColor::cmdChangeFillColor(	/* obj1: m_pDoc   */	UMLDoc *doc
											/* obj2: m_pView  */	, UMLView *view
											/* obj3: newColor */	, QColor col):already(false)
	{
		UMLWidget * widget = view->getFirstMultiSelectedWidget();
		pDoc=doc;
		pView=view;
		//oldColor=pView->getFillColour();
		color = col;
		oldColor= widget -> wgetFillColour() ;
		//widget -> setFillColour(color);
		//pView -> selectionSetFillColor( col );
		pDoc -> setModified(true);
	}

	cmdChangeFillColor::~cmdChangeFillColor()
	{

	}
	void cmdChangeFillColor::redo()
	{
			pView -> selectionSetFillColor( color );
        	pDoc -> setModified(true);
	}
	
	void cmdChangeFillColor::undo()
	{	
		
		pView -> selectionSetFillColor( oldColor );
        pDoc -> setModified(true);
	}
}
