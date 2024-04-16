/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2006 Gael de Chalendar (aka Kleag) kleag@free.fr
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "xhtmlgenerator.h"

#define DBG_SRC QStringLiteral("XhtmlGenerator")
#include "debug_utils.h"
#include "docbook2xhtmlgeneratorjob.h"
#include "uml.h"
#include "umldoc.h"
#include "umlviewimageexportermodel.h"
#include "docbookgenerator.h"

#include <kjobwidgets.h>
#include <KLocalizedString>
#include <KMessageBox>
#include <kio/job.h>

#include <QApplication>
#include <QFile>
#include <QRegExp>
#include <QStandardPaths>
#include <QTextStream>

DEBUG_REGISTER(XhtmlGenerator)

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
    QUrl url = m_umlDoc->url();
    QString fileName = url.fileName();
    fileName.remove(QRegExp(QStringLiteral(".xmi$")));
    url.setPath(fileName);
    logDebug1("XhtmlGenerator::generateXhtmlForProject: Exporting to directory %1", url.path());
    return generateXhtmlForProjectInto(url);
}

/**
 * Exports the current model to XHTML in the given directory
 * @param destDir the directory where the XHTML file and the figures will
 * be written
 * @todo better handling of error conditions
 * @return true if saving is successful and false otherwise.
 */
bool XhtmlGenerator::generateXhtmlForProjectInto(const QUrl& destDir)
{
    logDebug0("XhtmlGenerator::generateXhtmlForProjectInto: First convert to docbook");
    m_destDir = destDir;
//    KUrl url(QString("file://")+m_tmpDir.name());
    DocbookGenerator* docbookGenerator = new DocbookGenerator;
    docbookGenerator->generateDocbookForProjectInto(destDir);

    logDebug0("XhtmlGenerator::generateXhtmlForProjectInto: Connecting...");
    connect(docbookGenerator, SIGNAL(finished(bool)), this, SLOT(slotDocbookToXhtml(bool)));
    return true;
}

/**
 * This slot is triggered when the first part, xmi to docbook, is finished
 * @param status   status to continue with converting
 */
void XhtmlGenerator::slotDocbookToXhtml(bool status)
{
    logDebug0("XhtmlGenerator::slotDocbookToXhtml: Now convert docbook to html...");
    if (!status) {
        logWarn0("XhtmlGenerator::slotDocbookToXhtml: Error in converting to docbook");
        m_pStatus = false;
        return;
    }
    else {
        QUrl url = m_umlDoc->url();
        QString fileName = url.fileName();
        fileName.replace(QRegExp(QStringLiteral(".xmi$")), QStringLiteral(".docbook"));
        url.setPath(m_destDir.path() + QLatin1Char('/') + fileName);
        m_umlDoc->writeToStatusBar(i18n("Generating XHTML..."));
        m_d2xg  = new Docbook2XhtmlGeneratorJob(url, this);
        connect(m_d2xg, SIGNAL(xhtmlGenerated(QString)),
                this, SLOT(slotHtmlGenerated(QString)));
        connect(m_d2xg, SIGNAL(finished()), this, SLOT(threadFinished()));
        logDebug0("XhtmlGenerator::slotDocbookToXhtml: Threading.");
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
    logDebug1("XhtmlGenerator: HTML generated %1", tmpFileName);
    QUrl url = m_umlDoc->url();
    QString fileName = url.fileName();
    fileName.replace(QRegExp(QStringLiteral(".xmi$")), QStringLiteral(".html"));
    url.setPath(m_destDir.path() + QLatin1Char('/') + fileName);
    KIO::Job* htmlCopyJob = KIO::file_copy(QUrl::fromLocalFile(tmpFileName), url, -1, KIO::Overwrite | KIO::HideProgressInfo);
    KJobWidgets::setWindow(htmlCopyJob, (QWidget*)UMLApp::app());
    htmlCopyJob->exec();
    if (!htmlCopyJob->error()) {
        m_umlDoc->writeToStatusBar(i18n("XHTML Generation Complete..."));
    } else {
        m_pStatus = false;
        return;
    }

    m_umlDoc->writeToStatusBar(i18n("Copying CSS..."));

    QString cssBaseName = QStringLiteral("xmi.css");
    QString cssFileName(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("umbrello5/") + cssBaseName));
    if (cssFileName.isEmpty())
        cssFileName = QStringLiteral(DOCGENERATORS_DIR) + QLatin1Char('/') + cssBaseName;

    QUrl cssUrl = m_destDir;
    cssUrl.setPath(cssUrl.path() + QLatin1Char('/') + cssBaseName);
    KIO::Job* cssJob = KIO::file_copy(QUrl::fromLocalFile(cssFileName), cssUrl, -1, KIO::Overwrite | KIO::HideProgressInfo);
    KJobWidgets::setWindow(cssJob, (QWidget*)UMLApp::app());
    cssJob->exec();
    if (!cssJob->error()) {
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

    Q_EMIT finished(m_pStatus);
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
 * return custom xsl file for generating html
 *
 * @return filename with path
 */
QString XhtmlGenerator::customXslFile()
{
  QString xslBaseName = QStringLiteral("docbook2xhtml.xsl");
    QString xsltFileName(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("umbrello5/") + xslBaseName));
  if (xsltFileName.isEmpty())
      xsltFileName = QStringLiteral(DOCGENERATORS_DIR) + QLatin1Char('/') + xslBaseName;

  logDebug1("XhtmlGenerator::customXslFile returning %1", xsltFileName);
  return xsltFileName;
}
