#ifndef __CMD_CREATE_SEQDIAG__
#define __CMD_CREATE_SEQDIAG__

#include <QUndoCommand>

#include "umldoc.h"
#include "umlview.h"

namespace Uml
{
	class cmdCreateSeqDiag : public QUndoCommand
	{
		public:
			cmdCreateSeqDiag(UMLDoc* doc, const QString& name);
			~cmdCreateSeqDiag();

			void redo();
			void undo();

		private:
			UMLDoc*		m_pUMLDoc;
			UMLView*	m_pUMLView;
			QString		m_Name;
	};
};

#endif
