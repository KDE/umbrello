/*
    SPDX-FileCopyrightText: 2015-2020 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testbase.h"

// app includes
#include "codegenerationpolicy.h"
#include "uml.h"
#include "umldoc.h"

// qt includes
#include <QApplication>

#if !defined(QT_GUI_LIB)
#error umbrello unittests require QT_GUI_LIB to be present
#endif

#if QT_VERSION < 0x050000
#include <KTempDir>
#endif

#if QT_VERSION >= 0x050000
#include <QTemporaryDir>
#endif

TestBase::TestBase(QObject *parent)
  : QObject(parent)
{
}

void TestBase::initTestCase()
{
    QWidget *w = new QWidget;
    UMLApp *app = new UMLApp(w);
    app->setActiveLanguage(Uml::ProgrammingLanguage::Cpp);
}

void TestBase::cleanupTestCase()
{
    foreach(QObject *p, m_objectsToDelete) {
        delete p;
    }
    delete UMLApp::app();
}

void TestBase::cleanupOnExit(QObject *p)
{
    m_objectsToDelete.append(p);
}

void TestCodeGeneratorBase::initTestCase()
{
    TestBase::initTestCase();

#if QT_VERSION >= 0x050000
    static QTemporaryDir tmpDir;
    m_tempPath = tmpDir.path() + QLatin1String("/");
#else
    static KTempDir tmpDir;
    m_tempPath = tmpDir.name();
#endif
    UMLApp::app()->commonPolicy()->setOutputDirectory(m_tempPath);
}

/**
 * Return temporary path usable to generated code.
 * @return
 */
QString TestCodeGeneratorBase::temporaryPath()
{
    return m_tempPath;
}

SetLoading::SetLoading()
{
    _state = UMLApp::app()->document()->loading();
    UMLApp::app()->document()->setLoading();
}

SetLoading::~SetLoading()
{
    UMLApp::app()->document()->setLoading(_state);
}
