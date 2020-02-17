/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006      Gael de Chalendar (aka Kleag) kleag@free.fr   *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "xhtmlgenerator.h"

#include "debug_utils.h"
#include "docbook2xhtmlgeneratorjob.h"
#include "uml.h"
#include "umldoc.h"
#include "umlviewimageexportermodel.h"
#include "docbookgenerator.h"

#if QT_VERSION >= 0x050000
#include <kjobwidgets.h>
#endif
#include <KLocalizedString>
#include <KMessageBox>
#if QT_VERSION < 0x050000
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#endif
#include <kio/job.h>

#include <QApplication>
#include <QFile>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif
#include <QTextStream>

/**
 * Constructor.
 */
XhtmlGenerator::XhtmlGenerator()
{
    m_umlDoc = UMLApp::app()->document();
    m_pStatus = true;
    m_pThreadFinished = false;
    m_d2xg = 0;
}

/**
 * Destructor.
 */
XhtmlGenerator::~XhtmlGenerator()
{
}

/**
 * Exports the current model to XHTML in a directory named as the model
 * with the .xmi suffix removed. The XHTML file will have the same name
 * with the .html suffix. Figures will be named as the corresponding
 * diagrams in the GUI
 * @todo change file naming to avoid paths with spaces or non-ASCII chars
 * @todo better handling of error conditions
 * @return true if saving is successful and false otherwise.
 */
bool XhtmlGenerator::generateXhtmlForProject()
{
#if QT_VERSION >= 0x050000
    QUrl url = m_umlDoc->url();
#else
    KUrl url = m_umlDoc->url();
#endif
    QString fileName = url.fileName();
    fileName.remove(QRegExp(QLatin1String(".xmi$")));
#if QT_VERSION >= 0x050000
    url.setPath(fileName);
#else
    url.setFileName(fileName);
#endif
    uDebug() << "Exporting to directory: " << url;
    return generateXhtmlForProjectInto(url);
}

/**
 * Exports the current model to XHTML in the given directory
 * @param destDir the directory where the XHTML file and the figures will
 * be written
 * @todo better handling of error conditions
 * @return true if saving is successful and false otherwise.
 */
#if QT_VERSION >= 0x050000
bool XhtmlGenerator::generateXhtmlForProjectInto(const QUrl& destDir)
#else
bool XhtmlGenerator::generateXhtmlForProjectInto(const KUrl& destDir)
#endif
{
    uDebug() << "First convert to docbook";
    m_destDir = destDir;
//    KUrl url(QString("file://")+m_tmpDir.name());
    DocbookGenerator* docbookGenerator = new DocbookGenerator;
    docbookGenerator->generateDocbookForProjectInto(destDir);

    uDebug() << "Connecting...";
    connect(docbookGenerator, SIGNAL(finished(bool)), this, SLOT(slotDocbookToXhtml(bool)));
    return true;
}

/**
 * This slot is triggerd when the first part, xmi to docbook, is finished
 * @param status   status to continue with converting
 */
void XhtmlGenerator::slotDocbookToXhtml(bool status)
{
    uDebug() << "Now convert docbook to html...";
    if (!status) {
        uDebug() << "Error in converting to docbook";
        m_pStatus = false;
        return;
    }
    else {
#if QT_VERSION >= 0x050000
        QUrl url = m_umlDoc->url();
#else
        KUrl url = m_umlDoc->url();
#endif
        QString fileName = url.fileName();
        fileName.replace(QRegExp(QLatin1String(".xmi$")), QLatin1String(".docbook"));
#if QT_VERSION >= 0x050000
        url.setPath(m_destDir.path() + QLatin1Char('/') + fileName);
#else
        url.setPath(m_destDir.path());
        url.addPath(fileName);
#endif
        m_umlDoc->writeToStatusBar(i18n("Generating XHTML..."));
        m_d2xg  = new Docbook2XhtmlGeneratorJob(url, this);
        connect(m_d2xg, SIGNAL(xhtmlGenerated(QString)),
                this, SLOT(slotHtmlGenerated(QString)));
        connect(m_d2xg, SIGNAL(finished()), this, SLOT(threadFinished()));
        uDebug() << "Threading";
        m_d2xg->start();
    }
}

/**
 * Triggered when the copying of the HTML result file is finished.
 * Emits the signal finished().
 * @param tmpFileName   temporary file name
 */
void XhtmlGenerator::slotHtmlGenerated(const QString& tmpFileName)
{
    uDebug() << "HTML Generated " << tmpFileName;
#if QT_VERSION >= 0x050000
    QUrl url = m_umlDoc->url();
#else
    KUrl url = m_umlDoc->url();
#endif
    QString fileName = url.fileName();
    fileName.replace(QRegExp(QLatin1String(".xmi$")), QLatin1String(".html"));
#if QT_VERSION >= 0x050000
    url.setPath(m_destDir.path() + QLatin1Char('/') + fileName);
#else
    url.setPath(m_destDir.path());
    url.addPath(fileName);
#endif
#if QT_VERSION >= 0x050000
    KIO::Job* htmlCopyJob = KIO::file_copy(QUrl::fromLocalFile(tmpFileName), url, -1, KIO::Overwrite | KIO::HideProgressInfo);
    KJobWidgets::setWindow(htmlCopyJob, (QWidget*)UMLApp::app());
    htmlCopyJob->exec();
    if (!htmlCopyJob->error()) {
#else
    KIO::Job* htmlCopyJob = KIO::file_copy(KUrl::fromPath(tmpFileName), url, -1, KIO::Overwrite | KIO::HideProgressInfo);
    if (KIO::NetAccess::synchronousRun(htmlCopyJob, (QWidget*)UMLApp::app())) {
#endif
        m_umlDoc->writeToStatusBar(i18n("XHTML Generation Complete..."));
    } else {
        m_pStatus = false;
        return;
    }

    m_umlDoc->writeToStatusBar(i18n("Copying CSS..."));

    QString cssBaseName = QLatin1String("xmi.css");
#if QT_VERSION >= 0x050000
    QString cssFileName(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("umbrello5/") + cssBaseName));
#else
    QString cssFileName(KGlobal::dirs()->findResource("data", QLatin1String("umbrello/") + cssBaseName));
#endif
    if (cssFileName.isEmpty())
        cssFileName = QLatin1String(DOCGENERATORS_DIR) + QLatin1Char('/') + cssBaseName;

#if QT_VERSION >= 0x050000
    QUrl cssUrl = m_destDir;
    cssUrl.setPath(cssUrl.path() + QLatin1Char('/') + cssBaseName);
#else
    KUrl cssUrl = m_destDir;
    cssUrl.addPath(cssBaseName);
#endif
#if QT_VERSION >= 0x050000
    KIO::Job* cssJob = KIO::file_copy(QUrl::fromLocalFile(cssFileName), cssUrl, -1, KIO::Overwrite | KIO::HideProgressInfo);
    KJobWidgets::setWindow(cssJob, (QWidget*)UMLApp::app());
    cssJob->exec();
    if (!cssJob->error()) {
#else
    KIO::Job* cssJob = KIO::file_copy(cssFileName, cssUrl, -1, KIO::Overwrite | KIO::HideProgressInfo);

    if (KIO::NetAccess::synchronousRun(cssJob, (QWidget*)UMLApp::app())) {
#endif
        m_umlDoc->writeToStatusBar(i18n("Finished Copying CSS..."));
        m_pStatus = true;
    } else {
        m_umlDoc->writeToStatusBar(i18n("Failed Copying CSS..."));
        m_pStatus = false;
    }

    while (m_pThreadFinished == false) {
        // wait for thread to finish
        qApp->processEvents();
    }

    emit finished(m_pStatus);
}

/**
 * Invoked when a thread is finished
 */
void XhtmlGenerator::threadFinished()
{
    m_pThreadFinished = true;
    delete m_d2xg;
    m_d2xg = 0;
}

/**
 * return local dookbool xsl file for generating html
 *
 * @return filename if present
 */
QString XhtmlGenerator::localDocbookXslFile()
{
    QString xslFileName = QLatin1String("xml/docbook/stylesheet/nwalsh/current/html/docbook.xsl");
#if QT_VERSION >= 0x050000
    QString localXsl = QStandardPaths::locate(QStandardPaths::GenericDataLocation, xslFileName);
#else
    QString localXsl = KGlobal::dirs()->findResource("data", QLatin1String("../../") + xslFileName);
#endif
    QFileInfo fi(localXsl);
    return fi.canonicalFilePath();
}

/**
 * return custom xsl file for generating html
 *
 * @return filename
 */
QString XhtmlGenerator::customXslFile()
{

  QString xslBaseName = QLatin1String("docbook2xhtml.xsl");
#if QT_VERSION >= 0x050000
    QString xsltFileName(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("umbrello5/") + xslBaseName));
#else
    QString xsltFileName(KGlobal::dirs()->findResource("data", QLatin1String("umbrello/") + xslBaseName));
#endif
  if (xsltFileName.isEmpty())
      xsltFileName = QLatin1String(DOCGENERATORS_DIR) + QLatin1Char('/') + xslBaseName;

  return xsltFileName;
}
