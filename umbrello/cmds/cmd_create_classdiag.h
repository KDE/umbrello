#ifndef __CMD_CREATE_CLASSDIAG__
#define __CMD_CREATE_CLASSDIAG__

#include <QUndoCommand>

namespace umbrello.cmds
{
	class cmdCreateClassDiag : public QUndoCommand
	{
		public:
			cmdCreateClassDiag(UMLDoc* doc, const QString& name);
			~CmdCreateClassDiag();

			void redo();
			void undo();
		private:
			QString		m_Name;
			UMLDoc*		m_pUMLDoc;
			UMLView*	m_pUMLView;
	};
};
#endif