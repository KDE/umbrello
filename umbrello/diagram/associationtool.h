 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMBRELLO_ASSOCIATIONTOOL_H
#define UMBRELLO_ASSOCIATIONTOOL_H

#include "pathtool.h"


namespace Umbrello
{
class DiagramView;
class DiagramWidget;

class AssociationTool : public PathTool
{
public:
	AssociationTool( DiagramView  *view );
	virtual ~AssociationTool();
	
	virtual void activate();
	virtual void deactivate();
protected:
	virtual void setCursor( );
	
	virtual bool createNextPathPoint( );
	virtual void createPath( );
	
	virtual QPoint findNextPathPoint( );
	DiagramWidget *m_startWidget;
	DiagramWidget *m_underMouse;
};


}


#endif //#define UMBRELLO_ASSOCIATIONTOOL_H
