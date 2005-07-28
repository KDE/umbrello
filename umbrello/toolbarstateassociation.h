/*
 *  copyright (C) 2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TOOLBARSTATEASSOCIATION_H
#define TOOLBARSTATEASSOCIATION_H

#include "toolbarstatepool.h"
//Added by qt3to4:
#include <QMouseEvent>


class QMouseEvent;
class UMLView;

class ToolBarStateAssociation : public ToolBarStatePool
{

public:
    ToolBarStateAssociation(UMLView *umlView);
    virtual ~ToolBarStateAssociation();

    virtual void mousePress(QMouseEvent* ome);
    virtual void mouseRelease(QMouseEvent* ome);
    virtual void mouseDoubleClick(QMouseEvent* ome);
    virtual void mouseMove(QMouseEvent* ome);

    virtual void init();
};

#endif //TOOLBARSTATEASSOCIATION_H
