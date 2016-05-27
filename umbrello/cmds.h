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

#include "cmds/cmd_createDiagram.h"
#include "cmds/cmd_handleRename.h"
#include "cmds/cmd_removeDiagram.h"
#include "cmds/cmd_setStereotype.h"
#include "cmds/cmd_setVisibility.h"
#include "cmds/cmd_setQualifier.h"

/************************************************************
 * Generic
 ************************************************************/
#include "cmds/generic/cmd_createUMLObject.h"
#include "cmds/generic/cmd_removeUMLObject.h"
#include "cmds/generic/cmd_renameUMLObject.h"

/************************************************************
 * Widgets
 ************************************************************/
#include "cmds/widget/cmd_changeFillColor.h"
#include "cmds/widget/cmd_changeFont.h"
#include "cmds/widget/cmd_changeLineColor.h"
#include "cmds/widget/cmd_changeLineWidth.h"
#include "cmds/widget/cmd_changeMultiplicity.h"
#include "cmds/widget/cmd_changeTextColor.h"
#include "cmds/widget/cmd_changeUseFillColor.h"
#include "cmds/widget/cmd_changeVisualProperty.h"
#include "cmds/widget/cmd_createWidget.h"
#include "cmds/widget/cmd_moveWidget.h"
#include "cmds/widget/cmd_removeWidget.h"
#include "cmds/widget/cmd_resizeWidget.h"
#include "cmds/widget/cmd_setName.h"
#include "cmds/widget/cmd_setTxt.h"

#endif
