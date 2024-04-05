/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ARTIFACTWIDGET_H
#define ARTIFACTWIDGET_H

#include "umlwidget.h"

class UMLScene;
class UMLArtifact;

#define ARTIFACT_MARGIN 5

/**
 * Defines a graphical version of the @ref UMLArtifact.
 * Most of the functionality will come from the @ref UMLWidget class.
 *
 * @short A graphical version of an Artifact.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ArtifactWidget : public UMLWidget
{
public:
    ArtifactWidget(UMLScene *scene, UMLArtifact *a);
    virtual ~ArtifactWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    // Note: For loading from XMI, the inherited parent method is used.
    virtual void saveToXMI(QXmlStreamWriter& writer);

protected:
    QSizeF minimumSize() const;

private:
    void paintAsFile(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintAsLibrary(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintAsTable(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintAsNormal(QPainter *painter, const QStyleOptionGraphicsItem *option);

    QSize calculateIconSize() const;
    QSize calculateNormalSize() const;

};

#endif
