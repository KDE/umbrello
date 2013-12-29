/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
 * @short A graphical version of a Artifact.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class ArtifactWidget : public UMLWidget
{
public:
    ArtifactWidget(UMLScene *scene, UMLArtifact *a);
    virtual ~ArtifactWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    // Note: For loading from XMI, the inherited parent method is used.
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    QSizeF minimumSize();

private:
    void paintAsFile(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintAsLibrary(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintAsTable(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintAsNormal(QPainter *painter, const QStyleOptionGraphicsItem *option);

    QSize calculateIconSize();
    QSize calculateNormalSize();

};

#endif
