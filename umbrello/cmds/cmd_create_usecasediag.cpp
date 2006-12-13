#include "cmd_create_usecasediag.h"

#include "uml.h"

#include <klocale.h>

namespace Uml
{

	cmdCreateUseCaseDiag::cmdCreateUseCaseDiag(UMLDoc* doc, const QString& name):m_pUMLDoc(doc),m_pUMLView(NULL),m_Name(name)
	{
		setText(i18n("Create class diagram"));
	}
	
	cmdCreateUseCaseDiag::~cmdCreateUseCaseDiag()
	{
		if(m_pUMLView)
			delete m_pUMLView;
	}
	
	void cmdCreateUseCaseDiag::redo()
	{
		UMLFolder* temp = m_pUMLDoc->getRootFolder(Uml::mt_Logical);
		m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::dt_UseCase);
	}
	
	void cmdCreateUseCaseDiag::undo()
	{
		if(m_pUMLView)
		{
			m_pUMLDoc->removeDiagram(m_pUMLView->getID());
			delete m_pUMLView;
		}
	}

}
