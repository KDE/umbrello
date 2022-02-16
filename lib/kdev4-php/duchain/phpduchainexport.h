/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2004 Jaroslaw Staniek <js@iidea.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPDUCHAINEXPORT_H
#define PHPDUCHAINEXPORT_H

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>


#ifndef KDEVPHPDUCHAIN_EXPORT
# ifdef MAKE_KDEV4PHPDUCHAIN_LIB
#  define KDEVPHPDUCHAIN_EXPORT KDE_EXPORT
# else
#  define KDEVPHPDUCHAIN_EXPORT KDE_IMPORT
# endif
#endif

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
