#include "cmd_create_collaborationdiag.h"

#include "uml.h"

#include <klocale.h>

namespace Uml
{

	cmdCreateCollaborationDiag::cmdCreateCollaborationDiag(UMLDoc* doc, const QString& name):m_pUMLDoc(doc),m_pUMLView(NULL),m_Name(name)
	{
		setText(i18n("Create collaboration diagram"));
	}
	
	cmdCreateCollaborationDiag::~cmdCreateCollaborationDiag()
	{
		if(m_pUMLView)
			delete m_pUMLView;
	}
	
	void cmdCreateCollaborationDiag::redo()
	{
		UMLFolder* temp = m_pUMLDoc->getRootFolder(Uml::mt_Logical);
		m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::dt_Collaboration);
	}
	
	void cmdCreateCollaborationDiag::undo()
	{
		if(m_pUMLView)
		{
			m_pUMLDoc->removeDiagram(m_pUMLView->getID());
			delete m_pUMLView;
		}
	}

}
