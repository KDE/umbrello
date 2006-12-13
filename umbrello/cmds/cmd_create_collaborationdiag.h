#ifndef __CMD_CREATE_COLLABORATIONDIAG__
#define __CMD_CREATE_COLLABORATIONDIAG__

#include <QUndoCommand>

#include "umldoc.h"
#include "umlview.h"

namespace Uml
{
	class cmdCreateCollaborationDiag : public QUndoCommand
	{
		public:
			cmdCreateCollaborationDiag(UMLDoc* doc, const QString& name);
			~cmdCreateCollaborationDiag();

			void redo();
			void undo();

		private:
			UMLDoc*		m_pUMLDoc;
			UMLView*	m_pUMLView;
			QString		m_Name;
	};
};

#endif
