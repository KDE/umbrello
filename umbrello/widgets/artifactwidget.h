/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ArtifactWidget : public UMLWidget
{
public:
    ArtifactWidget(UMLScene *scene, UMLArtifact *a);
    virtual ~ArtifactWidget();

    virtual void paint(QPainter& p, int offsetX, int offsetY);

    // Note: For loading from XMI, the inherited parent method is
    //       used.
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    UMLSceneSize minimumSize();

private:
    QSize calculateIconSize();
    QSize calculateNormalSize();

    void drawAsFile(QPainter& p, int offsetX, int offsetY);
    void drawAsLibrary(QPainter& p, int offsetX, int offsetY);
    void drawAsTable(QPainter& p, int offsetX, int offsetY);
    void drawAsNormal(QPainter& p, int offsetX, int offsetY);


    /**
     * The right mouse button menu
     */
    ListPopupMenu* m_pMenu;
};

#endif
