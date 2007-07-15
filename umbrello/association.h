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

#ifndef ASSOCIATION_H
#define ASSOCIATION_H

#include <qdom.h>
#include "umlnamespace.h"
#include "umlobject.h"

class UMLRole;

/**
 * This class contains the non-graphic representation of an association.
 * An association can be a generalization, realization, simple association,
 * directed association, aggregation, or composition.
 *
 * @short Sets up association information.
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * @see UMLObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLAssociation : public UMLObject {
    Q_OBJECT
    friend class AssociationWidget;
public:
    /**
     * Sets up an association.
     * A new unique ID is assigned internally.
     *
     * @param type              The Uml::Association_Type to construct.
     * @param roleA             Pointer to the UMLObject in role A.
     * @param roleB             Pointer to the UMLObject in role B.
     */
    UMLAssociation(Uml::Association_Type type, UMLObject *roleA, UMLObject *roleB);

    /**
     * Constructs an association - for loading only.
     * This constructor should not normally be used as it constructs
     * an incomplete association (i.e. the role objects are missing.)
     *
     * @param type              The Uml::Association_Type to construct.
     *                  Default: Uml::at_Unknown.
     */
    UMLAssociation(Uml::Association_Type type = Uml::at_Unknown);

    /**
     * Overloaded '==' operator
     */
    bool operator==(UMLAssociation &rhs);

    /**
     * Standard deconstructor.
     */
    virtual ~UMLAssociation();

    /**
     * Returns a String representation of this UMLAssociation.
     */
    QString toString( ) const;

    /**
     * Converts a Uml::Association_Type to its string representation.
     *
     * @param atype             The Association_Type enum value to convert.
     * @return  The string representation of the Association_Type.
     */
    static QString typeAsString(Uml::Association_Type atype);

    /**
     * Returns true if the given Association_Type has a representation as a
     * UMLAssociation.
     */
    static bool assocTypeHasUMLRepresentation(Uml::Association_Type atype);

    /**
     * Returns the Association_Type of the UMLAssociation.
     *
     * @return  The Association_Type of the UMLAssociation.
     */
    Uml::Association_Type getAssocType() const;

    /**
     * Returns the UMLObject assigned to the given role.
     *
     * @return  Pointer to the UMLObject in the given role.
     */
    UMLObject* getObject(Uml::Role_Type role);

    /**
     * Returns the ID of the UMLObject assigned to the given role.
     * Shorthand for getObject(role)->getID().
     *
     * @return  ID of the UMLObject in the given role.
     */
    Uml::IDType getObjectId(Uml::Role_Type role);

    /*
     * Returns the ID of the UMLObject assigned to the given role.
     * CURRENTLY UNUSED.
     *
     * @return  ID of the UMLObject of the given role.
    Uml::IDType getRoleId(Uml::Role_Type role) const;
     */

    /**
     * Returns the Changeablity of the given role.
     *
     * @return  Changeability_Type of the given role.
     */
    Uml::Changeability_Type getChangeability(Uml::Role_Type role) const;

    /**
     * Returns the Visibility of the given role.
     *
     * @return  Visibility of the given role.
     */
    Uml::Visibility getVisibility(Uml::Role_Type role) const;

    /**
     * Returns the multiplicity assigned to the given role.
     *
     * @return  The multiplicity assigned to the given role.
     */
    QString getMulti(Uml::Role_Type role) const;

    /**
     * Returns the name assigned to the role A.
     *
     * @return  The name assigned to the given role.
     */
    QString getRoleName(Uml::Role_Type role) const;

    /**
     * Returns the documentation assigned to the given role.
     *
     * @return  Documentation text of given role.
     */
    QString getRoleDoc(Uml::Role_Type role) const;

    /**
     * Sets the assocType of the UMLAssociation.
     *
     * @param assocType The Association_Type of the UMLAssociation.
     */
    void setAssocType(Uml::Association_Type assocType);

    /**
     * Sets the UMLObject playing the given role in the association.
     *
     * @param obj  Pointer to the UMLObject of the given role.
     * @param role The Uml::Role_Type played by the association
     */
    void setObject(UMLObject *obj, Uml::Role_Type role);

    /**
     * Sets the visibility of the given role of the UMLAssociation.
     *
     * @param value  Visibility of role.
     * @param role   The Uml::Role_Type to which the visibility is being applied 
     */
    void setVisibility(Uml::Visibility value, Uml::Role_Type role);

    /**
     * Sets the changeability of the given role of the UMLAssociation.
     *
     * @param value     Changeability_Type of the given role.
     * @param role      The Uml::Role_Type to which the changeability is being set
     */
    void setChangeability(Uml::Changeability_Type value, Uml::Role_Type role);

    /**
     * Sets the multiplicity of the given role of the UMLAssociation.
     *
     * @param multi    The multiplicity of the given role.
     * @param role     The Uml::Role_Type to which the multiplicity is being applied
     */
    void setMulti(const QString &multi, Uml::Role_Type role);

    /**
     * Sets the name of the given role of the UMLAssociation.
     *
     * @param roleName  The name to set for the given role.
     * @param role      The Uml::Role_Type for which to set the name.
     */
    void setRoleName(const QString &roleName, Uml::Role_Type role);

    /**
     * Sets the documentation on the given role in the association.
     *
     * @param doc      The string with the documentation.
     * @param role     The Uml::Role_Type to which the documentation is being applied         
     */
    void setRoleDoc(const QString &doc, Uml::Role_Type role);

    /**
     * Convert Changeability_Type value into QString representation.
     *
     * @param type              The Changeability_Type enum value to convert.
     */
    static QString ChangeabilityToString(Uml::Changeability_Type type);

    /**
     * Get the underlying UMLRole object for the given role.
     *
     * @return  Pointer to the UMLRole object for the given role.
     */
    UMLRole * getUMLRole(Uml::Role_Type role);

    /**
     * Set backward compatibility flag for loading version 1.3.x files.
     * This flag is necessary because the handling of the isNavigable
     * attribute of <UML:AssociationEnd> was incorrect.
     */
    void setOldLoadMode(bool value = true);

    /**
     * Return the backward compatibility flag for loading files.
     */
    bool getOldLoadMode() const;

    /**
     * Make a clone of this object.
     * Cloning associations is not supported yet.
     */
    virtual UMLObject* clone() const { return NULL; }

    /**
     * Resolve types. Required when dealing with foreign XMI files.
     * Needs to be called after all UML objects are loaded from file.
     * Overrides the method from UMLObject.
     * Calls resolveRef() for each role.
     *
     * @return  True for success.
     */
    virtual bool resolveRef();

    /**
     * Creates the <UML:Generalization> or <UML:Association> XMI element
     * including its role objects.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    /**
     * Creates the <UML:Generalization> or <UML:Association> XMI element
     * including its role objects.
     */
    bool load(QDomElement& element);

    // keep track of number of parent widgets
    int nrof_parent_widgets;

    /**
     * Common initializations at construction time.
     *
     * @param type              The Association_Type to represent.
     * @param roleAObj  Pointer to the role A UMLObject.
     * @param roleBObj  Pointer to the role B UMLObject.
     */
    void init(Uml::Association_Type type, UMLObject *roleAObj, UMLObject *roleBObj);

    /* If the type Uml::Association_Type is changed then also the following
       must be changed accordingly:
       atypeFirst, atypeLast, assocTypeStr[], toAssocType(), toString().
       The ordering within assocTypeStr must match the enumeration
       order of Uml::Association_Type.
     */
    static const Uml::Association_Type atypeFirst;
    static const Uml::Association_Type atypeLast;
    static const unsigned int nAssocTypes;
    static const QString assocTypeStr[];

    UMLRole * m_pRole[2];

    Uml::Association_Type m_AssocType;
    QString m_Name;
    bool m_bOldLoadMode;
};

#endif
