/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlappprivate.h"

#define DBG_SRC QLatin1String("UMLAppPrivate")
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
    dirList.append(QCoreApplication::applicationDirPath() + QLatin1String("/../doc/apphelp"));

    // determine path from installation
#if QT_VERSION > 0x050000
    QString name = QLocale().name();
    QStringList lang = name.split(QLatin1Char('_'));
    QStringList langList;
    langList.append(lang[0]);
    if (lang.size() > 1)
        langList.append(name);

    // from custom install
    foreach(const QString &lang, langList) {
        dirList.append(QCoreApplication::applicationDirPath() + QString(QLatin1String("/../share/doc/HTML/%1/umbrello/apphelp")).arg(lang));
    }
    dirList.append(QCoreApplication::applicationDirPath() + QLatin1String("/../share/doc/HTML/en/umbrello/apphelp"));

    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    // from real installation
    foreach(const QString &location, locations) {
        foreach(const QString &lang, langList) {
            dirList.append(QString(QLatin1String("%1/doc/HTML/%2/umbrello/apphelp")).arg(location).arg(lang));
        }
        dirList.append(QString(QLatin1String("%1/doc/HTML/en/umbrello/apphelp")).arg(location));
    }
#else
    KLocale *local = KGlobal::locale();
    QString lang = local->language();
    // from custom install
    dirList.append(QCoreApplication::applicationDirPath() + QString(QLatin1String("/../share/doc/HTML/%1/umbrello/apphelp")).arg(lang));
    dirList.append(QCoreApplication::applicationDirPath() + QLatin1String("/../share/doc/HTML/en/umbrello/apphelp"));

    // /usr/share/doc/kde
    dirList.append(KStandardDirs::installPath("html") + lang + QLatin1String("/umbrello/apphelp"));
#endif
    foreach(const QString &dir, dirList) {
        QString filePath = dir + QLatin1String("/index.cache.bz2");
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
    if (file.endsWith(QLatin1String(".cache.bz2"))) {
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

    html.replace(QLatin1String("<FILENAME filename=\"index.html\">"),QLatin1String(""));
    html.replace(QLatin1String("</FILENAME>"),QLatin1String(""));
//#define WITH_HEADER
#ifndef WITH_HEADER
#ifdef WEBKIT_WELCOMEPAGE
    html.replace(QLatin1String("<div id=\"header\""),QLatin1String("<div id=\"header\" hidden"));
    html.replace(QLatin1String("<div class=\"navCenter\""),QLatin1String("<div id=\"navCenter\" hidden"));
    html.replace(QLatin1String("<div id=\"footer\""), QLatin1String("<div id=\"footer\" hidden"));
#else
    html.replace(QLatin1String("<div id=\"header\""), QLatin1String("<!-- <div id=\"header\""));
    html.replace(QLatin1String("<div id=\"contentBody\""), QLatin1String("--> <div id=\"contentBody\""));
    html.replace(QLatin1String("<div id=\"footer\""), QLatin1String("<!-- <div id=\"footer\""));
    html.replace(QLatin1String("</div></body>"), QLatin1String("--> </div></body>"));
#endif
#else
    // replace help:/ urls in html file to be able to find css files and images from kde help system
#if QT_VERSION >= 0x050000
    QString path;
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    foreach(const QString &l, locations) {
        QString a = QString(QLatin1String("%1/doc/HTML/en/")).arg(l);
        QFileInfo fi(a);
        if (fi.exists()) {
            path = a;
            break;
        }
    }
#else
    QString path = KStandardDirs::installPath("html") +  QLatin1String("en/");
#endif
    QUrl url(QUrl::fromLocalFile(path));
    QByteArray a = url.toEncoded();
    html.replace(QLatin1String("help:/"), QString::fromLocal8Bit(a));
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
        editorWindow = new QDockWidget(QLatin1String("Editor"));
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
