/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* File:    inputdialog.h
   Purpose: For KDE-3.1 compatibility, make the old KLineEditDlg look like
            the KDE-3.2 KInputDialog.
   Usage:   #include "inputdialog.h" instead of
            #include <klineeditdlg.h> or <kinputdialog.h>
 */
#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <kdeversion.h>
#if !KDE_IS_VERSION(3,1,90)
# include <klineeditdlg.h>  //deprecated
# define KInputDialog       KLineEditDlg
#else
# include <kinputdialog.h>
#endif

#endif // INPUTDIALOG_H
