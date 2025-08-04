/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef STEREOTYPE_H
#define STEREOTYPE_H

#include "umlobject.h"

/**
 * This class is used to set up information for a stereotype.
 * Stereotypes are used essentially as properties of
 * attributes and operations etc.
 *
 * Each stereotype object is reference counted, i.e. client code
 * manages it such that it comes into existence as soon as there is
 * at least one user, and ceases existing when the number of users
 * drops to 0.
 * m_refCount reflects the number of users. It is externally managed,
 * i.e. client code must take care to call incrRefCount() and
 * decrRefCount() as appropriate.
 *
 * The one and only owner of all stereotypes is the UMLDoc, and the
 * ownership is specially managed (umlPackage() returns nullptr for a
 * UMLStereotype.) The reason for this special treatment is that
 * class UMLDoc does not inherit from class UMLPackage, and therefore
 * setUMLPackage() cannot be used for stereotypes.
 *
 * @short Sets up stereotype information.
 * @author Jonathan Riddell
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLStereotype : public UMLObject
{
    Q_OBJECT
public:
    explicit UMLStereotype(const QString &name, Uml::ID::Type id = Uml::ID::None);
    UMLStereotype();

    virtual ~UMLStereotype();

    bool operator==(const UMLStereotype &rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    void incrRefCount();
    void decrRefCount();

    int refCount() const;

    QString name(bool includeAdornments=false) const;

    class AttributeDef
    {
    public:
        QString                   name;
        Uml::PrimitiveTypes::Enum type;
        QString                   defaultVal;
        AttributeDef()            : type(Uml::PrimitiveTypes::String) {}
        AttributeDef(QString nm, Uml::PrimitiveTypes::Enum t, QString dfltVal = QString())
                                  : name(nm), type(t), defaultVal(dfltVal) {}
        virtual ~AttributeDef() {}
    };

    typedef QVector<AttributeDef> AttributeDefs;  ///< size is at most N_STEREOATTRS

    void clearAttributeDefs();
    void setAttributeDefs(const AttributeDefs& adefs);
    const AttributeDefs& getAttributeDefs() const;
    AttributeDefs& getAttributeDefs();

    void saveToXMI(QXmlStreamWriter& writer);
    bool load1(QDomElement& element);

    virtual bool showPropertiesDialog(QWidget* parent);

protected:
    int m_refCount;
    AttributeDefs m_attrDefs;

};

Q_DECLARE_METATYPE(UMLStereotype*);

#endif
