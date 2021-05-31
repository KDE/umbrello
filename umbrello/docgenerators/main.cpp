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
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <KLocalizedString>
#include <ktip.h>
#include <kwin.h>

#include "version.h"

extern int xmlLoadExtDtdDefaultValue;

static const char description[] =
    I18N_NOOP("Umbrello UML Modeller autonomous code generator");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


int main(int argc, char *argv[])
{
  xsltStylesheetPtr cur = 0;
  xmlDocPtr doc, res;

  const char *params[16 + 1];
  int nbparams = 0;
  params[nbparams] = 0;

  KAboutData aboutData("umbodoc", 0, ki18n("Umbrello UML Modeller autonomous code generator"),
                        umbrelloVersion(), ki18n(description), KAboutData::License_GPL,
                        ki18n("(c) 2006 Gael de Chalendar (aka Kleag), (c) 2002-2006 Umbrello UML Modeller Authors"), KLocalizedString(),
                        "http://umbrello.kde.org/");
  aboutData.addAuthor(ki18n("Gael de Chalendar (aka Kleag)"),KLocalizedString(), "kleag@free.fr");
  aboutData.addAuthor(ki18n("Umbrello UML Modeller Authors"), KLocalizedString(), "umbrello-devel@kde.org");
  KCmdLineArgs::init(argc, argv, &aboutData);

  KCmdLineOptions options;
  options.add("+[File]", ki18n("File to transform"));
  options.add("xslt <url>", ki18n("The XSLT file to use"));
  KCmdLineArgs::addCmdLineOptions(options); // Add our own options.

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QCStringList xsltOpt = args->getOptionList("xslt");
  if (xsltOpt.size() > 0)
  {
    QString xsltFile(xsltOpt.last());

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    cur = xsltParseStylesheetFile((const xmlChar *)xsltFile.latin1());
    doc = xmlParseFile(args->url(0).url().latin1());
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
