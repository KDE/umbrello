/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include <ktip.h>
#include <kdebug.h>
#include <kwin.h>

#include "version.h"

extern int xmlLoadExtDtdDefaultValue;

static const char description[] =
    I18N_NOOP("Umbrello UML Modeller autonomous code generator");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("File to transform"), 0 },
  { "xslt <url>", I18N_NOOP("The XSLT file to use"), 0},
        // INSERT YOUR COMMANDLINE OPTIONS HERE
  KCmdLineLastOption
};

int main(int argc, char *argv[])
{
  xsltStylesheetPtr cur = NULL;
  xmlDocPtr doc, res;

  const char *params[16 + 1];
  int nbparams = 0;
  params[nbparams] = NULL;

  KAboutData aboutData( "umbodoc", I18N_NOOP("Umbrello UML Modeller autonomous code generator"),
                        UMBRELLO_VERSION, description, KAboutData::License_GPL,
                        I18N_NOOP("(c) 2006 Gael de Chalendar (aka Kleag), (c) 2002-2006 Umbrello UML Modeller Authors"), 0,
                        "http://uml.sf.net/");
  aboutData.addAuthor("Gael de Chalendar (aka Kleag)",0, "kleag@free.fr");
  aboutData.addAuthor(I18N_NOOP("Umbrello UML Modeller Authors"), 0, "uml-devel@lists.sourceforge.net");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QCStringList xsltOpt = args->getOptionList("xslt");
  if (xsltOpt.size() > 0)
  {
    QString xsltFile(xsltOpt.last());

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    cur = xsltParseStylesheetFile((const xmlChar *)xsltFile.latin1());
    doc = xmlParseFile(args->url( 0 ).url().latin1());
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

