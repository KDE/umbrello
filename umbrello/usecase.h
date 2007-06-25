/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
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

class UMLUseCase : public UMLCanvasObject {
public:
    /**
     *  Creates a UseCase object
     *
     *  @param name             The name of the object.
     *  @param  id                              The id of the object.
     */
    explicit UMLUseCase(const QString & name = "", Uml::IDType id = Uml::id_None);

    /**
     *  Standard deconstructor
     */
    ~UMLUseCase();

    /**
     *  Initializes key variables of the class.
     */
    virtual void init();

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * Creates the <UML:UseCase> element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    /**
     * Loads the <UML:UseCase> element (empty.)
     */
    bool load( QDomElement & element );
};

#endif
