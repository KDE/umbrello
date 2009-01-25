/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2008                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "cmd_changeLineColor.h"

// app includes
#include "umlwidgetcontroller.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlnamespace.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "classifierwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"

#include <klocale.h>

namespace Uml
{

    /*CmdChangeLineColor::CmdChangeLineColor(UMLView *view, const QColor& col)
    {
        setText(i18n("Change Line Color"));
        UMLWidget * widget = view->getFirstMultiSelectedWidget();
        pView=view;
        color = col;
        oldColor=widget->getLineColor() ;
    }*/

CmdChangeLineColor::CmdChangeLineColor(UMLWidget *w, const QColor& col):UMLw(w),color(col)
{
    setText(i18n("Change Line Color") + w->getName());
    oldColor= w -> getLineColor() ;

}
    CmdChangeLineColor::~CmdChangeLineColor()
    {
    }
    void CmdChangeLineColor::redo()
    {

        UMLw->setLineColorcmd( color );
    }

    void CmdChangeLineColor::undo()
    {
        UMLw->setLineColorcmd( oldColor );
    }
}
