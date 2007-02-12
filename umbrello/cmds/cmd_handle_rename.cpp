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

#include "cmd_handle_rename.h"

#include "uml.h"

// app includes
#include "umlwidgetcontroller.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlnamespace.h"
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

	cmdHandleRename::cmdHandleRename(FloatingTextWidget* _ftw, QString txt):ftw(_ftw),newstring(txt)
	{	
		oldstring = _ftw->getText();
	}
	
	cmdHandleRename::~cmdHandleRename()
	{

	}
	
	void cmdHandleRename::redo()
	{
		//ftw->changeName(newstring);
	}
	
	void cmdHandleRename::undo()
	{	
		//ftw->changeName(oldstring);
	}

}
