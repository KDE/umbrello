  /***************************************************************************
                               classtool.h
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMBRELLO_CLASSTOOL_H
#define UMBRELLO_CLASSTOOL_H

#include "../../diagram/tool.h"

class QMouseEvent;

namespace Umbrello
{
class DiagramView;


class ClassTool : public Tool
{
public:
	ClassTool( DiagramView  *view );
	virtual ~ClassTool();
	
	virtual void activate();
	virtual void deactivate();
protected:
	virtual void setCursor( );
	virtual bool mouseReleaseEvent( );
};


}


#endif //UMBRELLO_CLASSTOOL_H
