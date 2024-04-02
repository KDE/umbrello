/*
    SPDX-FileCopyrightText: 2016 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <qglobal.h>
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    // enable crash handler
    int *a = nullptr;
    *a = 1;
}
