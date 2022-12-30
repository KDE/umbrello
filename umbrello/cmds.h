/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDS_H
#define CMDS_H


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
