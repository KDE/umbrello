#ifndef __CMD_CREATE_DEPLOYDIAG__
#define __CMD_CREATE_DEPLOYDIAG__

#include <QUndoCommand>

#include "umldoc.h"
#include "umlview.h"

namespace Uml
{
	class cmdCreateDeployDiag : public QUndoCommand
	{
		public:
			cmdCreateDeployDiag(UMLDoc* doc, const QString& name);
			~cmdCreateDeployDiag();

			void redo();
			void undo();

		private:
			UMLDoc*		m_pUMLDoc;
			UMLView*	m_pUMLView;
			QString		m_Name;
	};
};

#endif
