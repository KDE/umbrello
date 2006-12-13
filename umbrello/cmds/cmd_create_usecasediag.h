#ifndef __CMD_CREATE_USECASEDIAG__
#define __CMD_CREATE_USECASEDIAG__

#include <QUndoCommand>

#include "umldoc.h"
#include "umlview.h"

namespace Uml
{
	class cmdCreateUseCaseDiag : public QUndoCommand
	{
		public:
			cmdCreateUseCaseDiag(UMLDoc* doc, const QString& name);
			~cmdCreateUseCaseDiag();

			void redo();
			void undo();

		private:
			UMLDoc*		m_pUMLDoc;
			UMLView*	m_pUMLView;
			QString		m_Name;
	};
};

#endif
