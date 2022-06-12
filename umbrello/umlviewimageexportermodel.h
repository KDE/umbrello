/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLVIEWIMAGEEXPORTERMODEL_H
#define UMLVIEWIMAGEEXPORTERMODEL_H

#include "umlscene.h"
#include "umlviewlist.h"

#include <QStringList>
#include <QRect>

// forward declarations
class KUrl;

/**
 * Exports a UMLView in various image formats.
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

    explicit UMLViewImageExporterModel(float resolution = 0.0);
    virtual ~UMLViewImageExporterModel();
#if QT_VERSION >= 0x050000
    QString exportView(UMLScene* scene, const QString &imageType, const QUrl &url) const;
#else
    QString exportView(UMLScene* scene, const QString &imageType, const KUrl &url) const;
#endif

    QStringList exportViews(const UMLViewList &views, const QString &imageType, const QUrl &directory, bool useFolders) const;
private:

    QString getDiagramFileName(UMLScene* scene, const QString &imageType, bool useFolders = false) const;

#if QT_VERSION >= 0x050000
    bool prepareDirectory(const QUrl &url) const;
#else
    bool prepareDirectory(const KUrl &url) const;
#endif

    bool exportViewTo(UMLScene* scene, const QString &imageType, const QString &fileName) const;
    bool exportViewToDot(UMLScene* scene, const QString &fileName) const;
    bool exportViewToEps(UMLScene* scene, const QString &fileName) const;
    bool exportViewToSvg(UMLScene* scene, const QString &fileName) const;
    bool exportViewToPixmap(UMLScene* scene, const QString &imageType, const QString &fileName) const;

    static QStringList *s_supportedImageTypesList;
    static QStringList *s_supportedMimeTypesList;
    float m_resolution;
};

#endif
