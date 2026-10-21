/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTMESSAGEWIDGET_H
#define TESTMESSAGEWIDGET_H

#include "testbase.h"

class TestMessageWidget : public TestBase
{
    Q_OBJECT

private Q_SLOTS:
    void test_selfMessageResizeArea();
};

#endif // TESTMESSAGEWIDGET_H
