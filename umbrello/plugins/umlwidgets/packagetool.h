  /***************************************************************************
                               packagetool.h
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

#ifndef UMBRELLO_PACKAGETOOL_H
#define UMBRELLO_PACKAGETOOL_H

#include "../../diagram/tool.h"
//Added by qt3to4:
#include <QMouseEvent>

class QMouseEvent;

namespace Umbrello
{
class DiagramView;


class PackageTool : public Tool
{
public:
	PackageTool( DiagramView  *view );
	virtual ~PackageTool();
	
	virtual void activate();
	virtual void deactivate();
protected:
	virtual void setCursor( );
	virtual bool mouseReleaseEvent( );
};


}


#endif //UMBRELLO_PACKAGETOOL_H
