/***************************************************************************
 *                        xhtmlgenerator.cpp  -  description             *
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

#include "xhtmlgenerator.h"

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
#include <kio/jobuidelegate.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>

#include "uml.h"
#include "umldoc.h"
#include "umlviewimageexportermodel.h"
#include "docbookgenerator.h"

extern int xmlLoadExtDtdDefaultValue;

XhtmlGenerator::XhtmlGenerator()
{
}

XhtmlGenerator::~XhtmlGenerator() {}


bool XhtmlGenerator::generateXhtmlForProject()
{
  UMLApp *app = UMLApp::app();
  UMLDoc* umlDoc = app->getDocument();
  KUrl url = umlDoc->url();
  QString fileName = url.fileName();
  fileName.replace(QRegExp(".xmi$"),"");
  url.setFileName(fileName);
  kDebug() << "Exporting to directory: " << url << endl;
  return generateXhtmlForProjectInto(url);
}

bool XhtmlGenerator::generateXhtmlForProjectInto(const KUrl& destDir)
{
  kDebug() << "First convert to docbook" << endl;
  m_destDir = destDir;
//   KUrl url(QString("file://")+m_tmpDir.name());
  KIO::Job* docbookJob = DocbookGenerator().generateDocbookForProjectInto(destDir);
  if (docbookJob == 0)
  {
    return false;
  }
  kDebug() << "Connecting..." << endl;
  connect(docbookJob, SIGNAL(result(KJob*)), this, SLOT(slotDocbookToXhtml(KJob*)));
  return true;
}

void XhtmlGenerator::slotDocbookToXhtml(KJob * docbookJob)
{
  kDebug() << "Now convert docbook to html..." << endl;
  if ( docbookJob->error() )
  {
    // error shown by setAutoErrorHandlingEnabled(true) already
    return;
  }

  UMLApp* app = UMLApp::app();
  UMLDoc* umlDoc = app->getDocument();

  const KUrl& url = umlDoc->url();
  QString docbookName = url.fileName();
  docbookName.replace(QRegExp(".xmi$"),".docbook");
  KUrl docbookUrl = m_destDir;
  docbookUrl.addPath(docbookName);

  xsltStylesheetPtr cur = NULL;
  xmlDocPtr doc, res;

  const char *params[16 + 1];
  int nbparams = 0;
  params[nbparams] = NULL;

  QString xsltFileName(KGlobal::dirs()->findResource("appdata","docbook2xhtml.xsl"));
  kDebug() << "XSLT file is'"<<xsltFileName<<"'" << endl;
  QFile xsltFile(xsltFileName);
  xsltFile.open(QIODevice::ReadOnly);
  QString xslt = xsltFile.readAll();
  kDebug() << "XSLT is'"<<xslt<<"'" << endl;
  xsltFile.close();

  QString localXsl = KGlobal::dirs()->findResource("data","ksgmltools2/docbook/xsl/html/docbook.xsl");
  kDebug() << "Local xsl is'"<<localXsl<<"'" << endl;
  if (!localXsl.isEmpty())
  {
    localXsl = QString("href=\"file://") + localXsl + "\"";
    xslt.replace(QRegExp("href=\"http://[^\"]*\""),localXsl);
  }
  KTemporaryFile tmpXsl;
  tmpXsl.setAutoRemove(false);
  tmpXsl.open();
  QTextStream str ( &tmpXsl );
  str << xslt;
  str.flush();

  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
  kDebug() << "Parsing stylesheet " << tmpXsl.fileName() << endl;
  cur = xsltParseStylesheetFile((const xmlChar *)tmpXsl.fileName().latin1());
  kDebug() << "Parsing file " << docbookUrl.path() << endl;
  doc = xmlParseFile((const char*)(docbookUrl.path().utf8()));
  kDebug() << "Applying stylesheet " << endl;
  res = xsltApplyStylesheet(cur, doc, params);

  KTemporaryFile tmpXhtml;
  tmpXhtml.setAutoRemove(false);
  tmpXhtml.open();

  kDebug() << "Writing HTML result to temp file: " << tmpXhtml.fileName() << endl;
  xsltSaveResultToFd(tmpXhtml.handle(), res, cur);

  xsltFreeStylesheet(cur);
  xmlFreeDoc(res);
  xmlFreeDoc(doc);

  xsltCleanupGlobals();
  xmlCleanupParser();

  QString xhtmlName = url.fileName();
  xhtmlName.replace(QRegExp(".xmi$"),".html");
  KUrl xhtmlUrl = m_destDir;
  xhtmlUrl.addPath(xhtmlName);

  kDebug() << "Copying HTML result to: " << xhtmlUrl << endl;
  KIO::Job* job = KIO::file_copy(tmpXhtml.fileName(),xhtmlUrl,-1,true,false,false);
  job->ui()->setAutoErrorHandlingEnabled(true);
  connect (job, SIGNAL(result( KJob* )), this, SLOT(slotHtmlCopyFinished( KJob* )));

  QString cssFileName(KGlobal::dirs()->findResource("appdata","xmi.css"));
  kDebug() << "CSS file is'"<<cssFileName<<"'" << endl;
  KUrl cssUrl = m_destDir;
  cssUrl.addPath("xmi.css");
  KIO::Job* cssJob = KIO::file_copy(cssFileName,cssUrl,-1,true,false,false);
  cssJob->ui()->setAutoErrorHandlingEnabled(true);
}

void XhtmlGenerator::slotHtmlCopyFinished( KJob* )
{
  kDebug() << "HTML copy finished: emiting finished" << endl;
  emit(finished());
}

#include "xhtmlgenerator.moc"
