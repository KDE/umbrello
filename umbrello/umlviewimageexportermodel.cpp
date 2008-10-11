/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlviewimageexportermodel.h"

// system includes
#include <math.h>

// include files for Qt
#include <QtGui/QApplication>
#include <QtGui/QImageWriter>
#include <QtCore/QRect>
#include <QtGui/QImage>
#include <QtGui/QPicture>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtSvg/QSvgGenerator>

// kde include files
#include <kdebug.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kio/netaccess.h>

// application specific includes
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umllistview.h"
#include "umllistviewitem.h"

static QStringList supportedImageTypesList;
static QStringList supportedMimeTypesList;

QStringList UMLViewImageExporterModel::supportedImageTypes()
{
    if (!supportedImageTypesList.size()) {
        // specific supported formats
        supportedImageTypesList << "eps";
        supportedImageTypesList << "svg";

        // QT supported formats
        QList<QByteArray> qImageFormats = QImageWriter::supportedImageFormats();
        QList<QByteArray>::const_iterator it, it_end;
        it = qImageFormats.begin(); it_end = qImageFormats.end();
        for (; it != it_end; ++it) {
            supportedImageTypesList << QString(*it).toLower();
        }
    }

    return supportedImageTypesList;
}

QStringList UMLViewImageExporterModel::supportedMimeTypes()
{
    if (!supportedMimeTypesList.size()) {
        const QStringList imageTypes = UMLViewImageExporterModel::supportedImageTypes();
        for (QStringList::ConstIterator it = imageTypes.begin(); it != imageTypes.end(); ++it) {
            QString mimeType = imageTypeToMimeType(*it);
            if (!mimeType.isNull())
                supportedMimeTypesList.append(mimeType);
        }
    }

    return supportedMimeTypesList;
}

QString UMLViewImageExporterModel::imageTypeToMimeType(const QString& imageType)
{
    const QString imgType = imageType.toLower();
    if (QString("bmp") == imgType) return "image/bmp";
    if (QString("jpeg") == imgType) return "image/jpeg";
    if (QString("pbm") == imgType) return "image/x-portable-bitmap";
    if (QString("pgm") == imgType) return "image/x-portable-graymap";
    if (QString("png") == imgType) return "image/png";
    if (QString("ppm") == imgType) return "image/x-portable-pixmap";
    if (QString("xbm") == imgType) return "image/x-xbitmap";
    if (QString("xpm") == imgType) return "image/x-xpixmap";
    if (QString("eps") == imgType) return "image/x-eps";
    if (QString("svg") == imgType) return "image/svg+xml";
    return QString();
}

QString UMLViewImageExporterModel::mimeTypeToImageType(const QString& mimeType)
{
    if (QString("image/bmp") == mimeType) return "bmp";
    if (QString("image/jpeg") == mimeType) return "jpeg";
    if (QString("image/x-portable-bitmap") == mimeType) return "pbm";
    if (QString("image/x-portable-graymap") == mimeType) return "pgm";
    if (QString("image/png") == mimeType) return "png";
    if (QString("image/x-portable-pixmap") == mimeType) return "ppm";
    if (QString("image/x-xbitmap") == mimeType) return "xbm";
    if (QString("image/x-xpixmap") == mimeType) return "xpm";
    if (QString("image/x-eps") == mimeType) return "eps";
    if (QString("image/svg+xml") == mimeType) return "svg";
    return QString();
}

QStringList UMLViewImageExporterModel::exportAllViews(const QString &imageType, const KUrl &directory, bool useFolders) const
{
    UMLApp *app = UMLApp::app();

    // contains all the error messages returned by exportView calls
    QStringList errors;

    UMLViewList views = app->getDocument()->getViewIterator();
    foreach (UMLView *view , views ) {
        KUrl url = directory;
        url.addPath(getDiagramFileName(view, imageType, useFolders));

        QString returnString = exportView(view, imageType, url);
        if (!returnString.isNull()) {
            errors.append(view->getName() + ": " + returnString);
        }
    }

    return errors;
}

QString UMLViewImageExporterModel::exportView(UMLView* view, const QString &imageType, const KUrl &url) const
{
    // create the needed directories
    if (!prepareDirectory(url)) {
        return i18n("Can not create directory: %1", url.directory());
    }

    // The fileName will be used when exporting the image. If the url isn't local,
    // the fileName is the name of a temporal local file to export the image to, and then
    // upload it to its destiny
    QString fileName;
    KTemporaryFile tmpFile;
    if (url.isLocalFile()) {
        fileName = url.path();
    } else {
        tmpFile.open();
        fileName = tmpFile.fileName();
    }

    // check that the diagram isn't empty
    QRect rect = view->getDiagramRect();
    if (rect.isEmpty()) {
        return i18n("Can not save an empty diagram");
    }

    // exporting the view to the file
    if (!exportViewTo(view, imageType, fileName)) {
        return i18n("A problem occurred while saving diagram in %1", fileName);
    }

    // if the file wasn't local, upload the temp file to the target
    if (!url.isLocalFile()) {
        if (!KIO::NetAccess::upload(tmpFile.fileName(), url, UMLApp::app())) {
            return i18n("There was a problem saving file: %1", url.path());
        }
    } //!isLocalFile

    return QString();
}

QString UMLViewImageExporterModel::getDiagramFileName(UMLView *view, const QString &imageType, bool useFolders /* = false */) const
{
    QString name = view->getName() + '.' + imageType.toLower();

    if (!useFolders) {
        return name;
    }

    qApp->processEvents();
    UMLListView *listView = UMLApp::app()->getListView();
    UMLListViewItem* listViewItem = listView->findItem(view->getID());
    // skip the name of the first item because it's the View
    listViewItem = static_cast<UMLListViewItem*>(listViewItem->parent());

    // Relies on the tree structure of the UMLListView. There are a base "Views" folder
    // and five children, one for each view type (Logical, use case, components, deployment
    // and entity relationship)
    while (listView->rootView(listViewItem->getType()) == NULL) {
        name.insert(0, listViewItem->getText() + '/');
        listViewItem = static_cast<UMLListViewItem*>(listViewItem->parent());
        if (listViewItem == NULL)
            break;
    }
    return name;
}

bool UMLViewImageExporterModel::prepareDirectory(const KUrl &url) const
{
    // the KUrl is copied to get protocol, user and so on and then the path is cleaned
    KUrl directory = url;
    directory.setPath("");

    // creates the directory and any needed parent directories
    QStringList dirs = url.directory().split(QDir::separator(), QString::SkipEmptyParts );
    for (QStringList::ConstIterator it = dirs.begin() ; it != dirs.end(); ++it) {
        directory.addPath(*it);

        if (!KIO::NetAccess::exists(directory, KIO::NetAccess::SourceSide, UMLApp::app())) {

            if (!KIO::NetAccess::mkdir(directory, UMLApp::app())) {
                return false;
            }
        }
    }

    return true;
}

bool UMLViewImageExporterModel::exportViewTo(UMLView* view, const QString &imageType, const QString &fileName) const
{
    // remove 'blue squares' from exported picture.
    view->clearSelected();

    QString imageMimeType = UMLViewImageExporterModel::imageTypeToMimeType(imageType);
    if (imageMimeType == "image/x-eps") {
        if (!exportViewToEps(view, fileName, true)) {
            return false;
        }
    } else if (imageMimeType == "image/svg+xml") {
        if (!exportViewToSvg(view, fileName)) {
            return false;
        }
    } else {
        if (!exportViewToPixmap(view, imageType, fileName)) {
            return false;
        }
    }

    return true;
}

bool UMLViewImageExporterModel::exportViewToEps(UMLView* view, const QString &fileName, bool isEPS) const
{
    bool exportSuccessful = true;

    // print the image to a normal postscript file,
    // do not clip so that everything ends up in the file
    // regardless of "paper size"

    // because we want to work with postscript
    // user-coordinates, set to the resolution
    // of the printer (which should be 72dpi here)
    QPrinter *printer;

    if (isEPS == false) {
        printer = new QPrinter(QPrinter::PrinterResolution);
    } else {
        printer = new QPrinter(QPrinter::ScreenResolution);
    }
    printer->setOutputFileName(fileName);
    printer->setColorMode(QPrinter::Color);

    // do not call printer.setup(); because we want no user
    // interaction here
    QPainter *painter = new QPainter(printer);

    // make sure the widget sizes will be according to the
    // actually used printer font, important for getDiagramRect()
    // and the actual painting
    view->forceUpdateWidgetFontMetrics(painter);

    QRect rect = view->getDiagramRect();
    painter->translate(-rect.x(),-rect.y());
    view->getDiagram(rect,*painter);

    int resolution = printer->resolution();

    // delete painter and printer before we try to open and fix the file
    delete painter;
    delete printer;
    if (isEPS) {
        // modify bounding box from screen to eps resolution.
        rect.setWidth( int(ceil(rect.width() * 72.0/resolution)) );
        rect.setHeight( int(ceil(rect.height() * 72.0/resolution)) );
        exportSuccessful = fixEPS(fileName,rect);
    }
    // next painting will most probably be to a different device (i.e. the screen)
    view->forceUpdateWidgetFontMetrics(0);

    return exportSuccessful;
}

bool UMLViewImageExporterModel::fixEPS(const QString &fileName, const QRect& rect) const
{
    // now open the file and make a correct eps out of it
    QFile epsfile(fileName);
    if (! epsfile.open(QIODevice::ReadOnly)) {
        return false;
    }
    // read
    QTextStream ts(&epsfile);
    QString fileContent = ts.readAll();
    epsfile.close();

    // read information
    QRegExp rx("%%BoundingBox:\\s*(-?[\\d\\.:]+)\\s*(-?[\\d\\.:]+)\\s*(-?[\\d\\.:]+)\\s*(-?[\\d\\.:]+)");
    const int pos = rx.indexIn(fileContent);
    if (pos < 0) {
        uError() << fileName << ": cannot find %%BoundingBox";
        return false;
    }

    // write new content to file
    if (! epsfile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        uError() << fileName << ": cannot open file for writing";
        return false;
    }

    // be careful when rounding (ceil/floor) the BB, these roundings
    // were mainly obtained experimentally...
    const double epsleft = rx.cap(1).toFloat();
    const double epstop = rx.cap(4).toFloat();
    const int left = int(floor(epsleft));
    const int right = int(ceil(epsleft)) + rect.width();
    const int top = int(ceil(epstop)) + 1;
    const int bottom = int(floor(epstop)) - rect.height() + 1;

    // modify content
    fileContent.replace(pos,rx.cap(0).length(),
                        QString("%%BoundingBox: %1 %2 %3 %4").arg(left).arg(bottom).arg(right).arg(top));

    ts << fileContent;
    epsfile.close();

    return true;
}

bool UMLViewImageExporterModel::exportViewToSvg(UMLView* view, const QString &fileName) const
{
    bool exportSuccessful;
    QRect rect = view->getDiagramRect();

    QSvgGenerator generator;
    generator.setFileName(fileName);
    generator.setSize(rect.size());
    QPainter painter(&generator);

    // make sure the widget sizes will be according to the
    // actually used printer font, important for getDiagramRect()
    // and the actual painting
    view->forceUpdateWidgetFontMetrics(&painter);


    painter.translate(-rect.x(),-rect.y());
    view->getDiagram(rect, painter);
    painter.end();

    //FIXME: Determine the status of svg generation.
    exportSuccessful = true;

    // next painting will most probably be to a different device (i.e. the screen)
    view->forceUpdateWidgetFontMetrics(0);

    uDebug() << "saving to file " << fileName << " successful=" << exportSuccessful;
    return exportSuccessful;
}

bool UMLViewImageExporterModel::exportViewToPixmap(UMLView* view, const QString &imageType, const QString &fileName) const
{
    bool exportSuccessful;
    QRect rect = view->getDiagramRect();
    QPixmap diagram(rect.width(), rect.height());
    view->getDiagram(rect, diagram);
    exportSuccessful = diagram.save(fileName, qPrintable(imageType.toUpper()));

    uDebug() << "saving to file " << fileName << " , imageType=" << imageType << " successful=" << exportSuccessful;
    return exportSuccessful;
}
