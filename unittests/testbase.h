/*
    SPDX-FileCopyrightText: 2015, 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTBASE_H
#define TESTBASE_H

// qt includes
#include <QObject>
#include <QtTest>

#ifdef RUN_ALL
#undef QCOMPARE
#define QCOMPARE(actual, expected) \
    QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__)
#undef QVERIFY
#define QVERIFY(statement) \
    QTest::qVerify((statement), #statement, "", __FILE__, __LINE__)
#endif

#define IS_NOT_IMPL() QSKIP("not implemented yet", SkipSingle)

/**
 * The TestBase class is intended as base class for umbrello unit tests.
 *
 * Currently it provides an instance of class UMLApp, which is required
 * to run mostly unit tests.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class TestBase : public QObject
{
    Q_OBJECT
public:
    explicit TestBase(QObject  *parent = nullptr);

protected Q_SLOTS:
    virtual void initTestCase();
    virtual void cleanupTestCase();
    virtual void cleanupOnExit(QObject *p);

protected:
    QList<QPointer<QObject>> m_objectsToDelete;
};

/**
 * The TestCodeGeneratorBase class is intended as base class for code generator unit tests
 *
 * Currently it provides a path to a temporary directory, where generated code could be
 * placed into. The temporary path is set as default output path for any code generating.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class TestCodeGeneratorBase : public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    virtual void initTestCase();

protected:
    QString m_tempPath;  ///< holds path to temporary directory
    QString temporaryPath();
};

#include <QSignalBlocker>
typedef QSignalBlocker SignalBlocker;

/**
 * Set loading state to avoid signaling to tree view etc.
 */
class SetLoading
{
public:
    SetLoading();
    ~SetLoading();
protected:
    bool _state;
};

#include <QDomDocument>
#include <QXmlStreamWriter>
#include "umlapp.h"
#include "umldoc.h"

/**
 * template for adding test save/load support to UML related classe
 */
template <class T, typename N>
class TestUML : public T
{
public:
    TestUML() : T() {}
    TestUML(N name) : T(name) {}
    TestUML(N p1, UMLObject *p2, UMLObject *p3) : T(p1, p2, p3) {}
    QString testSave1();
    bool testLoad1(const QString& xml);
    void testDump(const QString &title = QString());
    UMLObject *secondary() const;
};

template <class T, typename N>
QString TestUML<T,N>::testSave1()
{
    QString xml;
    QXmlStreamWriter stream(&xml);
    stream.writeStartElement("unittest");
    T::saveToXMI(stream);
    stream.writeEndElement();
    return xml;
}

template <class T, typename N>
bool TestUML<T,N>::testLoad1(const QString& xml)
{
    QDomDocument qDoc;
    QString error;
    int line;
    if (!qDoc.setContent(xml, &error, &line))
        return false;
    QDomElement root = qDoc.childNodes().at(0).toElement();
    QDomElement e = root.childNodes().at(0).toElement();
    bool result = T::loadFromXMI(e);
    if (result) {
        const SignalBlocker sb(UMLApp::app()->document());
        result = T::resolveRef();
    }
    return result;
}

template <class T, typename N>
void TestUML<T,N>::testDump(const QString &title)
{
    QString xml = testSave1();
    qDebug() << title << xml;
}

// used by resolveRef() tests
template <class T, typename N>
UMLObject *TestUML<T,N>::secondary() const
{
    return T::m_pSecondary.data();
}

/**
 * template for adding test save/load support to widget related classes
 */
template <class T, typename N>
class TestWidget : public T
{
public:
    TestWidget(UMLScene *scene, N w) : T(scene, w) {}
    QString testSave1();
    bool testLoad1(const QString& xml);
    void testDump(const QString &title = QString());
};

template <class T, typename N>
QString TestWidget<T,N>::testSave1()
{
    QString xml;
    QXmlStreamWriter stream(&xml);
    stream.writeStartElement("unittest");
    T::saveToXMI(stream);
    stream.writeEndElement();
    return xml;
}

template <class T, typename N>
bool TestWidget<T,N>::testLoad1(const QString& xml)
{
    QDomDocument qDoc;
    QString error;
    int line;
    if (!qDoc.setContent(xml, &error, &line))
        return false;
    QDomElement root = qDoc.childNodes().at(0).toElement();
    QDomElement e = root.childNodes().at(0).toElement();
    bool result = T::loadFromXMI(e);
    if (result) {
        const SignalBlocker sb(UMLApp::app()->document());
        result = T::activate(nullptr);
    }
    return result;
}

template <class T, typename N>
void TestWidget<T,N>::testDump(const QString &title)
{
    QString xml = testSave1();
    qDebug() << title << xml;
}

#endif // TESTBASE_H
