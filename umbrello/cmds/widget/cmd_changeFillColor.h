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

 /* Created By Krzywda Stanislas and Bouchikhi Mohamed-Amine ;) */

#ifndef CMD_CHANGEFILLCOLOR_H_
#define CMD_CHANGEFILLCOLOR_H_

#include <QUndoCommand>

#include "umlwidget.h"

namespace Uml
{

	class cmdChangeFillColor : public QUndoCommand
	{
		
		
		UMLWidget *UMLw;
				
		/* ancienne couleur */
		QColor oldColor;
				
		/* nouvelle couleur */
		QColor color;
				
		public:
			cmdChangeFillColor(UMLWidget *w, QColor col);
			~cmdChangeFillColor();
			void redo();
			void undo();
	};
}

#endif /*CMD_CHANGEFILLCOLOR_H_*/
