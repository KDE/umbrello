/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ARTIFACT_H
#define ARTIFACT_H

#include "package.h"

/**
 * This class contains the non-graphical information required for a UML
 * Artifact.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * @short Non-graphical information for a Artifact.
 * @author Jonathan Riddell
 * @see UMLCanvasObject
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLArtifact : public UMLPackage
{
    Q_OBJECT
public:

    /**
     * Artifacts can be drawn using one of several icons.
     */
    enum Draw_Type {
        defaultDraw,
        file,
        library,
        table
    };

    explicit UMLArtifact(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLArtifact();

    virtual UMLObject* clone() const;

    void saveToXMI1(QDomDocument & qDoc, QDomElement & qElement);

    void setDrawAsType(Draw_Type type);

    Draw_Type getDrawAsType();

    QString fullPath() const;

protected:

    bool load1(QDomElement & element);

private:

    /**
     * Artifacts can be drawn as one of several different icons,
     * this value choosing how to draw them.
     */
    Draw_Type m_drawAsType;
};

#endif
