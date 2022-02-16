/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2004 Jaroslaw Staniek <js@iidea.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PARSEREXPORT_H
#define PARSEREXPORT_H

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>


#ifndef KDEVPHPPARSER_EXPORT
# ifdef MAKE_KDEV4PHPPARSER_LIB
#  define KDEVPHPPARSER_EXPORT KDE_EXPORT
# else
#  define KDEVPHPPARSER_EXPORT KDE_IMPORT
# endif
#endif

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
