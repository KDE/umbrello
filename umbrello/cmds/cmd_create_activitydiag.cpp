#include "cmd_create_activitydiag.h"

#include "uml.h"

#include <klocale.h>

namespace Uml
{

	cmdCreateActivityDiag::cmdCreateActivityDiag(UMLDoc* doc, const QString& name):m_pUMLDoc(doc),m_pUMLView(NULL),m_Name(name)
	{
		setText(i18n("Create activity diagram"));
	}
	
	cmdCreateActivityDiag::~cmdCreateActivityDiag()
	{
		if(m_pUMLView)
			delete m_pUMLView;
	}
	
	void cmdCreateActivityDiag::redo()
	{
		UMLFolder* temp = m_pUMLDoc->getRootFolder(Uml::mt_Logical);
		m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::dt_Activity);
	}
	
	void cmdCreateActivityDiag::undo()
	{
		if(m_pUMLView)
		{
			m_pUMLDoc->removeDiagram(m_pUMLView->getID());
			delete m_pUMLView;
		}
	}

}
