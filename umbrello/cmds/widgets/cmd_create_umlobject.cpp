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

#include "cmd_create_umlobject.h"

namespace Uml
{
	cmdCreateUMLObject::cmdCreateUMLObject(UMLView *view, UMLObject *o, const QString& name):m_pUMLView(view), m_pUMLObject(o), m_Name(name)
	{	
		setText(i18n("Create uml object"));
	}
	
	cmdCreateUMLObject::~cmdCreateUMLObject()
	{
		if(m_pUMLObject)
			delete m_pUMLObject;
	}

	void cmdCreateUMLObject::undo()
	{
		if(!m_pUMLObject)
			return;

		if(m_pUMLView)
			m_pUMLView->getUMLDoc()->removeUMLObject(m_pUMLObject);
	}
	
	void cmdCreateUMLObject::redo()
	{
		if(!m_pUMLObject)
			return;

		if(m_pUMLView)
			m_pUMLView->addObject(m_pUMLObject);
	}
}
