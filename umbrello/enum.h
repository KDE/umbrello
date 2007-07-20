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
class UMLEnum : public UMLClassifier {
    Q_OBJECT
public:
    /**
     * Sets up an enum.
     *
     * @param name  The name of the Enum.
     * @param id  The unique id of the Enum.
     */
    explicit UMLEnum(const QString& name = "", Uml::IDType id = Uml::id_None);

    /**
     * Standard deconstructor.
     */
    virtual ~UMLEnum();

    /**
     * Overloaded '==' operator.
     */
    bool operator==(UMLEnum& rhs);

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto(UMLEnum *rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * Creates a literal for the enum.
     *
     * @return  The UMLEnum created
     */
    UMLObject* createEnumLiteral(const QString& name = QString());

    /**
     * Adds an enumliteral to the enum.
     *
     * @param name  The name of the enumliteral.
     * @param id  The id of the enumliteral (optional.)
     *            If omitted a new ID is assigned internally.
     * @return  Pointer to the UMLEnumliteral created.
     */
    UMLObject* addEnumLiteral(const QString &name, Uml::IDType id = Uml::id_None);

    /**
     * Adds an already created enumliteral.
     * The enumliteral object must not belong to any other concept.
     *
     * @param Att  Pointer to the UMLEnumLiteral.
     * @param Log  Pointer to the IDChangeLog.
     * @return  True if the enumliteral was successfully added.
     */
    bool addEnumLiteral(UMLEnumLiteral* Att, IDChangeLog* Log = 0);

    /**
     * Adds an enumliteral to the enum, at the given position.
     * If position is negative or too large, the enumliteral is added
     * to the end of the list.
     *
     * @param Att    Pointer to the UMLEnumLiteral.
     * @param position  Position index for the insertion.
     * @return  True if the enumliteral was successfully added.
     */
    //TODO:  give default value -1 to position (append) - now it conflicts with the method above..
    bool addEnumLiteral(UMLEnumLiteral* Att, int position );

    /**
     * Removes an enumliteral from the class.
     *
     * @param a  The enumliteral to remove.
     * @return  Count of the remaining enumliterals after removal.
     *          Returns -1 if the given enumliteral was not found.
     */
    int removeEnumLiteral(UMLEnumLiteral *a);

    /**
     * Returns the number of enumliterals for the class.
     *
     * @return  The number of enumliterals for the class.
     */
    int enumLiterals();

    /**
     * Emit the enumLiteralRemoved signal.
     */
    void signalEnumLiteralRemoved(UMLClassifierListItem *elit);

    /**
     * Creates the <UML:Enum> element including its enumliterals.
     */
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

signals:
    void enumLiteralAdded(UMLClassifierListItem*);
    void enumLiteralRemoved(UMLClassifierListItem*);

protected:
    /**
     * Loads the <UML:Enum> element including its enumliterals.
     */
    bool load( QDomElement & element );

private:
    /**
     * Initializes key variables of the class.
     */
    void init();

};

#endif // ENUM_H

