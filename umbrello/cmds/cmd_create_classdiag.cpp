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

#include "cmd_create_classdiag.h"

#include "uml.h"

#include <klocale.h>

namespace Uml
{

	cmdCreateClassDiag::cmdCreateClassDiag(UMLDoc* doc, const QString& name):m_pUMLDoc(doc),m_pUMLView(NULL),m_Name(name)
	{
		setText(i18n("Create class diagram"));
	}
	
	cmdCreateClassDiag::~cmdCreateClassDiag()
	{
		if(m_pUMLView)
			delete m_pUMLView;
	}
	
	void cmdCreateClassDiag::redo()
	{
		UMLFolder* temp = m_pUMLDoc->getRootFolder(Uml::mt_Logical);
		m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::dt_Class);
	}
	
	void cmdCreateClassDiag::undo()
	{
		if(m_pUMLView)
		{
			m_pUMLDoc->removeDiagram(m_pUMLView->getID());
			delete m_pUMLView;
		}
	}

}
