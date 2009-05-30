/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef USECASE_H
#define USECASE_H

#include "umlcanvasobject.h"

/**
 * This class contains the non-graphical information required for a UML UseCase.
 * This class inherits from @ref UMLCanvasObject which contains most of the information.
 *
 * @short Information for a non-graphical UML UseCase.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLUseCase : public UMLCanvasObject
{
public:
    explicit UMLUseCase(const QString & name = QString(), Uml::IDType id = Uml::id_None);
    ~UMLUseCase();

    virtual void init();

    virtual UMLObject* clone() const;

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    bool load( QDomElement & element );
};

#endif
