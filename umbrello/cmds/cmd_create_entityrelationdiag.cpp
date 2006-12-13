#include "cmd_create_entityrelationdiag.h"

#include "uml.h"

#include <klocale.h>

namespace Uml
{

	cmdCreateEntityRelationDiag::cmdCreateEntityRelationDiag(UMLDoc* doc, const QString& name):m_pUMLDoc(doc),m_pUMLView(NULL),m_Name(name)
	{
		setText(i18n("Create class diagram"));
	}
	
	cmdCreateEntityRelationDiag::~cmdCreateEntityRelationDiag()
	{
		if(m_pUMLView)
			delete m_pUMLView;
	}
	
	void cmdCreateEntityRelationDiag::redo()
	{
		UMLFolder* temp = m_pUMLDoc->getRootFolder(Uml::mt_Logical);
		m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::dt_EntityRelationship);
	}
	
	void cmdCreateEntityRelationDiag::undo()
	{
		if(m_pUMLView)
		{
			m_pUMLDoc->removeDiagram(m_pUMLView->getID());
			delete m_pUMLView;
		}
	}

}
