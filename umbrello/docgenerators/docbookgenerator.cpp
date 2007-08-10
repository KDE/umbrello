/***************************************************************************
 *                        docbookgenerator.cpp  -  description             *
 *                           -------------------                           *
 *  copyright            : (C) 2006 by Gael de Chalendar (aka Kleag)       *
 *    (C) 2006 Umbrello UML Modeller Authors <uml-devel@uml.sf.net>        *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "docbookgenerator.h"

#include <kdebug.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/job.h>

#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>

#include "docbookgeneratorjob.h"

#include "uml.h"
#include "umldoc.h"
#include "umlviewimageexportermodel.h"

DocbookGenerator::DocbookGenerator()
{
  umlDoc = UMLApp::app()->getDocument();
  m_pStatus = true;
}

DocbookGenerator::~DocbookGenerator() {}


bool DocbookGenerator::generateDocbookForProject()
{
  KUrl url = umlDoc->url();
  QString fileName = url.fileName();
  fileName.replace(QRegExp(".xmi$"),"");
  url.setFileName(fileName);
  kDebug() <<k_funcinfo<<"Exporting to directory: " << url;
  generateDocbookForProjectInto(url);
  return true;
}

void DocbookGenerator::generateDocbookForProjectInto(const KUrl& destDir)
{
    m_destDir = destDir;
    umlDoc->writeToStatusBar(i18n("Exporting all views..."));
    QStringList errors = UMLViewImageExporterModel().exportAllViews(
        UMLViewImageExporterModel::mimeTypeToImageType("image/png"), destDir, false);
    if (!errors.empty()) {
        KMessageBox::errorList(UMLApp::app(), i18n("Some errors happened when exporting the images:"), errors);
        return;
    }

    umlDoc->writeToStatusBar(i18n("Generating Docbook..."));
    DocbookGeneratorJob* docbookGeneratorJob = new DocbookGeneratorJob( this );
    connect( docbookGeneratorJob , SIGNAL(docbookGenerated(const QString&)), this, SLOT(slotDocbookGenerationFinished(const QString&)));
    kDebug()<<k_funcinfo<<"Threading";
    docbookGeneratorJob->start();
}

void DocbookGenerator::slotDocbookGenerationFinished(const QString& tmpFileName)
{
    kDebug()<<"Generation Finished"<<tmpFileName;
    KUrl url = umlDoc->url();
    QString fileName = url.fileName();
    fileName.replace(QRegExp(".xmi$"),".docbook");
    url.setPath(m_destDir.path());
    url.addPath(fileName);

    KIO::Job* job = KIO::file_copy(KUrl::fromPath(tmpFileName),url,-1, true, true, false);
    if ( KIO::NetAccess::synchronousRun( job, (QWidget*)UMLApp::app() ) ) {
        umlDoc->writeToStatusBar(i18n("Docbook Generation Complete..."));
        m_pStatus = true;
    } else {
        umlDoc->writeToStatusBar(i18n("Docbook Generation Failed..."));
        m_pStatus = false;
    }

    emit finished(m_pStatus);
}

#include "docbookgenerator.moc"
