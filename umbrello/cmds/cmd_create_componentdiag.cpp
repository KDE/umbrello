#include "cmd_create_componentdiag.h"

#include "uml.h"

#include <klocale.h>

namespace Uml
{

	cmdCreateComponentDiag::cmdCreateComponentDiag(UMLDoc* doc, const QString& name):m_pUMLDoc(doc),m_pUMLView(NULL),m_Name(name)
	{
		setText(i18n("Create conpoment diagram"));
	}
	
	cmdCreateComponentDiag::~cmdCreateComponentDiag()
	{
		if(m_pUMLView)
			delete m_pUMLView;
	}
	
	void cmdCreateComponentDiag::redo()
	{
		UMLFolder* temp = m_pUMLDoc->getRootFolder(Uml::mt_Logical);
		m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::dt_Component);
	}
	
	void cmdCreateComponentDiag::undo()
	{
		if(m_pUMLView)
		{
			m_pUMLDoc->removeDiagram(m_pUMLView->getID());
			delete m_pUMLView;
		}
	}

}
