/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef USECASE_H
#define USECASE_H

#include "umlcanvasobject.h"

/**
 * This class contains the non-graphical information required for a UML UseCase.
 * This class inherits from @ref UMLCanvasObject which contains most of the information.
 *
 * @short Information for a non-graphical UML UseCase.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLUseCase : public UMLCanvasObject
{
public:
    explicit UMLUseCase(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    ~UMLUseCase();

    virtual void init();

    virtual UMLObject* clone() const;

    void saveToXMI(QXmlStreamWriter& writer);

protected:
    bool load1(QDomElement & element);
};

#endif
