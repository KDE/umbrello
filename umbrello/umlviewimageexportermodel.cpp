/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlviewimageexportermodel.h"

// application specific includes
#include "debug_utils.h"
#include "dotgenerator.h"
#include "model_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde include files
#if QT_VERSION >= 0x050000
#include <KIO/Job>
#include <KJobWidgets>
#include <KIO/MkdirJob>
#else
#include <kio/netaccess.h>
#endif
#if QT_VERSION < 0x050000
#include <ktemporaryfile.h>
#endif
#include <KLocalizedString>

// include files for Qt
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QImage>
#include <QImageWriter>
#include <QPainter>
#include <QPicture>
#include <QPrinter>
#include <QRect>
#include <QRegExp>
#include <QSvgGenerator>
#if QT_VERSION >= 0x050000
#include <QTemporaryFile>
#endif

// system includes
#include <cmath>

#define DBG_IEM QLatin1String("UMLViewImageExporterModel")

DEBUG_REGISTER(UMLViewImageExporterModel)

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
            const QString format = QString::fromLatin1(it.toLower());
            if (!s_supportedImageTypesList.contains(format))
                s_supportedImageTypesList << format;
        }
        // specific supported formats
        if (!s_supportedImageTypesList.contains(QLatin1String("dot")))
            s_supportedImageTypesList << QLatin1String("dot");
        if (!s_supportedImageTypesList.contains(QLatin1String("eps")))
            s_supportedImageTypesList << QLatin1String("eps");
        if (!s_supportedImageTypesList.contains(QLatin1String("svg")))
            s_supportedImageTypesList << QLatin1String("svg");
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
    struct lut_t { const char *key, *value; };
    const struct lut_t lut[] =
               { { "bmp", "image/bmp"                },
                 { "dot", "image/x-dot"              },
                 { "jpeg","image/jpeg"               },
                 { "pbm", "image/x-portable-bitmap"  },
                 { "pgm", "image/x-portable-graymap" },
                 { "png", "image/png"                },
                 { "ppm", "image/x-portable-pixmap"  },
                 { "xbm", "image/x-xbitmap"          },
                 { "xpm", "image/x-xpixmap"          },
                 { "eps", "image/x-eps"              },
                 { "svg", "image/svg+xml"            } };
    const size_t lut_len = sizeof(lut) / sizeof(lut_t);
    for (size_t i = 0; i < lut_len; i++) {
        const lut_t& ent = lut[i];
        if (imgType == QString::fromLatin1(ent.key))
            return QString::fromLatin1(ent.value);
    }
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
    struct lut_t { const char *key, *value; };
    const struct lut_t lut[] =
               { { "image/bmp",                "bmp" },
                 { "image/x-dot",              "dot" },
                 { "image/jpeg",               "jpeg"},
                 { "image/x-portable-bitmap",  "pbm" },
                 { "image/x-portable-graymap", "pgm" },
                 { "image/png",                "png" },
                 { "image/x-portable-pixmap",  "ppm" },
                 { "image/x-xbitmap",          "xbm" },
                 { "image/x-xpixmap",          "xpm" },
                 { "image/x-eps",              "eps" },
                 { "image/svg+xml",            "svg" } };
    const size_t lut_len = sizeof(lut) / sizeof(lut_t);
    for (size_t i = 0; i < lut_len; i++) {
        const lut_t& ent = lut[i];
        if (mimeType == QString::fromLatin1(ent.key))
            return QString::fromLatin1(ent.value);
    }
    return QString();
}

/**
 * Constructor for UMLViewImageExporterModel.
 * @param resolution resolution of export in DPI (default 0.0 means export type related default)
 */
UMLViewImageExporterModel::UMLViewImageExporterModel(float resolution)
  : m_resolution(resolution)
{
}

/**
 * Destructor for UMLViewImageExporterModel.
 */
UMLViewImageExporterModel::~UMLViewImageExporterModel()
{
}

/**
 * Exports views in the document to the directory specified in the url
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
QStringList UMLViewImageExporterModel::exportViews(const UMLViewList &views, const QString &imageType, const QUrl &directory, bool useFolders) const
{
    // contains all the error messages returned by exportView calls
    QStringList errors;

    foreach (UMLView *view, views) {
#if QT_VERSION >= 0x050000
        QUrl url = QUrl::fromLocalFile(directory.path() + QLatin1Char('/') +
                 getDiagramFileName(view->umlScene(), imageType, useFolders));
#else
        KUrl url = directory;
        url.addPath(getDiagramFileName(view->umlScene(), imageType, useFolders));
#endif

        QString returnString = exportView(view->umlScene(), imageType, url);
        if (!returnString.isNull()) {
            // [PORT]
            errors.append(view->umlScene()->name() + QLatin1String(": ") + returnString);
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
 * @param scene      The UMLScene to export.
 * @param imageType  The type of the image the view will be exported to.
 * @param url        The url where the image will be saved.
 * @return  The error message if some problem occurred when exporting, or
 *          QString() if all went fine.
 */
#if QT_VERSION >= 0x050000
QString UMLViewImageExporterModel::exportView(UMLScene* scene, const QString &imageType, const QUrl &url) const
#else
QString UMLViewImageExporterModel::exportView(UMLScene* scene, const QString &imageType, const KUrl &url) const
#endif
{
    if (!scene) {
        return i18n("Empty scene");
    }

    // create the needed directories
    if (!prepareDirectory(url)) {
#if QT_VERSION >= 0x050000
        return i18n("Cannot create directory: %1", url.path());
#else
        return i18n("Cannot create directory: %1", url.directory());
#endif
    }

    // The fileName will be used when exporting the image. If the url isn't local,
    // the fileName is the name of a temporary local file to export the image to, and then
    // upload it to its destiny
    QString fileName;
#if QT_VERSION >= 0x050000
    QTemporaryFile tmpFile;
#else
    KTemporaryFile tmpFile;
#endif
    if (url.isLocalFile()) {
        fileName = url.toLocalFile();
    } else {
        tmpFile.open();
        fileName = tmpFile.fileName();
    }

    // exporting the view to the file
    if (!exportViewTo(scene, imageType, fileName)) {
        return i18n("A problem occurred while saving diagram in %1", fileName);
    }

    // if the file wasn't local, upload the temp file to the target
    if (!url.isLocalFile()) {
#if QT_VERSION >= 0x050000
        KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(tmpFile.fileName()), url);
        KJobWidgets::setWindow(job, UMLApp::app());
        job->exec();
        if (job->error()) {
#else
        if (!KIO::NetAccess::upload(tmpFile.fileName(), url, UMLApp::app())) {
#endif
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
            return Model_Utils::treeViewBuildDiagramName(scene->ID()) + QLatin1Char('.') + imageType.toLower();
        }
        else {
            return scene->name() + QLatin1Char('.') + imageType.toLower();
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
#if QT_VERSION >= 0x050000
bool UMLViewImageExporterModel::prepareDirectory(const QUrl &url) const
#else
bool UMLViewImageExporterModel::prepareDirectory(const KUrl &url) const
#endif
{
    // the KUrl is copied to get protocol, user and so on and then the path is cleaned
#if QT_VERSION >= 0x050000
    QUrl directory = url;
    directory.setPath(QString());

    // creates the directory and any needed parent directories
    QStringList dirs = url.adjusted(QUrl::RemoveFilename).path().split(QDir::separator(), QString::SkipEmptyParts);
    for (QStringList::ConstIterator it = dirs.constBegin() ; it != dirs.constEnd(); ++it) {
        directory.setPath(directory.path() + QLatin1Char('/') + *it);
        KIO::StatJob *statJob = KIO::stat(directory, KIO::StatJob::SourceSide, 0);
        KJobWidgets::setWindow(statJob, UMLApp::app());
        statJob->exec();
        if (statJob->error()) {

            KIO::MkdirJob* job = KIO::mkdir(directory);
            if (!job->exec()) {
                return false;
            }
        }
    }
#else
    KUrl directory = url;
    directory.setPath(QString());

    // creates the directory and any needed parent directories
    QStringList dirs = url.directory().split(QDir::separator(), QString::SkipEmptyParts);
    for (QStringList::ConstIterator it = dirs.constBegin() ; it != dirs.constEnd(); ++it) {
        directory.addPath(*it);

        if (!KIO::NetAccess::exists(directory, KIO::NetAccess::SourceSide, UMLApp::app())) {

            if (!KIO::NetAccess::mkdir(directory, UMLApp::app())) {
                return false;
            }
        }
    }
#endif

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
    if (imageMimeType == QLatin1String("image/x-dot")) {
        if (!exportViewToDot(scene, fileName)) {
            return false;
        }
    } else if (imageMimeType == QLatin1String("image/x-eps")) {
        if (!exportViewToEps(scene, fileName)) {
            return false;
        }
    } else if (imageMimeType == QLatin1String("image/svg+xml")) {
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
 * Exports the view to the file 'fileName' as a dot file.
 *
 * @param scene     The scene to export.
 * @param fileName  The name of the file where the image will be saved.
 * @return True if the operation was successful,
 *         false if a problem occurred while exporting.
 */
bool UMLViewImageExporterModel::exportViewToDot(UMLScene* scene, const QString &fileName) const
{
    if (!scene) {
        uWarning() << "Scene is null!";
        return false;
    }

    DotGenerator dot;
    bool result = dot.createDotFile(scene, fileName, QLatin1String("export"));

    DEBUG(DBG_IEM) << "saving to file " << fileName << result;
    return result;
}

/**
 * Exports the view to the file 'fileName' as EPS.
 *
 * @param scene    The scene to export.
 * @param fileName The name of the file where the image will be saved.
 * @return True if the operation was successful,
 *         false if a problem occurred while exporting.
 */
bool UMLViewImageExporterModel::exportViewToEps(UMLScene* scene, const QString &fileName) const
{
    if (!scene) {
        uWarning() << "Scene is null!";
        return false;
    }

    qreal border = 0.01; // mm
    QRectF rect = scene->diagramRect();
    if (rect.isEmpty()) {
        rect = QRectF(0,0, 10, 10);
    }

    QSizeF paperSize(rect.size() * 25.4f / qApp->desktop()->logicalDpiX());

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFileName(fileName);
#if QT_VERSION >= 0x050000
    printer.setOutputFormat(QPrinter::PdfFormat);
#else
    printer.setOutputFormat(QPrinter::PostScriptFormat);
#endif
    printer.setColorMode(QPrinter::Color);
    printer.setPaperSize(paperSize, QPrinter::Millimeter);
    printer.setPageMargins(paperSize.width() * border, paperSize.height() * border, 0, 0, QPrinter::Millimeter);
    printer.setResolution(qApp->desktop()->logicalDpiX());
    printer.setOrientation(paperSize.width() < paperSize.height() ? QPrinter::Landscape : QPrinter::Portrait);

    // do not call printer.setup(); because we want no user
    // interaction here
    QPainter painter(&printer);

    // add border around image
    painter.scale(1 - border, 1 - border);

    // make sure the widget sizes will be according to the
    // actually used printer font, important for diagramRect()
    // and the actual painting
    scene->forceUpdateWidgetFontMetrics(&painter);

    scene->getDiagram(painter, rect);

    // next painting will most probably be to a different device (i.e. the screen)
    scene->forceUpdateWidgetFontMetrics(0);

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
    QRectF rect = scene->diagramRect();
    if (rect.isEmpty()) {
        rect = QRectF(0,0, 10, 10);
    }

    QSvgGenerator generator;
    generator.setFileName(fileName);
    generator.setSize(rect.toRect().size());
    generator.setResolution(qApp->desktop()->logicalDpiX());
    generator.setViewBox(QRect(0, 0, rect.width(), rect.height()));
    QPainter painter(&generator);

    // make sure the widget sizes will be according to the
    // actually used printer font, important for diagramRect()
    // and the actual painting
//    scene->forceUpdateWidgetFontMetrics(&painter);
    //Note: The above was commented out because other exportViewTo...
    //      do not have it and it forces a resize of the widgets,
    //      which is not correctly implemented for now.

    painter.translate(0, 0);
    scene->getDiagram(painter, rect);
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

    QRectF rect = scene->diagramRect();
    if (rect.isEmpty()) {
        rect = QRectF(0,0, 10, 10);
    }

    float scale = m_resolution != 0.0 ? m_resolution / qApp->desktop()->logicalDpiX() : 72.0f / qApp->desktop()->logicalDpiX();
    QSizeF size = rect.size() * scale;
    QPixmap diagram(size.toSize());
    scene->getDiagram(diagram, rect);
    bool exportSuccessful = diagram.save(fileName, qPrintable(imageType.toUpper()));
    DEBUG(DBG_IEM) << "saving to file " << fileName
                   << ", imageType=" << imageType
                   << ", width=" << rect.width()
                   << ", height=" << rect.height()
                   << ", successful=" << exportSuccessful;
    return exportSuccessful;
}
