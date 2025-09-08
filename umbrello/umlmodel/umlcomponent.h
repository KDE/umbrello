/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef COMPONENT_H
#define COMPONENT_H

#include "package.h"

/**
 * This class contains the non-graphical information required for a
 * UML Component.
 * This class inherits from @ref UMLPackage which contains most
 * of the information.
 *
 * @short Non-graphical information for a Component.
 * @author Jonathan Riddell
 * @see UMLCanvasObject
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLComponent : public UMLPackage
{
    Q_OBJECT
public:
    explicit UMLComponent(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLComponent();

    virtual UMLObject* clone() const;

    void saveToXMI(QXmlStreamWriter& writer);

    void setExecutable(bool executable);
    bool getExecutable() const;

protected:
    bool load1(QDomElement & element);

private:
    bool m_executable;  ///< holds whether this is an executable component or not
};

#endif
