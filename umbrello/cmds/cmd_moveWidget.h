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

#ifndef __CMD_MOVEWIDGET__
#define __CMD_MOVEWIDGET__

#include <QUndoCommand>

#include "umlwidget.h"
#include "cmd_id.h"

namespace Uml
{
	class cmdMoveWidget : public QUndoCommand
	{
		public:
			cmdMoveWidget(UMLWidgetController* _UMLwc);
			~cmdMoveWidget();

			void redo();
			void undo();

			bool mergeWith(const QUndoCommand* other);

		private:
			UMLWidgetController* 	UMLwc;
			int 		X;
			int 		Y;
			int 		oldX;
			int 		oldY;
			bool		already;

	};
};

#endif
