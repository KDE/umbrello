/*
    SPDX-FileCopyrightText: 2015-2020 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testbase.h"

// app includes
#include "codegenerationpolicy.h"
#include "umlapp.h"
#include "umldoc.h"

// KDE includes
#include <KLocalizedString>

// qt includes
#include <QApplication>
#include <QTemporaryDir>

#if !defined(QT_GUI_LIB)
#error umbrello unittests require QT_GUI_LIB to be present
#endif

static QtMessageHandler defaultHandler = nullptr;

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (type == QtWarningMsg && msg.contains(QStringLiteral("The window title does not contain a '[*]' placeholder"))) {
        return;
    }
    defaultHandler(type, context, msg);
}

TestBase::TestBase(QObject *parent)
  : QObject(parent)
{
}

void TestBase::initTestCase()
{
    // used in UMLListView::setDocument()
    qApp->setProperty("umbrello_unittest", true);

    KLocalizedString::setApplicationDomain("umbrello");

    // hide Qt warning
    // QWARN  : ... QWidget::setWindowModified: The window title does not contain a '[*]' placeholder
    defaultHandler = qInstallMessageHandler(customMessageHandler);

    QWidget *w = new QWidget;
    UMLApp *app = new UMLApp(w);
    app->setup();
    app->setActiveLanguage(Uml::ProgrammingLanguage::Cpp);
}

void TestBase::cleanupTestCase()
{
    qDeleteAll(m_objectsToDelete);
    delete UMLApp::app();
}

void TestBase::cleanupOnExit(QObject *p)
{
    m_objectsToDelete.append(p);
}

void TestCodeGeneratorBase::initTestCase()
{
    TestBase::initTestCase();

    static QTemporaryDir tmpDir;
    m_tempPath = tmpDir.path() + QStringLiteral("/");
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
