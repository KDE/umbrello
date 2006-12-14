/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

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
			cmdCreateSeqDiag(UMLDoc* doc, const QString& name = "");
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
