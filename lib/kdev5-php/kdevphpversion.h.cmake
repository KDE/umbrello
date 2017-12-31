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
#ifndef KDEVPHP_VERSION_H
#define KDEVPHP_VERSION_H

#include <qglobal.h>

#define KDEVPHP_MAJOR_VERSION @KDEVPHP_VERSION_MAJOR@
#define KDEVPHP_MINOR_VERSION @KDEVPHP_VERSION_MINOR@
#define KDEVPHP_PATCH_VERSION @KDEVPHP_VERSION_PATCH@

#define KDEVPHP_VERSION_STR "@KDEVPHP_VERSION_MAJOR@.@KDEVPHP_VERSION_MINOR@.@KDEVPHP_VERSION_PATCH@"

#define KDEVPHP_VERSION QT_VERSION_CHECK(@KDEVPHP_VERSION_MAJOR@, @KDEVPHP_VERSION_MINOR@, @KDEVPHP_VERSION_PATCH@)

#endif
