/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_create_umlobject.h"

#include "uml.h"
#include "umldoc.h"

#include <klocale.h>

namespace Uml
{

	cmdCreateUMLObject::cmdCreateUMLObject(UMLObject* o):m_obj(o)
	{
		setText(i18n("Create uml object :") + o->getFullyQualifiedName());
	}
	
	cmdCreateUMLObject::~cmdCreateUMLObject()
	{
	}
	
	// Create the UMLObject
	void cmdCreateUMLObject::redo()
	{
		UMLDoc *doc = UMLApp::app()->getDocument();
		doc->signalUMLObjectCreated(m_obj);
	}
	
	// Suppress the UMLObject
	void cmdCreateUMLObject::undo()
	{
		UMLDoc *doc = UMLApp::app()->getDocument();
		doc->removeUMLObject(m_obj);
	}

}
