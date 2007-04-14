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
#include <ktemporaryfile.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
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
  KUrl url = umlDoc->url();
  QString fileName = url.fileName();
  fileName.replace(QRegExp(".xmi$"),"");
  url.setFileName(fileName);
  kDebug() << "Exporting to directory: " << url << endl;
  generateDocbookForProjectInto(url);
  return true;
}

KIO::Job* DocbookGenerator::generateDocbookForProjectInto(const KUrl& destDir)
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
    KMessageBox::errorList(app, i18n("Some errors happened when exporting the images:"), errors);
    return 0;
  }

  //write the XMI model in an in-memory char* string
  QString xmi;
  QTextOStream xmiStream(&xmi);

  KTemporaryFile file; // we need this tmp file if we are writing to a remote file
  file.setAutoRemove(false);

  // lets open the file for writing
  if( !file.open() ) {
    KMessageBox::error(0, i18n("There was a problem saving file: %1", file.fileName()), i18n("Save Error"));
    return 0;
  }
  umlDoc->saveToXMI(file); // save the xmi stuff to it

  xsltStylesheetPtr cur = NULL;
  xmlDocPtr doc, res;

  const char *params[16 + 1];
  int nbparams = 0;
  params[nbparams] = NULL;

  QString xsltFile(KGlobal::dirs()->findResource("appdata","xmi2docbook.xsl"));

  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
  cur = xsltParseStylesheetFile((const xmlChar *)xsltFile.latin1());
  doc = xmlParseFile((const char*)(file.name().utf8()));
  res = xsltApplyStylesheet(cur, doc, params);

  KTemporaryFile tmpDocBook;
  tmpDocBook.setAutoRemove(false);
  tmpDocBook.open();

  xsltSaveResultToFd(tmpDocBook.handle(), res, cur);
  xsltFreeStylesheet(cur);
  xmlFreeDoc(res);
  xmlFreeDoc(doc);

  xsltCleanupGlobals();
  xmlCleanupParser();

  KUrl url = umlDoc->url();
  QString fileName = url.fileName();
  fileName.replace(QRegExp(".xmi$"),".docbook");
  url.setPath(destDir.path());
  url.addPath(fileName);
  kDebug() << "Copying result to: " << url << endl;
  KIO::Job* job = KIO::file_copy(KUrl::fromPath(tmpDocBook.fileName()),url,true);
  job->ui()->setAutoErrorHandlingEnabled(true);

  return job;
}


#include "docbookgenerator.moc"
