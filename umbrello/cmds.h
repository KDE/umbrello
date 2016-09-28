/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2014                                                *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                   *
 ***************************************************************************/

#ifndef CMDS_H
#define CMDS_H

#if QT_VERSION < 0x050000
#include <kundostack.h>
#endif

#include <QUndoCommand>
#include <QUndoStack>

#include "cmds/cmdcreatediagram.h"
#include "cmds/cmdhandlerename.h"
#include "cmds/cmdremovediagram.h"
#include "cmds/cmdsetstereotype.h"
#include "cmds/cmdsetvisibility.h"

/************************************************************
 * Generic
 ************************************************************/
#include "cmds/generic/cmdcreateumlobject.h"
#include "cmds/generic/cmdremoveumlobject.h"
#include "cmds/generic/cmdrenameumlobject.h"
#include "cmds/generic/cmdrenameumlobjectinstance.h"  

/************************************************************
 * Widgets
 ************************************************************/
#include "cmds/widget/cmdchangefillcolor.h"
#include "cmds/widget/cmdchangefont.h"
#include "cmds/widget/cmdchangelinecolor.h"
#include "cmds/widget/cmdchangelinewidth.h"
#include "cmds/widget/cmdchangemultiplicity.h"
#include "cmds/widget/cmdchangetextcolor.h"
#include "cmds/widget/cmdchangeusefillcolor.h"
#include "cmds/widget/cmdchangevisualproperty.h"
#include "cmds/widget/cmdcreatewidget.h"
#include "cmds/widget/cmdmovewidget.h"
#include "cmds/widget/cmdremovewidget.h"
#include "cmds/widget/cmdresizewidget.h"
#include "cmds/widget/cmdsetname.h"
#include "cmds/widget/cmdsettxt.h"

#endif
