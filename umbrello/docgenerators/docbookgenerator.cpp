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

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include <kdebug.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>

#include "uml.h"
#include "umldoc.h"
#include "umlviewimageexportermodel.h"

extern int xmlLoadExtDtdDefaultValue;

DocbookGenerator::DocbookGenerator()
{
}

DocbookGenerator::~DocbookGenerator() {}


bool DocbookGenerator::generateDocbookForProject()
{
  UMLApp *app = UMLApp::app();
  UMLDoc* umlDoc = app->getDocument();
  KURL url = umlDoc->URL();
  QString fileName = url.fileName();
  fileName.replace(QRegExp(".xmi$"),"");
  url.setFileName(fileName);
  kDebug() << "Exporting to directory: " << url << endl;
  generateDocbookForProjectInto(url);
  return true;
}

KIO::Job* DocbookGenerator::generateDocbookForProjectInto(const KURL& destDir)
{
  UMLApp* app = UMLApp::app();
  UMLDoc* umlDoc = app->getDocument();

  // export all views
  umlDoc->writeToStatusBar(i18n("Exporting all views..."));
  QStringList errors = UMLViewImageExporterModel().exportAllViews(
      UMLViewImageExporterModel::mimeTypeToImageType("image/png"),
      destDir, false);
  if (!errors.empty())
  {
#if KDE_IS_VERSION(3,4,0)
    KMessageBox::errorList(app, i18n("Some errors happened when exporting the images:"), errors);
#else
    QString errorsCaption;
    for (QStringList::Iterator it = errors.begin(); it != errors.end(); ++it) {
        errorsCaption += "\n" + *it;
    }
    KMessageBox::error(app, i18n("Some errors happened when exporting the images:") + errorsCaption);
#endif
    return 0;
  }

  //write the XMI model in an in-memory char* string
  QString xmi;
  QTextOStream xmiStream(&xmi);

  KTempFile tmpfile; // we need this tmp file if we are writing to a remote file

  QFile file;
  file.setName( tmpfile.name() );

  // lets open the file for writing
  if( !file.open( IO_WriteOnly ) ) {
    KMessageBox::error(0, i18n("There was a problem saving file: %1").arg(tmpfile.name()), i18n("Save Error"));
    return false;
  }
  umlDoc->saveToXMI(file); // save the xmi stuff to it
  file.close();
  tmpfile.close();

  xsltStylesheetPtr cur = NULL;
  xmlDocPtr doc, res;

  const char *params[16 + 1];
  int nbparams = 0;
  params[nbparams] = NULL;

  QString xsltFile(KGlobal::dirs()->findResource("appdata","xmi2docbook.xsl"));

  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
  cur = xsltParseStylesheetFile((const xmlChar *)xsltFile.latin1());
  doc = xmlParseFile((const char*)(tmpfile.name().utf8()));
  res = xsltApplyStylesheet(cur, doc, params);

  KTempFile tmpDocBook;
  tmpDocBook.setAutoDelete(false);

  xsltSaveResultToFile(tmpDocBook.fstream(), res, cur);
  xsltFreeStylesheet(cur);
  xmlFreeDoc(res);
  xmlFreeDoc(doc);

  xsltCleanupGlobals();
  xmlCleanupParser();

  KURL url = umlDoc->URL();
  QString fileName = url.fileName();
  fileName.replace(QRegExp(".xmi$"),".docbook");
  url.setPath(destDir.path());
  url.addPath(fileName);
  kDebug() << "Copying result to: " << url << endl;
  KIO::Job* job = KIO::file_copy(tmpDocBook.file()->name(),url,-1,true,false,false);
  job->setAutoErrorHandlingEnabled(true);

  return job;
}


#include "docbookgenerator.moc"
