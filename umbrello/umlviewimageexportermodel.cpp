/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlviewimageexportermodel.h"

// application specific includes
#include "debug_utils.h"
#include "model_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde include files
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kio/netaccess.h>

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

// system includes
#include <cmath>

#define DBG_IEM "UMLViewImageExporterModel"

QStringList UMLViewImageExporterModel::s_supportedImageTypesList;
QStringList UMLViewImageExporterModel::s_supportedMimeTypesList;

/**
 * Returns a QStringList containing all the supported image types to use when exporting.
 * All the types will be lower case.
 *
 * @return A QStringList containing all the supported image types to use when exporting.
 */
QStringList UMLViewImageExporterModel::supportedImageTypes()
{
    if (!s_supportedImageTypesList.size()) {
        // QT supported formats
        QList<QByteArray> qImageFormats = QImageWriter::supportedImageFormats();
        Q_FOREACH(const QByteArray& it, qImageFormats) {
            const QString format = it.toLower();
            if (!s_supportedImageTypesList.contains(format))
                s_supportedImageTypesList << format;
        }
        // specific supported formats
        if (!s_supportedImageTypesList.contains("eps"))
            s_supportedImageTypesList << "eps";
        if (!s_supportedImageTypesList.contains("svg"))
            s_supportedImageTypesList << "svg";
    }
    s_supportedImageTypesList.sort();

    return s_supportedImageTypesList;
}

/**
 * Returns a QStringList containing all the supported mime types to use when exporting.
 * All the types will be lower case.
 *
 * @return A QStringList containing all the supported mime types to use when exporting.
 */
QStringList UMLViewImageExporterModel::supportedMimeTypes()
{
    if (!s_supportedMimeTypesList.size()) {
        const QStringList imageTypes = UMLViewImageExporterModel::supportedImageTypes();
        for (QStringList::ConstIterator it = imageTypes.begin(); it != imageTypes.end(); ++it) {
            QString mimeType = imageTypeToMimeType(*it);
            if (!mimeType.isNull())
                s_supportedMimeTypesList.append(mimeType);
        }
    }

    return s_supportedMimeTypesList;
}

/**
 * Returns the mime type for an image type.
 * The supported image types are those that the diagrams can be exported to.
 *
 * @param imageType The type of the image.
 * @return A QString with the equivalent mime type, or QString() if
 *         it's unknown.
 */
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

/**
 * Returns the image type for a mime type.
 * The supported image types are those that the diagrams can be exported to.
 *
 * @param mimeType The mime type.
 * @return A lowercase QString with the equivalent image type, or QString()
 *         if it's unknown.
 */
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

/**
 * Constructor for UMLViewImageExporterModel.
 */
UMLViewImageExporterModel::UMLViewImageExporterModel()
{
    DEBUG_REGISTER(DBG_IEM);
}

/**
 * Destructor for UMLViewImageExporterModel.
 */
UMLViewImageExporterModel::~UMLViewImageExporterModel()
{
}

/**
 * Exports all the views in the document to the directory specified in the url
 * using the 'imageType' for the images.
 * The name of the exported images will be like their view's name and using the
 * 'imageType' as extension.
 *
 * The views are stored in folders in the document. The same tree structure used
 * in the document to store the views can be created in the target directory with
 * 'useFolders'. Only the folders made by the user are created in the target
 * directory (Logical view, use case view and so on aren't created).
 *
 * This method creates the specified target directory if needed. If there was an
 * existing file with the same path as one to be created overwrites it without asking.
 * The url used can be local or remote, using supported KIO slaves.
 *
 * @param imageType The type of the images the views will be exported to.
 * @param directory The url of the directory where the images will be saved.
 * @param useFolders If the tree structure of the views in the document must be created
 *                   in the target directory.
 * @return A QStringList with all the error messages that occurred during export.
 *         If the list is empty, all the views were exported successfully.
 */
QStringList UMLViewImageExporterModel::exportAllViews(const QString &imageType, const KUrl &directory, bool useFolders) const
{
    UMLApp *app = UMLApp::app();

    // contains all the error messages returned by exportView calls
    QStringList errors;

    UMLViewList views = app->document()->viewIterator();
    foreach (UMLView *view , views) {
        KUrl url = directory;
        url.addPath(getDiagramFileName(view->umlScene(), imageType, useFolders));

        QString returnString = exportView(view->umlScene(), imageType, url);
        if (!returnString.isNull()) {
            // [PORT]
            errors.append(view->umlScene()->name() + ": " + returnString);
        }
    }

    return errors;
}

/**
 * Exports the view to the url using the 'imageType' for the image.
 *
 * This method creates the needed directories, if any. If there was an existing
 * file in the specified url overwrites it without asking.
 * The url used can be local or remote, using supported KIO slaves.
 *
 * If some problem occurs when exporting, an error message is returned.
 *
 * @param view The view to export.
 * @param imageType The type of the image the view will be exported to.
 * @param url The url where the image will be saved.
 * @return The message error if some problem occurred when exporting, or
 *         QString() if all went fine.
 */
QString UMLViewImageExporterModel::exportView(UMLScene* scene, const QString &imageType, const KUrl &url) const
{
    if (!scene) {
        return i18n("Empty scene");
    }

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
        fileName = url.toLocalFile();
    } else {
        tmpFile.open();
        fileName = tmpFile.fileName();
    }

    QRectF rect = scene->diagramRect();
    if (rect.isEmpty()) {
        return i18n("Can not save an empty diagram");
    }

    // exporting the view to the file
    if (!exportViewTo(scene, imageType, fileName)) {
        return i18n("A problem occurred while saving diagram in %1", fileName);
    }

    // if the file wasn't local, upload the temp file to the target
    if (!url.isLocalFile()) {
        if (!KIO::NetAccess::upload(tmpFile.fileName(), url, UMLApp::app())) {
            return i18n("There was a problem saving file: %1", url.path());
        }
    }

    return QString();
}

/**
 * Returns the diagram file name.
 * @param scene        the diagram
 * @param imageType    the image type as file name extension
 * @param useFolders   flag whether to add folder to the file name
 * @return the file name with extension
 */
QString UMLViewImageExporterModel::getDiagramFileName(UMLScene* scene, const QString &imageType, bool useFolders /* = false */) const
{
    if (scene) {
        if (useFolders) {
            qApp->processEvents();  //:TODO: still needed ???
            return Model_Utils::treeViewBuildDiagramName(scene->getID());
        }
        else {
            return scene->name() + '.' + imageType.toLower();;
        }
    }
    else {
        uWarning() << "Scene is null!";
        return QString();
    }
}

/**
 * Creates, if it doesn't exist, the directory to save the file.
 * It also creates all the needed parent directories.
 *
 * @param url The url where the image will be saved.
 * @return True if the operation was successful,
 *         false if the directory didn't exist and couldn't be created.
 */
bool UMLViewImageExporterModel::prepareDirectory(const KUrl &url) const
{
    // the KUrl is copied to get protocol, user and so on and then the path is cleaned
    KUrl directory = url;
    directory.setPath("");

    // creates the directory and any needed parent directories
    QStringList dirs = url.directory().split(QDir::separator(), QString::SkipEmptyParts );
    for (QStringList::ConstIterator it = dirs.constBegin() ; it != dirs.constEnd(); ++it) {
        directory.addPath(*it);

        if (!KIO::NetAccess::exists(directory, KIO::NetAccess::SourceSide, UMLApp::app())) {

            if (!KIO::NetAccess::mkdir(directory, UMLApp::app())) {
                return false;
            }
        }
    }

    return true;
}

/**
 * Exports the scene to the file 'fileName' as the specified type.
 *
 * @param scene     The scene to export.
 * @param imageType The type of the image the scene will be exported to.
 * @param fileName  The name of the file where the image will be saved.
 * @return True if the operation was successful,
 *         false if a problem occurred while exporting.
 */
bool UMLViewImageExporterModel::exportViewTo(UMLScene* scene, const QString &imageType, const QString &fileName) const
{
    if (!scene) {
        uWarning() << "Scene is null!";
        return false;
    }

    // remove 'blue squares' from exported picture.
    scene->clearSelected();

    QString imageMimeType = UMLViewImageExporterModel::imageTypeToMimeType(imageType);
    if (imageMimeType == "image/x-eps") {
        if (!exportViewToEps(scene, fileName, true)) {
            return false;
        }
    } else if (imageMimeType == "image/svg+xml") {
        if (!exportViewToSvg(scene, fileName)) {
            return false;
        }
    } else {
        if (!exportViewToPixmap(scene, imageType, fileName)) {
            return false;
        }
    }

    return true;
}

/**
 * Exports the view to the file 'fileName' as EPS.
 *
 * @param scene    The scene to export.
 * @param fileName The name of the file where the image will be saved.
 * @param isEPS    The file is an eps file and needs adjusting
 *                 of the eps bounding box values.
 * @return True if the operation was successful,
 *         false if a problem occurred while exporting.
 */
bool UMLViewImageExporterModel::exportViewToEps(UMLScene* scene, const QString &fileName, bool isEPS) const
{
    if (!scene) {
        uWarning() << "Scene is null!";
        return false;
    }

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
    printer->setOutputFormat(QPrinter::PostScriptFormat);
    printer->setColorMode(QPrinter::Color);

    // do not call printer.setup(); because we want no user
    // interaction here
    QPainter *painter = new QPainter(printer);

    // make sure the widget sizes will be according to the
    // actually used printer font, important for diagramRect()
    // and the actual painting
    scene->forceUpdateWidgetFontMetrics(painter);

    QRect rect = scene->diagramRect();
    painter->translate(-rect.x(), -rect.y());
    scene->getDiagram(rect, *painter);

    int resolution = printer->resolution();

    // delete painter and printer before we try to open and fix the file
    delete painter;
    delete printer;
    if (isEPS) {
        // modify bounding box from screen to eps resolution.
        rect.setWidth( int(ceil(rect.width() * 72.0/resolution)) );
        rect.setHeight( int(ceil(rect.height() * 72.0/resolution)) );
        exportSuccessful = fixEPS(fileName, rect);
    }
    // next painting will most probably be to a different device (i.e. the screen)
    scene->forceUpdateWidgetFontMetrics(0);

    return exportSuccessful;
}

/**
 * Fix the file 'fileName' to be a valid EPS containing the
 * specified area (rect) of the diagram.
 * Corrects the bounding box.
 *
 * @return True if the operation was successful,
 *         false if a problem occurred while exporting.
 */
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

/**
 * Exports the view to the file 'fileName' as SVG.
 *
 * @param scene    The scene to export.
 * @param fileName The name of the file where the image will be saved.
 * @return True if the operation was successful,
 *         false if a problem occurred while exporting.
 */
bool UMLViewImageExporterModel::exportViewToSvg(UMLScene* scene, const QString &fileName) const
{
    if (!scene) {
        uWarning() << "Scene is null!";
        return false;
    }

    bool exportSuccessful;
    QRect rect = scene->diagramRect();

    QSvgGenerator generator;
    generator.setFileName(fileName);
    generator.setSize(rect.size());
    generator.setViewBox(QRect(0, 0, rect.width(), rect.height()));
    QPainter painter(&generator);

    // make sure the widget sizes will be according to the
    // actually used printer font, important for diagramRect()
    // and the actual painting
//    scene->forceUpdateWidgetFontMetrics(&painter);
    //Note: The above was commented out because other exportViewTo...
    //      do not have it and it forces a resize of the widgets,
    //      which is not correctly implemented for now.

    painter.translate(-rect.x(),-rect.y());
    scene->getDiagram(rect, painter);
    painter.end();

    //FIXME: Determine the status of svg generation.
    exportSuccessful = true;

    // next painting will most probably be to a different device (i.e. the screen)
//    scene->forceUpdateWidgetFontMetrics(0);
    //Note: See comment above.

    DEBUG(DBG_IEM) << "saving to file " << fileName << " successful=" << exportSuccessful;
    return exportSuccessful;
}

/**
 * Exports the view to the file 'fileName' as a pixmap of the specified type.
 * The valid types are those supported by QPixmap save method.
 *
 * @param scene     The scene to export.
 * @param imageType The type of the image the view will be exported to.
 * @param fileName  The name of the file where the image will be saved.
 * @return True if the operation was successful,
 *         false if a problem occurred while exporting.
 */
bool UMLViewImageExporterModel::exportViewToPixmap(UMLScene* scene, const QString &imageType, const QString &fileName) const
{
    if (!scene) {
        uWarning() << "Scene is null!";
        return false;
    }

    bool exportSuccessful;
    QRect rect = scene->diagramRect();
    QPixmap diagram(rect.width(), rect.height());
    scene->getDiagram(rect, diagram);
    exportSuccessful = diagram.save(fileName, qPrintable(imageType.toUpper()));

    DEBUG(DBG_IEM) << "saving to file " << fileName << " , imageType=" << imageType << " successful=" << exportSuccessful;
    return exportSuccessful;
}
