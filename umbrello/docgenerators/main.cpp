/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include <unistd.h>

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

// kde includes
#include <kaboutdata.h>
#include <QCommandLineParser>
#include <kconfig.h>
#include <KLocalizedString>
#include <KF5/KConfigWidgets/ktip.h>
#include <kwin.h>

#include "version.h"

extern int xmlLoadExtDtdDefaultValue;

static const char description[] =
    I18N_NOOP("Umbrello UML Modeller autonomous code generator");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


int main(int argc, char *argv[])
{
  xsltStylesheetPtr cur = nullptr;
  xmlDocPtr doc, res;

  const char *params[16 + 1];
  int nbparams = 0;
  params[nbparams] = nullptr;

  QCoreApplication app(argc, argv);

  KAboutData aboutData(QStringLiteral("umbodoc"), ki18n("Umbrello UML Modeller autonomous code generator").toString(),
                        QLatin1String(umbrelloVersion()), ki18n(description).toString(), KAboutLicense::LicenseKey::GPL,
                        ki18n("(c) 2006 Gael de Chalendar (aka Kleag), (c) 2002-2006 Umbrello UML Modeller Authors").toString(), KLocalizedString().toString(),
                        QStringLiteral("https://apps.kde.org/umbrello"));
  aboutData.addAuthor(ki18n("Gael de Chalendar (aka Kleag)").toString(), KLocalizedString().toString(), QStringLiteral("kleag@free.fr"));
  aboutData.addAuthor(ki18n("Umbrello UML Modeller Authors").toString(), KLocalizedString().toString(), QStringLiteral("umbrello-devel@kde.org"));

  // KCmdLineArgs::init(argc, argv, &aboutData);
  QCommandLineParser parser;
  parser.addPositionalArgument(QStringLiteral("xslt"), QCoreApplication::translate("main", "The XSLT file to use."));
  
  // KCmdLineOptions options;
  // options.add("+[File]", ki18n("File to transform"));
  // options.add("xslt <url>", ki18n("The XSLT file to use"));
  // KCmdLineArgs::addCmdLineOptions(options); // Add our own options.

  // KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  parser.process(app);

  const QStringList args = parser.positionalArguments();
  
  
  if (parser.isSet(QStringLiteral("xslt")))
  {
    QString xsltFile = parser.value(QStringLiteral("xslt"));

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    cur = xsltParseStylesheetFile((const xmlChar *)xsltFile.toStdString().c_str());
    doc = xmlParseFile(args.first().toStdString().c_str());
    res = xsltApplyStylesheet(cur, doc, params);
    xsltSaveResultToFile(stdout, res, cur);

    xsltFreeStylesheet(cur);
    xmlFreeDoc(res);
    xmlFreeDoc(doc);

    xsltCleanupGlobals();
    xmlCleanupParser();
  }
  return(0);
}
