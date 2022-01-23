/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2008-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "docbookgeneratorjob.h"

#include "docbookgenerator.h"
#include "debug_utils.h"
#include "file_utils.h"
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
#if QT_VERSION < 0x050000
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#endif
#include <KLocalizedString>

// qt includes
#include <QTextStream>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#include <QTemporaryFile>
#endif

extern int xmlLoadExtDtdDefaultValue;

#define MAX_PATHS 64
static xmlExternalEntityLoader defaultEntityLoader = NULL;
static xmlChar *paths[MAX_PATHS + 1];
static int nbpaths = 0;
static QHash<QString,QString> replaceURLList;

/*
* Entity loading control and customization.
* taken from kdelibs/kdoctools/xslt.cpp
*/
static xmlParserInputPtr xsltprocExternalEntityLoader(const char *_URL, const char *ID,xmlParserCtxtPtr ctxt)
{
    xmlParserInputPtr ret;
    warningSAXFunc warning = NULL;

    // use local available dtd versions instead of fetching it every time from the internet
    QString url = QLatin1String(_URL);
    QHash<QString, QString>::const_iterator i;
    for(i = replaceURLList.constBegin(); i != replaceURLList.constEnd(); i++)
    {
        if (url.startsWith(i.key()))
        {
            url.replace(i.key(),i.value());
            qDebug() << "converted" << _URL << "to" << url;
        }
    }

    QByteArray URL = url.toLatin1();

    if ((ctxt != NULL) && (ctxt->sax != NULL)) {
        warning = ctxt->sax->warning;
        ctxt->sax->warning = NULL;
    }

    if (defaultEntityLoader != NULL) {
        ret = defaultEntityLoader(URL.constData(), ID, ctxt);
        if (ret != NULL) {
            if (warning != NULL)
                ctxt->sax->warning = warning;
            qDebug() << "Loaded URL=\"" << URL << "\" ID=\"" << ID << "\"";
            return(ret);
        }
    }

    int j = URL.lastIndexOf("/");
    const char *lastsegment = j > -1 ? URL.constData()+j+1 : URL.constData();

    for (int i = 0;i < nbpaths;i++) {
        xmlChar *newURL;

        newURL = xmlStrdup((const xmlChar *) paths[i]);
        newURL = xmlStrcat(newURL, (const xmlChar *) "/");
        newURL = xmlStrcat(newURL, (const xmlChar *) lastsegment);
        if (newURL != NULL) {
            if (defaultEntityLoader != NULL) {
                ret = defaultEntityLoader((const char *)newURL, ID, ctxt);
                if (ret != NULL) {
                    if (warning != NULL)
                        ctxt->sax->warning = warning;
                    qDebug() << "Loaded URL=\"" << newURL << "\" ID=\"" << ID << "\"";
                    xmlFree(newURL);
                    return(ret);
                }
            }
            xmlFree(newURL);
        }
    }
    if (warning != NULL) {
        ctxt->sax->warning = warning;
        if (_URL != NULL)
            warning(ctxt, "failed to load external entity \"%s\"\n", _URL);
        else if (ID != NULL)
            warning(ctxt, "failed to load external entity \"%s\"\n", ID);
    }
    return(NULL);
}

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
    QTextStream xmiStream(&xmi, QIODevice::WriteOnly);

#if QT_VERSION >= 0x050000
    QTemporaryFile file; // we need this tmp file if we are writing to a remote file
#else
    KTemporaryFile file; // we need this tmp file if we are writing to a remote file
#endif
    file.setAutoRemove(false);

    // lets open the file for writing
    if (!file.open()) {
        logError1("DocbookGeneratorJob::run: There was a problem saving file %1",
                  file.fileName());
        return;
    }

    umlDoc->saveToXMI(file); // save the xmi stuff to it

    xsltStylesheetPtr cur = 0;
    xmlDocPtr doc, res;

    const char *params[16 + 1];
    int nbparams = 0;
    params[nbparams] = 0;

    // use public xml catalogs
    xmlLoadCatalogs(File_Utils::xmlCatalogFilePath().toLocal8Bit().constData());

    QString xsltFile = DocbookGenerator::customXslFile();

    if (!defaultEntityLoader) {
        defaultEntityLoader = xmlGetExternalEntityLoader();
        xmlSetExternalEntityLoader(xsltprocExternalEntityLoader);
        QFileInfo xsltFilePath(xsltFile);

        // Note: This would not be required if the dtd would be registered in global xml catalog
        replaceURLList[QLatin1String("http://www.oasis-open.org/docbook/xml/simple/4.1.2.5/sdocbook.dtd")] = QString(QLatin1String("file:///%1/simple4125/sdocbook.dtd")).arg(xsltFilePath.absolutePath());
    }

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    cur = xsltParseStylesheetFile((const xmlChar *)xsltFile.toLatin1().constData());
    doc = xmlParseFile((const char*)(file.fileName().toUtf8()));
    res = xsltApplyStylesheet(cur, doc, params);

#if QT_VERSION >= 0x050000
    QTemporaryFile tmpDocBook;
#else
    KTemporaryFile tmpDocBook;
#endif
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

