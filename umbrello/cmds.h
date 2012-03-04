/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2012                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef CMDS_H
#define CMDS_H

#include <QtGui/QUndoCommand>
#include <QtGui/QUndoStack>

#include <kundostack.h>

#include "cmds/cmd_create_diagram.h"
#include "cmds/cmd_handle_rename.h"
#include "cmds/cmd_moveWidget.h"
#include "cmds/cmd_resizeWidget.h"
#include "cmds/cmd_set_stereotype.h"
#include "cmds/cmd_set_visibility.h"

/************************************************************
 * Generic
 ************************************************************/
#include "cmds/generic/cmd_create_umlobject.h"
#include "cmds/generic/cmd_rename_umlobject.h"

/************************************************************
 * Widgets
 ************************************************************/
#include "cmds/widget/cmd_changeFillColor.h"
#include "cmds/widget/cmd_changeFontSelection.h"
#include "cmds/widget/cmd_changeLineColor.h"
#include "cmds/widget/cmd_changeMulti.h"
#include "cmds/widget/cmd_changeTextColor.h"
#include "cmds/widget/cmd_create_widget.h"
#include "cmds/widget/cmd_set_name.h"
#include "cmds/widget/cmd_set_txt.h"

#endif
