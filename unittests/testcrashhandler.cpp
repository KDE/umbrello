/*
    SPDX-FileCopyrightText: 2016 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#include <QApplication>
#else
#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#endif

int main(int argc, char **argv)
{
#if QT_VERSION >= 0x050000
    QApplication app(argc, argv);
    // enable crash handler
#else
    KAboutData aboutData("testcrashhandler", 0, KLocalizedString(),
                         "0.0.0", KLocalizedString(), KAboutData::License_GPL,
                         KLocalizedString(), KLocalizedString(),
                         "");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;
#endif
    int *a = 0;
    *a = 1;
}
