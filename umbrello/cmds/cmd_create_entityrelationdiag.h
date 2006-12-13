#ifndef __CMD_CREATE_ENTITYRELATIONDIAG__
#define __CMD_CREATE_ENTITYRELATIONDIAG__

#include <QUndoCommand>

#include "umldoc.h"
#include "umlview.h"

namespace Uml
{
	class cmdCreateEntityRelationDiag : public QUndoCommand
	{
		public:
			cmdCreateEntityRelationDiag(UMLDoc* doc, const QString& name);
			~cmdCreateEntityRelationDiag();

			void redo();
			void undo();

		private:
			UMLDoc*		m_pUMLDoc;
			UMLView*	m_pUMLView;
			QString		m_Name;
	};
};

#endif
