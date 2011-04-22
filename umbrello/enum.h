/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ENUM_H
#define ENUM_H

#include "classifier.h"

class UMLEnumLiteral;

/**
 * This class contains the non-graphical information required for a UML
 * Enum.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
 *
 * @short Non-graphical Information for an Enum.
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLEnum : public UMLClassifier
{
    Q_OBJECT
public:
    explicit UMLEnum(const QString& name = QString(), Uml::IDType id = Uml::id_None);

    virtual ~UMLEnum();

    bool operator==(const UMLEnum& rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    UMLObject* createEnumLiteral(const QString& name = QString());

    UMLObject* addEnumLiteral(const QString &name, Uml::IDType id = Uml::id_None);

    bool addEnumLiteral(UMLEnumLiteral* literal, IDChangeLog* Log = 0);
    bool addEnumLiteral(UMLEnumLiteral* literal, int position );

    int removeEnumLiteral(UMLEnumLiteral* literal);

    int enumLiterals();

    void signalEnumLiteralRemoved(UMLClassifierListItem *elit);

    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    virtual UMLClassifierListItem* makeChildObject(const QString& xmiTag);

signals:
    void enumLiteralAdded(UMLClassifierListItem*);
    void enumLiteralRemoved(UMLClassifierListItem*);

protected:
    bool load( QDomElement & element );

private:
    void init();

};

#endif // ENUM_H

