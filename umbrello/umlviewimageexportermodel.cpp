/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlviewimageexportermodel.h"

// system includes
#include <math.h>

// include files for Qt
#include <qstringlist.h>
#include <qrect.h>
#include <qimage.h>
#include <qpicture.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qdir.h>
#include <qregexp.h>

// kde include files
#include <kdebug.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kapplication.h>
#include <kio/netaccess.h>

// application specific includes
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umllistview.h"
#include "umllistviewitem.h"

static QStringList supportedImageTypesList;
static QStringList supportedMimeTypesList;

QStringList UMLViewImageExporterModel::supportedImageTypes() {
    if (!supportedImageTypesList.size()) {
        // specific supported formats
        supportedImageTypesList << "eps";
        supportedImageTypesList << "svg";

        // QT supported formats
        QStrList qImageFormats = QImage::outputFormats();
        for (const char* format = qImageFormats.first(); format; format = qImageFormats.next()) {
            supportedImageTypesList << QString(format).lower();
        }
    }

    return supportedImageTypesList;
}

QStringList UMLViewImageExporterModel::supportedMimeTypes() {
    if (!supportedMimeTypesList.size()) {
        QStringList imageTypes = UMLViewImageExporterModel::supportedImageTypes();
        for(QStringList::Iterator it = imageTypes.begin(); it != imageTypes.end(); ++it ) {
            QString mimeType = imageTypeToMimeType(*it);
            if (!mimeType.isNull())
                supportedMimeTypesList.append(mimeType);
        }
    }

    return supportedMimeTypesList;
}

QString UMLViewImageExporterModel::imageTypeToMimeType(const QString& imageType) {
    const QString imgType = imageType.lower();
    if (QString("bmp") == imgType) return "image/x-bmp";
    if (QString("jpeg") == imgType) return "image/jpeg";
    if (QString("pbm") == imgType) return "image/x-portable-bitmap";
    if (QString("pgm") == imgType) return "image/x-portable-greymap";
    if (QString("png") == imgType) return "image/png";
    if (QString("ppm") == imgType) return "image/x-portable-pixmap";
    if (QString("xbm") == imgType) return "image/x-xbm";
    if (QString("xpm") == imgType) return "image/x-xpm";
    if (QString("eps") == imgType) return "image/x-eps";
    if (QString("svg") == imgType) return "image/svg+xml";
    return QString::null;
}

QString UMLViewImageExporterModel::mimeTypeToImageType(const QString& mimeType) {
    if (QString("image/x-bmp") == mimeType) return "bmp";
    if (QString("image/jpeg") == mimeType) return "jpeg";
    if (QString("image/x-portable-bitmap") == mimeType) return "pbm";
    if (QString("image/x-portable-greymap") == mimeType) return "pgm";
    if (QString("image/png") == mimeType) return "png";
    if (QString("image/x-portable-pixmap") == mimeType) return "ppm";
    if (QString("image/x-xbm") == mimeType) return "xbm";
    if (QString("image/x-xpm") == mimeType) return "xpm";
    if (QString("image/x-eps") == mimeType) return "eps";
    if (QString("image/svg+xml") == mimeType) return "svg";
    return QString::null;
}

QStringList UMLViewImageExporterModel::exportAllViews(const QString &imageType, const KURL &directory, bool useFolders) const {
    UMLApp *app = UMLApp::app();

    // contains all the error messages returned by exportView calls
    QStringList errors;

    UMLViewList views = app->getDocument()->getViewIterator();
    for(UMLView *view = views.first(); view; view = views.next()) {
        KURL url = directory;
        url.addPath(getDiagramFileName(view, imageType, useFolders));

        QString returnString = exportView(view, imageType, url);
        if (!returnString.isNull()) {
            errors.append(view->getName() + ": " + returnString);
        }
    }

    return errors;
}

QString UMLViewImageExporterModel::exportView(UMLView* view, const QString &imageType, const KURL &url) const {
    // create the needed directories
    if (!prepareDirectory(url)) {
        return i18n("Can not create directory: %1").arg(url.directory());
    }

    // The fileName will be used when exporting the image. If the url isn't local,
    // the fileName is the name of a temporal local file to export the image to, and then
    // upload it to its destiny
    QString fileName;
    // tmpFile needs to be unlinked before exiting the method!!!
    KTempFile tmpFile;
    if (url.isLocalFile()) {
        fileName = url.path();
    } else {
        fileName = tmpFile.name();
    }

    // check that the diagram isn't empty
    QRect rect = view->getDiagramRect();
    if (rect.isEmpty()) {
        tmpFile.unlink();
        return i18n("Can not save an empty diagram");
    }

    // exporting the view to the file
    if (!exportViewTo(view, imageType, fileName)) {
        tmpFile.unlink();
        return i18n("A problem occured while saving diagram in %1").arg(fileName);
    }

    // if the file wasn't local, upload the temp file to the target
    if (!url.isLocalFile()) {
        if (!KIO::NetAccess::upload(tmpFile.name(), url, UMLApp::app())) {
            tmpFile.unlink();
            return i18n("There was a problem saving file: %1").arg(url.path());
        }
    } //!isLocalFile

    tmpFile.unlink();
    return QString::null;
}

QString UMLViewImageExporterModel::getDiagramFileName(UMLView *view, const QString &imageType, bool useFolders /* = false */) const {
    QString name = view->getName() + '.' + imageType.lower();

    if (!useFolders) {
        return name;
    }

    kapp->processEvents();
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

bool UMLViewImageExporterModel::prepareDirectory(const KURL &url) const {
    // the KURL is copied to get protocol, user and so on and then the path is cleaned
    KURL directory = url;
    directory.setPath("");

    // creates the directory and any needed parent directories
    QStringList dirs = QStringList::split(QDir::separator(), url.directory());
    for (QStringList::ConstIterator it = dirs.begin() ; it != dirs.end(); ++it ) {
        directory.addPath(*it);

        if (!KIO::NetAccess::exists(directory, true, UMLApp::app())) {

            if (!KIO::NetAccess::mkdir(directory, UMLApp::app())) {
                return false;
            }
        }
    }

    return true;
}

bool UMLViewImageExporterModel::exportViewTo(UMLView* view, const QString &imageType, const QString &fileName) const {
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

bool UMLViewImageExporterModel::exportViewToEps(UMLView* view, const QString &fileName, bool isEPS) const {
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
    printer->setOutputToFile(true);
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

bool UMLViewImageExporterModel::fixEPS(const QString &fileName, const QRect& rect) const {
    // now open the file and make a correct eps out of it
    QFile epsfile(fileName);
    if (! epsfile.open(IO_ReadOnly)) {
        return false;
    }
    // read
    QTextStream ts(&epsfile);
    QString fileContent = ts.read();
    epsfile.close();

    // read information
    QRegExp rx("%%BoundingBox:\\s*(-?[\\d\\.:]+)\\s*(-?[\\d\\.:]+)\\s*(-?[\\d\\.:]+)\\s*(-?[\\d\\.:]+)");
    const int pos = rx.search(fileContent);
    if (pos < 0) {
        kError() << "UMLViewImageExporterModel::fixEPS(" << fileName
                  << "): cannot find %%BoundingBox" << endl;
        return false;
    }

    // write new content to file
    if (! epsfile.open(IO_WriteOnly | IO_Truncate)) {
        kError() << "UMLViewImageExporterModel::fixEPS(" << fileName
                  << "): cannot open file for writing" << endl;
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

bool UMLViewImageExporterModel::exportViewToSvg(UMLView* view, const QString &fileName) const {
    bool exportSuccesful;

    QPicture* diagram = new QPicture();

    // do not call printer.setup(); because we want no user
    // interaction here
    QPainter* painter = new QPainter();
    painter->begin( diagram );

    // make sure the widget sizes will be according to the
    // actually used printer font, important for getDiagramRect()
    // and the actual painting
    view->forceUpdateWidgetFontMetrics(painter);

    QRect rect = view->getDiagramRect();
    painter->translate(-rect.x(),-rect.y());
    view->getDiagram(rect,*painter);
    painter->end();
    exportSuccesful = diagram->save(fileName, QString("SVG").ascii());

    // delete painter and printer before we try to open and fix the file
    delete painter;
    delete diagram;
    // next painting will most probably be to a different device (i.e. the screen)
    view->forceUpdateWidgetFontMetrics(0);

    return exportSuccesful;
}

bool UMLViewImageExporterModel::exportViewToPixmap(UMLView* view, const QString &imageType, const QString &fileName) const {
    QRect rect = view->getDiagramRect();
    QPixmap diagram(rect.width(), rect.height());
    view->getDiagram(rect, diagram);
    return diagram.save(fileName, imageType.upper().ascii());
}
