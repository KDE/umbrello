/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlappprivate.h"

#define DBG_SRC QStringLiteral("UMLAppPrivate")
#include "debug_utils.h"

#include <KFilterDev>

DEBUG_REGISTER(UMLAppPrivate)

/**
 * Find welcome.html file for displaying in the welcome window.
 *
 * @return path to welcome file or empty if not found
 */
QString UMLAppPrivate::findWelcomeFile()
{
    QStringList dirList;
    // from build dir
    dirList.append(QCoreApplication::applicationDirPath() + QStringLiteral("/../doc/apphelp"));

    // determine path from installation
    QString name = QLocale().name();
    QStringList lang = name.split(QLatin1Char('_'));
    QStringList langList;
    langList.append(lang[0]);
    if (lang.size() > 1)
        langList.append(name);

    // from custom install
    for(const QString &lang: langList) {
        dirList.append(QCoreApplication::applicationDirPath() + QString(QStringLiteral("/../share/doc/HTML/%1/umbrello/apphelp")).arg(lang));
    }
    dirList.append(QCoreApplication::applicationDirPath() + QStringLiteral("/../share/doc/HTML/en/umbrello/apphelp"));

    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    // from real installation
    for(const QString &location: locations) {
        for(const QString &lang: langList) {
            dirList.append(QString(QStringLiteral("%1/doc/HTML/%2/umbrello/apphelp")).arg(location).arg(lang));
        }
        dirList.append(QString(QStringLiteral("%1/doc/HTML/en/umbrello/apphelp")).arg(location));
    }
    for(const QString &dir: dirList) {
        QString filePath = dir + QStringLiteral("/index.cache.bz2");
        QFileInfo fi(filePath);
        if (fi.exists()) {
            DEBUG() << "UMLAppPrivate::findWelcomeFile found " << filePath;
            return filePath;
        }
        DEBUG() << "UMLAppPrivate::findWelcomeFile tried " << filePath << " (not found)";
    }
    return QString();
}

/**
 * Read welcome file for displaying in the welcome window.
 *
 * This method also patches out some unrelated stuff from
 * the html file intended or being displayed with khelpcenter.
 *
 * @return html content of welcome file
 */
QString UMLAppPrivate::readWelcomeFile(const QString &file)
{
    QString html;
    if (file.endsWith(QStringLiteral(".cache.bz2"))) {
        QIODevice *d =  KFilterDev::deviceForFile(file);
        if (!d->open(QIODevice::ReadOnly)) {
            uError() << "could not open archive " << file;
            return QString();
        }
        QByteArray data = d->readAll();
        html = QString::fromUtf8(data);
        d->close();
        delete d;
    } else {
        QFile f(file);
        if (!f.open(QIODevice::ReadOnly))
            return QString();
        QTextStream in(&f);
        html = in.readAll();
    }

    if (html.isEmpty()) {
        uError() << "Empty welcome page loaded" << file;
        return QString();
    }

    html.replace(QStringLiteral("<FILENAME filename=\"index.html\">"),QStringLiteral(""));
    html.replace(QStringLiteral("</FILENAME>"),QStringLiteral(""));
//#define WITH_HEADER
#ifndef WITH_HEADER
#ifdef WEBKIT_WELCOMEPAGE
    html.replace(QStringLiteral("<div id=\"header\""),QStringLiteral("<div id=\"header\" hidden"));
    html.replace(QStringLiteral("<div class=\"navCenter\""),QStringLiteral("<div id=\"navCenter\" hidden"));
    html.replace(QStringLiteral("<div id=\"footer\""), QStringLiteral("<div id=\"footer\" hidden"));
#else
    html.replace(QStringLiteral("<div id=\"header\""), QStringLiteral("<!-- <div id=\"header\""));
    html.replace(QStringLiteral("<div id=\"contentBody\""), QStringLiteral("--> <div id=\"contentBody\""));
    html.replace(QStringLiteral("<div id=\"footer\""), QStringLiteral("<!-- <div id=\"footer\""));
    html.replace(QStringLiteral("</div></body>"), QStringLiteral("--> </div></body>"));
#endif
#else
    // replace help:/ urls in html file to be able to find css files and images from kde help system
    QString path;
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for(const QString &l: locations) {
        QString a = QString(QStringLiteral("%1/doc/HTML/en/")).arg(l);
        QFileInfo fi(a);
        if (fi.exists()) {
            path = a;
            break;
        }
    }
    QUrl url(QUrl::fromLocalFile(path));
    QByteArray a = url.toEncoded();
    html.replace(QStringLiteral("help:/"), QString::fromLocal8Bit(a));
#endif
    return html;
}

bool UMLAppPrivate::openFileInEditor(const QUrl &file, int startCursor, int endCursor)
{
    if (editor == nullptr) {
        uError() << "could not get editor instance, which indicates an installation problem, see for kate[4]-parts package";
        return false;
    }

    if (file.isLocalFile()) {
        QFileInfo fi(file.toLocalFile());
        if (!fi.exists())
            return false;
    }

    if (!editorWindow) {
        editorWindow = new QDockWidget(QStringLiteral("Editor"));
        parent->addDockWidget(Qt::RightDockWidgetArea, editorWindow);
    }

    if (document) {
        editorWindow->setWidget(0);
        delete view;
        delete document;
    }
    document = editor->createDocument(0);
    view = document->createView(parent);
    view->document()->openUrl(file);
    view->document()->setReadWrite(false);
    if (startCursor != endCursor)
        view->setCursorPosition(KTextEditor::Cursor(startCursor, endCursor));
    KTextEditor::ConfigInterface *iface = qobject_cast<KTextEditor::ConfigInterface*>(view);
    if(iface)
        iface->setConfigValue(QString::fromLatin1("line-numbers"), true);

    editorWindow->setWidget(view);
    editorWindow->setVisible(true);
    return true;
}
