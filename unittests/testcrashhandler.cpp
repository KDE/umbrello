/*
    Copyright 2016  Ralf Habacker  <ralf.habacker@freenet.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
