/***************************************************************************
 *                        xhtmlgenerator.cpp  -  description               *
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
  KURL url = umlDoc->URL();
  QString fileName = url.fileName();
  fileName.replace(QRegExp(".xmi$"),"");
  url.setFileName(fileName);
  kDebug() << "Exporting to directory: " << url << endl;
  return generateXhtmlForProjectInto(url);
}

bool XhtmlGenerator::generateXhtmlForProjectInto(const KURL& destDir)
{
  kDebug() << "First convert to docbook" << endl;
  m_destDir = destDir;
//   KURL url(QString("file://")+m_tmpDir.name());
  KIO::Job* docbookJob = DocbookGenerator().generateDocbookForProjectInto(destDir);
  if (docbookJob == 0)
  {
    return false;
  }
  kDebug() << "Connecting..." << endl;
  connect(docbookJob, SIGNAL(result( KIO::Job * )), this, SLOT(slotDocbookToXhtml( KIO::Job *)));
  return true;
}

void XhtmlGenerator::slotDocbookToXhtml(KIO::Job * docbookJob)
{
  kDebug() << "Now convert docbook to html..." << endl;
  if ( docbookJob->error() )
  {
    docbookJob->showErrorDialog( 0L  );
    return;
  }

  UMLApp* app = UMLApp::app();
  UMLDoc* umlDoc = app->getDocument();

  const KURL& url = umlDoc->URL();
  QString docbookName = url.fileName();
  docbookName.replace(QRegExp(".xmi$"),".docbook");
//   KURL docbookUrl(QString("file://")+m_tmpDir.name());
  KURL docbookUrl = m_destDir;
  docbookUrl.addPath(docbookName);

  xsltStylesheetPtr cur = NULL;
  xmlDocPtr doc, res;

  const char *params[16 + 1];
  int nbparams = 0;
  params[nbparams] = NULL;

  QString xsltFileName(KGlobal::dirs()->findResource("appdata","docbook2xhtml.xsl"));
  kDebug() << "XSLT file is'"<<xsltFileName<<"'" << endl;
  QFile xsltFile(xsltFileName);
  xsltFile.open(IO_ReadOnly);
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
  KTempFile tmpXsl;
  *tmpXsl.textStream() << xslt;
  tmpXsl.file()->close();


  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
  kDebug() << "Parsing stylesheet " << tmpXsl.name() << endl;
  cur = xsltParseStylesheetFile((const xmlChar *)tmpXsl.name().latin1());
  kDebug() << "Parsing file " << docbookUrl.path() << endl;
  doc = xmlParseFile((const char*)(docbookUrl.path().utf8()));
  kDebug() << "Applying stylesheet " << endl;
  res = xsltApplyStylesheet(cur, doc, params);

  KTempFile tmpXhtml;
  tmpXhtml.setAutoDelete(false);

  kDebug() << "Writing HTML result to temp file: " << tmpXhtml.file()->name() << endl;
  xsltSaveResultToFile(tmpXhtml.fstream(), res, cur);

  xsltFreeStylesheet(cur);
  xmlFreeDoc(res);
  xmlFreeDoc(doc);

  xsltCleanupGlobals();
  xmlCleanupParser();

  QString xhtmlName = url.fileName();
  xhtmlName.replace(QRegExp(".xmi$"),".html");
  KURL xhtmlUrl = m_destDir;
  xhtmlUrl.addPath(xhtmlName);

  kDebug() << "Copying HTML result to: " << xhtmlUrl << endl;
  KIO::Job* job = KIO::file_copy(tmpXhtml.file()->name(),xhtmlUrl,-1,true,false,false);
  job->setAutoErrorHandlingEnabled(true);
  connect (job, SIGNAL(result( KIO::Job* )), this, SLOT(slotHtmlCopyFinished( KIO::Job* )));

  QString cssFileName(KGlobal::dirs()->findResource("appdata","xmi.css"));
  kDebug() << "CSS file is'"<<cssFileName<<"'" << endl;
  KURL cssUrl = m_destDir;
  cssUrl.addPath("xmi.css");
  KIO::Job* cssJob = KIO::file_copy(cssFileName,cssUrl,-1,true,false,false);
  cssJob->setAutoErrorHandlingEnabled(true);
}

void XhtmlGenerator::slotHtmlCopyFinished( KIO::Job* )
{
  kDebug() << "HTML copy finished: emiting finished" << endl;
  emit(finished());
}

#include "xhtmlgenerator.moc"
