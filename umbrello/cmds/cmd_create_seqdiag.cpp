#include "cmd_create_seqdiag.h"

#include "uml.h"

#include <klocale.h>

namespace Uml
{

	cmdCreateSeqDiag::cmdCreateSeqDiag(UMLDoc* doc, const QString& name):m_pUMLDoc(doc),m_pUMLView(NULL),m_Name(name)
	{
		setText(i18n("Create sequence diagram"));
	}
	
	cmdCreateSeqDiag::~cmdCreateSeqDiag()
	{
		if(m_pUMLView)
			delete m_pUMLView;
	}
	
	void cmdCreateSeqDiag::redo()
	{
		UMLFolder* temp = m_pUMLDoc->getRootFolder(Uml::mt_Logical);
		m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::dt_Sequence);
	}
	
	void cmdCreateSeqDiag::undo()
	{
		if(m_pUMLView)
		{
			m_pUMLDoc->removeDiagram(m_pUMLView->getID());
			delete m_pUMLView;
		}
	}

}
