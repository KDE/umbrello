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
