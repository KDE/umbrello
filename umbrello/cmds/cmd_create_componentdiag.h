#ifndef __CMD_CREATE_COMPONENTDIAG__
#define __CMD_CREATE_COMPONENTDIAG__

#include <QUndoCommand>

#include "umldoc.h"
#include "umlview.h"

namespace Uml
{
	class cmdCreateComponentDiag : public QUndoCommand
	{
		public:
			cmdCreateComponentDiag(UMLDoc* doc, const QString& name);
			~cmdCreateComponentDiag();

			void redo();
			void undo();

		private:
			UMLDoc*		m_pUMLDoc;
			UMLView*	m_pUMLView;
			QString		m_Name;
	};
};

#endif
