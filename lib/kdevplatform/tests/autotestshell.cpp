/*
    SPDX-FileCopyrightText: 2013 Kevin Funk <kfunk@kde.org

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "autotestshell.h"

KDevelop::AutoTestShell::AutoTestShell(const QStringList& plugins)
    : m_plugins(plugins)
{
}

void KDevelop::AutoTestShell::init(const QStringList& plugins)
{
    // TODO: Maybe generalize, add KDEVELOP_STANDALONE build option
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    qputenv("KDE_FORK_SLAVES", "1"); // KIO slaves will be forked off instead of being started via DBus
#endif
    qputenv("CLEAR_DUCHAIN_DIR", "1"); // Always clear duchain dir (also to avoid popups asking the user to clear it)

    s_instance = new AutoTestShell(plugins);
}
