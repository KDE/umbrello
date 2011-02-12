/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2008-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "docbookgeneratorjob.h"

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

#include <ktemporaryfile.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include <QtCore/QTextOStream>

extern int xmlLoadExtDtdDefaultValue;

DocbookGeneratorJob::DocbookGeneratorJob(QObject* parent):
        QThread(parent)
{
}

void DocbookGeneratorJob::run()
{
    UMLApp* app = UMLApp::app();
    UMLDoc* umlDoc = app->document();

    //write the XMI model in an in-memory char* string
    QString xmi;
    QTextOStream xmiStream(&xmi);

    KTemporaryFile file; // we need this tmp file if we are writing to a remote file
    file.setAutoRemove(false);

    // lets open the file for writing
    if (!file.open()) {
        uError() << "There was a problem saving file" << file.fileName();
        return;
    }

    umlDoc->saveToXMI(file); // save the xmi stuff to it

    xsltStylesheetPtr cur = NULL;
    xmlDocPtr doc, res;

    const char *params[16 + 1];
    int nbparams = 0;
    params[nbparams] = NULL;

    QString xsltFile(KGlobal::dirs()->findResource("appdata", "xmi2docbook.xsl"));

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    cur = xsltParseStylesheetFile((const xmlChar *)xsltFile.toLatin1().constData());
    doc = xmlParseFile((const char*)(file.fileName().toUtf8()));
    res = xsltApplyStylesheet(cur, doc, params);

    KTemporaryFile tmpDocBook;
    tmpDocBook.setAutoRemove(false);
    tmpDocBook.open();

    umlDoc->writeToStatusBar(i18n("Exporting to DocBook..."));
    xsltSaveResultToFd(tmpDocBook.handle(), res, cur);
    xsltFreeStylesheet(cur);
    xmlFreeDoc(res);
    xmlFreeDoc(doc);

    xsltCleanupGlobals();
    xmlCleanupParser();

    emit docbookGenerated(tmpDocBook.fileName());
}

#include "docbookgeneratorjob.moc"
