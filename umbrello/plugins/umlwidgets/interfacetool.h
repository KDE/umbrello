  /***************************************************************************
                               interfacetool.h
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

#ifndef UMBRELLO_INTERFACETOOL_H
#define UMBRELLO_INTERFACETOOL_H

#include "../../diagram/tool.h"

class QMouseEvent;

namespace Umbrello
{
class DiagramView;


class InterfaceTool : public Tool
{
public:
	InterfaceTool( DiagramView  *view );
	virtual ~InterfaceTool();
	
	virtual void activate();
	virtual void deactivate();
protected:
	virtual void setCursor( );
	virtual bool mouseReleaseEvent( );
};


}


#endif //UMBRELLO_INTERFACETOOL_H
