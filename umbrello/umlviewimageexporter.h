/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLVIEWIMAGEEXPORTER_H
#define UMLVIEWIMAGEEXPORTER_H

#include "umlviewimageexportermodel.h"

#include <QString>
#include <QUrl>

class UMLScene;
class UMLFileDialog;

/**
 * Exports the scene as an image.
 * This class takes care of asking the user the needed parameters and
 * then exports the scene.
 */
class UMLViewImageExporter
{
public:

    explicit UMLViewImageExporter(UMLScene* scene);
    virtual ~UMLViewImageExporter();

    void exportView();
    QUrl    getImageURL() const { return m_imageURL; }
    QString getImageMimeType() const { return m_imageMimeType; }

private:

    UMLScene* m_scene;          ///< The scene to export.
    QUrl      m_imageURL;       ///< The URL used to save the image.
    QString   m_imageMimeType;  ///< The mime type used to save the image.

    bool getParametersFromUser();

    bool prepareExport();
    void prepareFileDialog(UMLFileDialog *fileDialog);

};

#endif
