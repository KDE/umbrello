#ifndef __CMD_CREATE_ACTIVITYDIAG__
#define __CMD_CREATE_ACTIVITYDIAG__

#include <QUndoCommand>

#include "umldoc.h"
#include "umlview.h"

namespace Uml
{
	class cmdCreateActivityDiag : public QUndoCommand
	{
		public:
			cmdCreateActivityDiag(UMLDoc* doc, const QString& name);
			~cmdCreateActivityDiag();

			void redo();
			void undo();

		private:
			UMLDoc*		m_pUMLDoc;
			UMLView*	m_pUMLView;
			QString		m_Name;
	};
};

#endif
