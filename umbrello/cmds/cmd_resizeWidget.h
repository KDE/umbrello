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

#ifndef __CMD_RESIZEWIDGET__
#define __CMD_RESIZEWIDGET__

#include <QUndoCommand>


#include "umlwidget.h"

namespace Uml
{
	class cmdResizeWidget : public QUndoCommand
	{
		public:
			cmdResizeWidget(UMLWidgetController* _UMLwc);
			~cmdResizeWidget();

			void redo();
			void undo();

		private:
			UMLWidgetController* 	UMLwc;
			int 		H;
			int 		W;
			int 		oldH;
			int 		oldW;
			bool 		already;
	};
};

#endif
