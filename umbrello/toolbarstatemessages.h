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

#ifndef TOOLBARSTATEMESSAGES_H
#define TOOLBARSTATEMESSAGES_H

#include <qpoint.h>
//Added by qt3to4:
#include <QMouseEvent>
#include "toolbarstatepool.h"


class QMouseEvent;
class UMLView;
class UMLWidget;
class Q3CanvasLine;

class ToolBarStateMessages : public ToolBarStatePool
{

public:
    ToolBarStateMessages(UMLView *umlView);
    virtual ~ToolBarStateMessages();

    virtual void mousePress(QMouseEvent* ome);
    virtual void mouseRelease(QMouseEvent* ome);
    virtual void mouseDoubleClick(QMouseEvent* ome);
    virtual void mouseMove(QMouseEvent* ome);

    virtual void init();

protected:
    Uml::Sequence_Message_Type getMessageType ();
    virtual bool setSelectedWidget(QMouseEvent * me);
    void removeLine();

protected:
    UMLWidget* m_pSelectedWidget;
    Q3CanvasLine* m_pLine;
    QPoint m_FirstMousePos;
};

#endif //TOOLBARSTATEMESSAGES_H
