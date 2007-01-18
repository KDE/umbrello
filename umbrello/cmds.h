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

#ifndef __CMDS_H__
#define __CMDS_H__

#include <QUndoCommand>
#include <QUndoStack>

#include "cmds/cmd_create_activitydiag.h"
#include "cmds/cmd_create_classdiag.h"
#include "cmds/cmd_create_collaborationdiag.h"
#include "cmds/cmd_create_componentdiag.h"
#include "cmds/cmd_create_deploydiag.h"
#include "cmds/cmd_create_entityrelationdiag.h"
#include "cmds/cmd_create_seqdiag.h"
#include "cmds/cmd_create_statediag.h"
#include "cmds/cmd_create_usecasediag.h"
#include "cmds/cmd_moveWidget.h"

/************************************************************
 * Widgets
 ************************************************************/

#include "cmds/widgets/cmd_create_umlobject.h"

/************************************************************
 * Class Diagram
 ************************************************************/

#include "cmds/classdiag/cmd_create_class.h"

#endif
