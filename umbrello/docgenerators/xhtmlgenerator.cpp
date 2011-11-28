/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006      Gael de Chalendar (aka Kleag) kleag@free.fr   *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "xhtmlgenerator.h"

#include "debug_utils.h"
#include "docbook2xhtmlgeneratorjob.h"
#include "uml.h"
#include "umldoc.h"
#include "umlviewimageexportermodel.h"
#include "docbookgenerator.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/job.h>

#include <QtGui/QApplication>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

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
    KUrl url = m_umlDoc->url();
    QString fileName = url.fileName();
    fileName.remove(QRegExp(".xmi$"));
    url.setFileName(fileName);
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
bool XhtmlGenerator::generateXhtmlForProjectInto(const KUrl& destDir)
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
    if ( !status ) {
        uDebug() << "Error in converting to docbook";
        m_pStatus = false;
        return;
    }
    else {
        KUrl url = m_umlDoc->url();
        QString fileName = url.fileName();
        fileName.replace(QRegExp(".xmi$"),".docbook");
        url.setPath(m_destDir.path());
        url.addPath(fileName);

        m_umlDoc->writeToStatusBar( i18n( "Generating XHTML..." ) );
        m_d2xg  = new Docbook2XhtmlGeneratorJob( url, this );
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
    KUrl url = m_umlDoc->url();
    QString fileName = url.fileName();
    fileName.replace(QRegExp(".xmi$"),".html");
    url.setPath(m_destDir.path());
    url.addPath(fileName);

    KIO::Job* htmlCopyJob = KIO::file_copy( KUrl::fromPath( tmpFileName ), url, -1, KIO::Overwrite | KIO::HideProgressInfo );
    if ( KIO::NetAccess::synchronousRun( htmlCopyJob, (QWidget*)UMLApp::app() ) ) {
        m_umlDoc->writeToStatusBar(i18n("XHTML Generation Complete..."));
    } else {
        m_pStatus = false;
        return;
    }

    m_umlDoc->writeToStatusBar(i18n("Copying CSS..."));

    QString cssFileName(KGlobal::dirs()->findResource("appdata","xmi.css"));
    KUrl cssUrl = m_destDir;
    cssUrl.addPath("xmi.css");
    KIO::Job* cssJob = KIO::file_copy(cssFileName,cssUrl,-1, KIO::Overwrite | KIO::HideProgressInfo );

    if ( KIO::NetAccess::synchronousRun( cssJob, (QWidget*)UMLApp::app() ) ) {
        m_umlDoc->writeToStatusBar(i18n("Finished Copying CSS..."));
        m_pStatus = true;
    } else {
        m_umlDoc->writeToStatusBar(i18n("Failed Copying CSS..."));
        m_pStatus = false;
    }

    while ( m_pThreadFinished == false ) {
        // wait for thread to finish
        qApp->processEvents();
    }

    emit finished( m_pStatus );
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

#include "xhtmlgenerator.moc"
