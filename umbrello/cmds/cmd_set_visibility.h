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

#ifndef __CMD_SETVISIBILITY__
#define __CMD_SETVISIBILITY__

#include <QUndoCommand>


#include "umlwidget.h"

namespace Uml
{
	class cmdSetVisibility : public QUndoCommand
	{
		public:
			/*Constructor */
			cmdSetVisibility(UMLObject * _UMLObj, Uml::Visibility  _visibility);
			/*Destructor */
			~cmdSetVisibility();
			/*Redo method */
			void redo();
			/*Undo method */
			void undo();

		private:

			Uml::Visibility  oldvisibility;
			Uml::Visibility  visib;
			UMLObject * UMLObj;
	};
}

#endif
