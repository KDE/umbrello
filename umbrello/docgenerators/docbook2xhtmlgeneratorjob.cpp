/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "docbook2xhtmlgeneratorjob.h"

#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"

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

// kde includes
#include <KLocalizedString>
#if QT_VERSION < 0x050000
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <KUrl>
#endif

// qt includes
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QUrl>
#endif

#include <QTextOStream>

extern int xmlLoadExtDtdDefaultValue;

/**
 * Constructor
 * @param docBookUrl The Url of the Docbook that is to be converted to XHtml
 * @param parent     Parent object for QThread constructor
 */

#if QT_VERSION >= 0x050000
Docbook2XhtmlGeneratorJob::Docbook2XhtmlGeneratorJob(QUrl& docBookUrl, QObject* parent)
    :QThread(parent), m_docbookUrl(docBookUrl)
#else
Docbook2XhtmlGeneratorJob::Docbook2XhtmlGeneratorJob(KUrl& docBookUrl, QObject* parent)
    :QThread(parent), m_docbookUrl(docBookUrl)
#endif
{
}

void Docbook2XhtmlGeneratorJob::run()
{
  UMLDoc* umlDoc = UMLApp::app()->document();
  xsltStylesheetPtr cur = NULL;
  xmlDocPtr doc, res;

  const char *params[16 + 1];
  int nbparams = 0;
  params[nbparams] = NULL;

  umlDoc->writeToStatusBar(i18n("Exporting to XHTML..."));

#if QT_VERSION >= 0x050000
  QString xsltFileName(QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("docbook2xhtml.xsl")));
#else
  QString xsltFileName(KGlobal::dirs()->findResource("appdata", QLatin1String("docbook2xhtml.xsl")));
#endif
  uDebug() << "XSLT file is'" << xsltFileName << "'";
  QFile xsltFile(xsltFileName);
  xsltFile.open(QIODevice::ReadOnly);
  QString xslt = QString::fromLatin1(xsltFile.readAll());
  uDebug() << "XSLT is'" << xslt << "'";
  xsltFile.close();

#if QT_VERSION >= 0x050000
  QString localXsl = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("ksgmltools2/docbook/xsl/html/docbook.xsl"));
#else
  QString localXsl = KGlobal::dirs()->findResource("data", QLatin1String("ksgmltools2/docbook/xsl/html/docbook.xsl"));
#endif
  uDebug() << "Local xsl is'" << localXsl << "'";
  if (!localXsl.isEmpty())
  {
    localXsl = QLatin1String("href=\"file://") + localXsl + QLatin1String("\"");
    xslt.replace(QRegExp(QLatin1String("href=\"http://[^\"]*\"")), localXsl);
  }
#if QT_VERSION >= 0x050000
  QTemporaryFile tmpXsl;
#else
  KTemporaryFile tmpXsl;
#endif
  tmpXsl.setAutoRemove(false);
  tmpXsl.open();
  QTextStream str (&tmpXsl);
  str << xslt;
  str.flush();

  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
  uDebug() << "Parsing stylesheet " << tmpXsl.fileName();
  cur = xsltParseStylesheetFile((const xmlChar *)tmpXsl.fileName().toLatin1().constData());
  uDebug() << "Parsing file " << m_docbookUrl.path();
  doc = xmlParseFile((const char*)(m_docbookUrl.path().toUtf8()));
  uDebug() << "Applying stylesheet ";
  res = xsltApplyStylesheet(cur, doc, params);

#if QT_VERSION >= 0x050000
  QTemporaryFile tmpXhtml;
#else
  KTemporaryFile tmpXhtml;
#endif
  tmpXhtml.setAutoRemove(false);
  tmpXhtml.open();

  uDebug() << "Writing HTML result to temp file: " << tmpXhtml.fileName();
  xsltSaveResultToFd(tmpXhtml.handle(), res, cur);

  xsltFreeStylesheet(cur);
  xmlFreeDoc(res);
  xmlFreeDoc(doc);

  xsltCleanupGlobals();
  xmlCleanupParser();

  emit xhtmlGenerated(tmpXhtml.fileName());
}



