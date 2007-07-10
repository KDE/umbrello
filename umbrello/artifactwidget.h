/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ARTIFACTWIDGET_H
#define ARTIFACTWIDGET_H


#include "umlwidget.h"

class UMLView;
class UMLArtifact;

#define ARTIFACT_MARGIN 5

/**
 * Defines a graphical version of the Artifact.
 * Most of the functionality will come from the @ref UMLArtifact class.
 *
 * @short A graphical version of a Artifact.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ArtifactWidget : public UMLWidget {
public:

    /**
     * Constructs a ArtifactWidget.
     *
     * @param view              The parent of this ArtifactWidget.
     * @param a         The Artifact this widget will be representing.
     */
    ArtifactWidget(UMLView *view, UMLArtifact *a);

    /**
     * destructor
     */
    virtual ~ArtifactWidget();

    /**
     * Overrides standard method
     */
    void draw(QPainter& p, int offsetX, int offsetY);

    /**
     * Saves the widget to the "artifactwidget" XMI element.
     * Note: For loading from XMI, the inherited parent method is used.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    /**
     * Overrides method from UMLWidget.
     */
    QSize calculateSize();

private:
    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * calculates the size when drawing as an icon (it's the same size for all icons)
     */
    QSize calculateIconSize();

    /**
     * calculates the size for drawing as a box
     */
    QSize calculateNormalSize();

    /**
     * draw as a file icon
     */
    void drawAsFile(QPainter& p, int offsetX, int offsetY);

    /**
     * draw as a library file icon
     */
    void drawAsLibrary(QPainter& p, int offsetX, int offsetY);

    /**
     * draw as a database table icon
     */
    void drawAsTable(QPainter& p, int offsetX, int offsetY);

    /**
     * draw as a box
     */
    void drawAsNormal(QPainter& p, int offsetX, int offsetY);

    /**
     * The right mouse button menu
     */
    ListPopupMenu* m_pMenu;
};

#endif
