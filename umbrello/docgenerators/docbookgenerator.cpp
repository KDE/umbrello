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

#include "docbookgenerator.h"

#include "debug_utils.h"
#include "docbookgeneratorjob.h"
#include "uml.h"
#include "umldoc.h"
#include "umlviewimageexportermodel.h"

#if QT_VERSION >= 0x050000
#include <kjobwidgets.h>
#endif
#include <KLocalizedString>
#include <KMessageBox>
#if QT_VERSION < 0x050000
#include <kio/netaccess.h>
#endif
#include <kio/job.h>

#include <QApplication>
#include <QFile>
#include <QRegExp>
#include <QTextStream>

/**
 * Constructor.
 */
DocbookGenerator::DocbookGenerator()
{
  umlDoc = UMLApp::app()->document();
  m_pStatus = true;
  m_pThreadFinished = false;
  docbookGeneratorJob = 0;
}

/**
 * Destructor.
 */
DocbookGenerator::~DocbookGenerator()
{
}

/**
 * Exports the current model to docbook in a directory named as the model
 * with the .xmi suffix removed. The docbook file will have the same name
 * with the .docbook suffix. Figures will be named as the corresponding
 * diagrams in the GUI
 * @todo change file naming to avoid paths with spaces or non-ASCII chars
 * @todo better handling of error conditions
 * @return true if saving is successful and false otherwise.
 */
bool DocbookGenerator::generateDocbookForProject()
{
#if QT_VERSION >= 0x050000
    QUrl url = umlDoc->url();
#else
    KUrl url = umlDoc->url();
#endif
  QString fileName = url.fileName();
  fileName.remove(QRegExp(QLatin1String(".xmi$")));
#if QT_VERSION >= 0x050000
  url.setPath(url.path() + QLatin1Char('/') + fileName);
#else
  url.setFileName(fileName);
#endif
  uDebug() << "Exporting to directory: " << url;
  generateDocbookForProjectInto(url);
  return true;
}

/**
 * Exports the current model to docbook in the given directory
 * @param destDir the directory where the docbook file and the figures will
 * be written
 * @todo better handling of error conditions
 * @return true if saving is successful and false otherwise.
 */
#if QT_VERSION >= 0x050000
void DocbookGenerator::generateDocbookForProjectInto(const QUrl& destDir)
#else
void DocbookGenerator::generateDocbookForProjectInto(const KUrl& destDir)
#endif
{
    m_destDir = destDir;
    umlDoc->writeToStatusBar(i18n("Exporting all views..."));

    QStringList errors = UMLViewImageExporterModel().exportAllViews(
        UMLViewImageExporterModel::mimeTypeToImageType(QLatin1String("image/png")), destDir, false);
    if (!errors.empty()) {
        KMessageBox::errorList(UMLApp::app(), i18n("Some errors happened when exporting the images:"), errors);
        return;
    }

    umlDoc->writeToStatusBar(i18n("Generating Docbook..."));

    docbookGeneratorJob = new DocbookGeneratorJob(this);
    connect(docbookGeneratorJob, SIGNAL(docbookGenerated(QString)), this, SLOT(slotDocbookGenerationFinished(QString)));
    connect(docbookGeneratorJob, SIGNAL(finished()), this, SLOT(threadFinished()));
    uDebug()<<"Threading";
    docbookGeneratorJob->start();
}

void DocbookGenerator::slotDocbookGenerationFinished(const QString& tmpFileName)
{
    uDebug() << "Generation Finished" << tmpFileName;
#if QT_VERSION >= 0x050000
    QUrl url = umlDoc->url();
#else
    KUrl url = umlDoc->url();
#endif
    QString fileName = url.fileName();
    fileName.replace(QRegExp(QLatin1String(".xmi$")), QLatin1String(".docbook"));
#if QT_VERSION >= 0x050000
    url.setPath(m_destDir.path() + QLatin1Char('/') + fileName);
#else
    url.setPath(m_destDir.path());
    url.addPath(fileName);
#endif
#if QT_VERSION >= 0x050000
    KIO::Job* job = KIO::file_copy(QUrl(tmpFileName), url, -1, KIO::Overwrite | KIO::HideProgressInfo);
    KJobWidgets::setWindow(job, (QWidget*)UMLApp::app());
    job->exec();
    if (!job->error()) {
#else
    KIO::Job* job = KIO::file_copy(KUrl::fromPath(tmpFileName), url, -1, KIO::Overwrite | KIO::HideProgressInfo);
    if (KIO::NetAccess::synchronousRun(job, (QWidget*)UMLApp::app())) {
#endif
        umlDoc->writeToStatusBar(i18n("Docbook Generation Complete..."));
        m_pStatus = true;
    } else {
        umlDoc->writeToStatusBar(i18n("Docbook Generation Failed..."));
        m_pStatus = false;
    }

    while (m_pThreadFinished == false) {
        // wait for thread to finish
        qApp->processEvents();
    }

    emit finished(m_pStatus);
}

void DocbookGenerator::threadFinished()
{
    m_pThreadFinished = true;
    delete docbookGeneratorJob;
    docbookGeneratorJob = 0;
}


