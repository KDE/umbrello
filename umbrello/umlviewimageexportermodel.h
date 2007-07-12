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

#ifndef UMLVIEWIMAGEEXPORTERMODEL_H
#define UMLVIEWIMAGEEXPORTERMODEL_H

#include <qstringlist.h>
#include <qrect.h>

// forward declarations
class UMLView;

// KDE forward declarations
class KTempFile;
class KURL;

/**
 * Exports an UMLView in various image formats.
 * It can also export all the views in the current document.
 *
 * The methods in this class don't communicate with the user, so asking the format
 * to save the images in, checking if the target file exists and so on must be done before
 * calling those methods, if needed.
 * The only exception is asking passwords for example when KIO slaves are used, as this
 * operation is made automatically by the KIO classes.
 */
class UMLViewImageExporterModel {
public:

    /**
     * Returns a QStringList containing all the supported image types to use when exporting.
     * All the types will be lower case.
     *
     * @return A QStringList containing all the supported image types to use when exporting.
     */
    static QStringList supportedImageTypes();

    /**
     * Returns a QStringList containing all the supported mime types to use when exporting.
     * All the types will be lower case.
     *
     * @return A QStringList containing all the supported mime types to use when exporting.
     */
    static QStringList supportedMimeTypes();

    /**
     * Returns the mime type for an image type.
     * The supported image types are those that the diagrams can be exported to.
     *
     * @param imageType The type of the image.
     * @return A QString with the equivalent mime type, or QString::null if
     *         it's unknown.
     */
    static QString imageTypeToMimeType(const QString& imageType);

    /**
     * Returns the image type for a mime type.
     * The supported image types are those that the diagrams can be exported to.
     *
     * @param mimeType The mime type.
     * @return A lowercase QString with the equivalent image type, or QString::null
     *         if it's unknown.
     */
    static QString mimeTypeToImageType(const QString& mimeType);

    /**
     * Constructor for UMLViewImageExporterModel.
     */
    UMLViewImageExporterModel() {
    }

    /**
     * Destructor for UMLViewImageExporterModel.
     */
    virtual ~UMLViewImageExporterModel() {
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
    QStringList exportAllViews(const QString &imageType, const KURL &directory, bool useFolders) const;

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
     *         QString::null if all went fine.
     */
    QString exportView(UMLView* view, const QString &imageType, const KURL &url) const;

private:

    /**
     * Returns the name of the file where the view will be exported to.
     * The name of the exported images will be like their view's name and using the
     * 'imageType' as extension. It can also include the parent folders of the view.
     *
     * The views are stored in folders in the document. The same tree structure used
     * in the document to store the views can be created with 'useFolders', so the file name
     * will include recursively also its parent folders. Only the folders made by the user
     * are included in the file name (Logical view, use case view and so on aren't created).
     *
     * @param view The view to export.
     * @param imageType The type of the image the view will be exported to.
     * @param useFolders If the tree structure of the views in the document must be included
     *                   in the file name.
     * @return The name of the file where the view will be exported to.
     */
    QString getDiagramFileName(UMLView *view, const QString &imageType, bool useFolders = false) const;

    /**
     * Creates, if it doesn't exist, the directory to save the file.
     * It also creates all the needed parent directories.
     *
     * @param url The url where the image will be saved.
     * @return True if the operation was successful,
     *         false if the directory didn't exist and couldn't be created.
     */
    bool prepareDirectory(const KURL &url) const;

    /**
     * Exports the view to the file 'fileName' as the specified type.
     *
     * @param view The view to export.
     * @param imageType The type of the image the view will be exported to.
     * @param fileName The name of the file where the image will be saved.
     * @return True if the operation was successful,
     *         false if a problem occurred while exporting.
     */
    bool exportViewTo(UMLView* view, const QString &imageType, const QString &fileName) const;

    /**
     * Exports the view to the file 'fileName' as EPS.
     *
     * @param view The view to export.
     * @param fileName The name of the file where the image will be saved.
     * @param isEPS The file is an eps file and needs adjusting
     *              of the eps bounding box values.
     * @return True if the operation was successful,
     *         false if a problem occurred while exporting.
     */
    bool exportViewToEps(UMLView* view, const QString &fileName, bool isEPS) const;

    /**
     * Fix the file 'fileName' to be a valid EPS containing the
     * specified area (rect) of the diagram.
     * Corrects the bounding box.
     *
     * @return True if the operation was successful,
     *         false if a problem occurred while exporting.
     */
    bool fixEPS(const QString &fileName, const QRect& rect) const;

    /**
     * Exports the view to the file 'fileName' as SVG.
     *
     * @param view The view to export.
     * @param fileName The name of the file where the image will be saved.
     * @return True if the operation was successful,
     *         false if a problem occurred while exporting.
     */
    bool exportViewToSvg(UMLView* view, const QString &fileName) const;

    /**
     * Exports the view to the file 'fileName' as a pixmap of the specified type.
     * The valid types are those supported by QPixmap save method.
     *
     * @param view The view to export.
     * @param imageType The type of the image the view will be exported to.
     * @param fileName The name of the file where the image will be saved.
     * @return True if the operation was successful,
     *         false if a problem occurred while exporting.
     */
    bool exportViewToPixmap(UMLView* view, const QString &imageType, const QString &fileName) const;

};

#endif
