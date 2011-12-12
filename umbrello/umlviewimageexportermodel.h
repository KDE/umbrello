/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLVIEWIMAGEEXPORTERMODEL_H
#define UMLVIEWIMAGEEXPORTERMODEL_H

#include <QtCore/QStringList>
#include <QtCore/QRect>

// forward declarations
class UMLScene;
class KUrl;

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
class UMLViewImageExporterModel
{
public:

    static QStringList supportedImageTypes();
    static QStringList supportedMimeTypes();

    static QString imageTypeToMimeType(const QString& imageType);
    static QString mimeTypeToImageType(const QString& mimeType);

    UMLViewImageExporterModel();
    virtual ~UMLViewImageExporterModel();

    QStringList exportAllViews(const QString &imageType, const KUrl &directory, bool useFolders) const;
    QString exportView(UMLScene* scene, const QString &imageType, const KUrl &url) const;

private:

    QString getDiagramFileName(UMLScene* scene, const QString &imageType, bool useFolders = false) const;

    bool prepareDirectory(const KUrl &url) const;

    bool exportViewTo(UMLScene* scene, const QString &imageType, const QString &fileName) const;
    bool exportViewToSvg(UMLScene* scene, const QString &fileName) const;
    bool exportViewToPixmap(UMLScene* scene, const QString &imageType, const QString &fileName) const;
    bool exportViewToEps(UMLScene* scene, const QString &fileName, bool isEPS) const;

    bool fixEPS(const QString &fileName, const QRect& rect) const;

    static QStringList s_supportedImageTypesList;
    static QStringList s_supportedMimeTypesList;

};

#endif
