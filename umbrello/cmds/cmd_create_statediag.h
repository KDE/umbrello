#ifndef __CMD_CREATE_STATEDIAG__
#define __CMD_CREATE_STATEDIAG__

#include <QUndoCommand>

#include "umldoc.h"
#include "umlview.h"

namespace Uml
{
	class cmdCreateStateDiag : public QUndoCommand
	{
		public:
			cmdCreateStateDiag(UMLDoc* doc, const QString& name);
			~cmdCreateStateDiag();

			void redo();
			void undo();

		private:
			UMLDoc*		m_pUMLDoc;
			UMLView*	m_pUMLView;
			QString		m_Name;
	};
};

#endif
