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

class UMLArtifact;

/**
 * Defines a graphical version of the @ref UMLArtifact.
 * Most of the functionality will come from the @ref UMLWidget class.
 *
 * @short A graphical version of a Artifact.
 * @author Jonathan Riddell
 * @author Gopala Krishna (port using TextItems)
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ArtifactWidget : public UMLWidget
{
public:
    ArtifactWidget(UMLArtifact *a);
    virtual ~ArtifactWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    // Note: For loading from XMI, the inherited parent method is used.
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    virtual void updateGeometry();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    virtual void updateTextItemGroups();

private:
    void drawAsFile(QPainter *painter);
    void drawAsLibrary(QPainter *painter);
    void drawAsTable(QPainter *painter);
    void drawAsNormal(QPainter *painter);

    static const QSizeF MinimumIconSize;

    enum {
        GroupIndex = 0
    };
    enum {
        StereotypeItemIndex = 0,
        NameItemIndex,
        TextItemCount
    };

    qreal m_cachedTextHeight; //< Cache textheight to speedup.
};

#endif
