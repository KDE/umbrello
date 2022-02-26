/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2007-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "docbook2xhtmlgeneratorjob.h"

#include "debug_utils.h"
#include "file_utils.h"
#include "uml.h"
#include "umldoc.h"
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

#include <QTextStream>

DEBUG_REGISTER(Docbook2XhtmlGeneratorJob)

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
  xsltStylesheetPtr cur = 0;
  xmlDocPtr doc, res;

  const char *params[16 + 1];
  int nbparams = 0;
  params[nbparams] = 0;

  umlDoc->writeToStatusBar(i18n("Exporting to XHTML..."));
  QString xsltFileName = XhtmlGenerator::customXslFile();

  // use public xml catalogs
  xmlLoadCatalogs(File_Utils::xmlCatalogFilePath().toLocal8Bit().constData());

  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
  logDebug1("Docbook2XhtmlGeneratorJob::run: Parsing stylesheet %1", xsltFileName);
  cur = xsltParseStylesheetFile((const xmlChar *)xsltFileName.toLatin1().constData());
  logDebug1("Docbook2XhtmlGeneratorJob::run: Parsing file %1", m_docbookUrl.path());
  doc = xmlParseFile((const char*)(m_docbookUrl.path().toUtf8()));
  logDebug0("Docbook2XhtmlGeneratorJob::run: Applying stylesheet");
  res = xsltApplyStylesheet(cur, doc, params);

#if QT_VERSION >= 0x050000
  QTemporaryFile tmpXhtml;
#else
  KTemporaryFile tmpXhtml;
#endif
  tmpXhtml.setAutoRemove(false);
  tmpXhtml.open();

  logDebug1("Docbook2XhtmlGeneratorJob::run: Writing HTML result to temp file: %1",
            tmpXhtml.fileName());
  xsltSaveResultToFd(tmpXhtml.handle(), res, cur);
  tmpXhtml.close();

  xsltFreeStylesheet(cur);
  xmlFreeDoc(res);
  xmlFreeDoc(doc);

  xsltCleanupGlobals();
  xmlCleanupParser();

  emit xhtmlGenerated(tmpXhtml.fileName());
}



