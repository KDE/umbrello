/*****************************************************************************
 * This file is part of the KDE project
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *******************************************************************************/
#ifndef KDEVPHPDOCS_VERSION_H
#define KDEVPHPDOCS_VERSION_H

#include <kdeversion.h>

#define KDEVPHPDOCS_MAJOR_VERSION @KDEVPHPDOCS_VERSION_MAJOR@
#define KDEVPHPDOCS_MINOR_VERSION @KDEVPHPDOCS_VERSION_MINOR@
#define KDEVPHPDOCS_PATCH_VERSION @KDEVPHPDOCS_VERSION_PATCH@

#define KDEVPHPDOCS_VERSION_STR "@KDEVPHPDOCS_VERSION_MAJOR@.@KDEVPHPDOCS_VERSION_MINOR@.@KDEVPHPDOCS_VERSION_PATCH@"

#define KDEVPHPDOCS_VERSION KDE_MAKE_VERSION(@KDEVPHPDOCS_VERSION_MAJOR@, @KDEVPHPDOCS_VERSION_MINOR@, @KDEVPHPDOCS_VERSION_PATCH@)

#endif
