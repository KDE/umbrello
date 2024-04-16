/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2006 Gael de Chalendar (aka Kleag) kleag@free.fr
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "docbookgenerator.h"

#define DBG_SRC QStringLiteral("DocbookGenerator")
#include "debug_utils.h"
#include "docbookgeneratorjob.h"
#include "optionstate.h"
#include "uml.h"
#include "umldoc.h"
#include "umlviewimageexportermodel.h"

#include <kjobwidgets.h>
#include <KLocalizedString>
#include <KMessageBox>
#include <kio/job.h>

#include <QApplication>
#include <QFile>
#include <QRegExp>
#include <QTextStream>

DEBUG_REGISTER(DocbookGenerator)

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
    QUrl url = umlDoc->url();
  QString fileName = url.fileName();
  fileName.remove(QRegExp(QStringLiteral(".xmi$")));
  url.setPath(url.path() + QLatin1Char('/') + fileName);
  logDebug1("DocbookGenerator::generateDocbookForProject: Exporting to directory %1", url.path());
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
void DocbookGenerator::generateDocbookForProjectInto(const QUrl& destDir)
{
    m_destDir = destDir;
    umlDoc->writeToStatusBar(i18n("Exporting all views..."));

    UMLViewList views = UMLApp::app()->document()->viewIterator();
    QStringList errors = UMLViewImageExporterModel().exportViews(views,
        UMLViewImageExporterModel::mimeTypeToImageType(QStringLiteral("image/png")), destDir, false);
    if (!errors.empty()) {
        KMessageBox::errorList(UMLApp::app(), i18n("Some errors happened when exporting the images:"), errors);
        return;
    }

    umlDoc->writeToStatusBar(i18n("Generating Docbook..."));

    docbookGeneratorJob = new DocbookGeneratorJob(this);
    connect(docbookGeneratorJob, SIGNAL(docbookGenerated(QString)), this, SLOT(slotDocbookGenerationFinished(QString)));
    connect(docbookGeneratorJob, SIGNAL(finished()), this, SLOT(threadFinished()));
    logDebug0("DocbookGenerator::generateDocbookForProjectInto: Threading.");
    docbookGeneratorJob->start();
}

void DocbookGenerator::slotDocbookGenerationFinished(const QString& tmpFileName)
{
    logDebug1("DocbookGenerator: Generation finished (%1)", tmpFileName);
    QUrl url = umlDoc->url();
    QString fileName = url.fileName();
    fileName.replace(QRegExp(QStringLiteral(".xmi$")), QStringLiteral(".docbook"));
    url.setPath(m_destDir.path() + QLatin1Char('/') + fileName);
    KIO::Job* job = KIO::file_copy(QUrl::fromLocalFile(tmpFileName), url, -1, KIO::Overwrite | KIO::HideProgressInfo);
    KJobWidgets::setWindow(job, (QWidget*)UMLApp::app());
    job->exec();
    if (!job->error()) {
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

    Q_EMIT finished(m_pStatus);
}

void DocbookGenerator::threadFinished()
{
    m_pThreadFinished = true;
    delete docbookGeneratorJob;
    docbookGeneratorJob = 0;
}

/**
 * return custom xsl file for generating docbook
 *
 * @return filename with path
 */
QString DocbookGenerator::customXslFile()
{
    QString xslBaseName;
    if (Settings::optionState().generalState.uml2) {
        xslBaseName = QStringLiteral("xmi2docbook.xsl");
    } else {
        xslBaseName = QStringLiteral("xmi1docbook.xsl");
    }
    QString xsltFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("umbrello5/") + xslBaseName));
    if (xsltFile.isEmpty())
        xsltFile = QStringLiteral(DOCGENERATORS_DIR) + QLatin1Char('/') + xslBaseName;

    logDebug1("DocbookGenerator::customXslFile returning %1", xsltFile);
    return xsltFile;
}

