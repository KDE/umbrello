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

#include "cmd_resizeWidget.h"

#include "uml.h"

// app includes
#include "umlwidgetcontroller.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlnamespace.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "listpopupmenu.h"
#include "classifierwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"

#include <klocale.h>

namespace Uml
{

cmdResizeWidget::cmdResizeWidget(UMLWidgetController* _UMLwc):UMLwc(_UMLwc),already(false)
{
    UMLWidget * w = _UMLwc->getWidget();
    W = w->width();
    H = w->height();
    oldH = _UMLwc->getOldH();
    oldW = _UMLwc->getOldW();
}

cmdResizeWidget::~cmdResizeWidget()
{

}

void cmdResizeWidget::redo()
{
    if (already == false)
    {already = true;}
    else{UMLwc->resizeWidget(W,H);}
}

void cmdResizeWidget::undo()
{
    UMLwc->resizeWidget(oldW,oldH);
}

}
