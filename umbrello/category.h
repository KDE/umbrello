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

#ifndef CATEGORY_H
#define CATEGORY_H

#include "umlcanvasobject.h"

/**
 * This class contains the non-graphical information required for a UML Category.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 * The @ref UMLDoc class creates instances of this type.
 *
 * @short Information for a non-graphical UML Category.
 * @author Sharan Rao
 * @see UMLCanvasObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLCategory : public UMLCanvasObject {
    Q_OBJECT
public:

    enum Category_Type {
        ct_Disjoint_Specialisation,
        ct_Overlapping_Specialisation,
        ct_Union
    };

    explicit UMLCategory(const QString & name = QString(), Uml::IDType id = Uml::id_None);

    /**
     * Standard deconstructor.
     */
    ~UMLCategory();

    virtual void init();

    void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    UMLCategory::Category_Type getType();

    void setType(Category_Type type);

protected:

    bool load( QDomElement & element );

private:

    Category_Type m_CategoryType;
};

#endif
