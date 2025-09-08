/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    void saveToXMI(QXmlStreamWriter& writer);

    void setDrawAsType(Draw_Type type);

    Draw_Type getDrawAsType() const;

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
