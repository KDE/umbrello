#include "cmd_create_deploydiag.h"

#include "uml.h"

#include <klocale.h>

namespace Uml
{

	cmdCreateDeployDiag::cmdCreateDeployDiag(UMLDoc* doc, const QString& name):m_pUMLDoc(doc),m_pUMLView(NULL),m_Name(name)
	{
		setText(i18n("Create deployment diagram"));
	}
	
	cmdCreateDeployDiag::~cmdCreateDeployDiag()
	{
		if(m_pUMLView)
			delete m_pUMLView;
	}
	
	void cmdCreateDeployDiag::redo()
	{
		UMLFolder* temp = m_pUMLDoc->getRootFolder(Uml::mt_Logical);
		m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::dt_Deployment);
	}
	
	void cmdCreateDeployDiag::undo()
	{
		if(m_pUMLView)
		{
			m_pUMLDoc->removeDiagram(m_pUMLView->getID());
			delete m_pUMLView;
		}
	}

}
