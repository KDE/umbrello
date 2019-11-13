/*
    Copyright 2015  Ralf Habacker  <ralf.habacker@freenet.de>

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

#include "testbase.h"

// app includes
#include "codegenerationpolicy.h"
#include "uml.h"

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
 * Return temporay path usable to generated code.
 * @return
 */
QString TestCodeGeneratorBase::temporaryPath()
{
    return m_tempPath;
}
