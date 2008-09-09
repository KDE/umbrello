/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLOBJECT_H
#define UMLOBJECT_H

//qt includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <qdom.h>

#include "umlnamespace.h"
#include "umlpackagelist.h"

class UMLStereotype;
class UMLObject;

/**
 * This class is the non-graphical version of @ref UMLRectWidget.  These are
 * created and maintained in the class @ref UMLDoc.  This class holds all
 * the generic information needed for all UML objects.
 *
 * @short The base class for UML objects.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLObject : public QObject
{
    Q_OBJECT
public:

    /**
     * Creates a UMLObject.
     *
     * @param parent   The parent of the object.
     * @param name     The name of the object.
     * @param id       The ID of the object (optional.) If omitted
     *                 then a new ID will be assigned internally.
     */
    UMLObject(UMLObject * parent, const QString &name, Uml::IDType id = Uml::id_None);

    /**
     * Creates a UMLObject.
     *
     * @param   parent   The parent of the object.
     */
    UMLObject(UMLObject * parent);

    explicit UMLObject(const QString &name = QString() , Uml::IDType id = Uml::id_None);

    /**
     * Overloaded '==' operator
     */
    bool operator==(const UMLObject & rhs );

    /**
     * Standard deconstructor.
     */
    virtual ~UMLObject();

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const = 0;

    Uml::Object_Type getBaseType() const;

    virtual void setBaseType(Uml::Object_Type ot);

    virtual Uml::IDType getID() const;

    void setDoc(const QString &d);

    QString getDoc() const;

    Uml::Visibility getVisibility() const;

    void setVisibility(Uml::Visibility s);

     void setVisibilitycmd(Uml::Visibility s);

    void setStereotype(const QString &_name);

    void setStereotypecmd(const QString &_name);

    void setUMLStereotype(UMLStereotype *s);

    void setPackage(const QString &_name);

    void setUMLPackage(UMLPackage* pPkg);

    /**
     * Returns the classes UMLStereotype object.
     *
     * @return   Returns the classes UMLStereotype object.
     */
    const UMLStereotype * getUMLStereotype();

    QString getStereotype(bool includeAdornments = false) const;

    QString getPackage(const QString& separator = QString(),
                       bool includeRoot = false);

    UMLPackageList getPackages(bool includeRoot = false) const;

    UMLPackage* getUMLPackage();

    virtual void setID(Uml::IDType NewID);

    QString getName() const;

    virtual void setName(const QString &strName);

    void setNamecmd(const QString &strName) ;

    virtual QString getFullyQualifiedName(const QString& separator = QString(),
                                          bool includeRoot = false) const;

    bool getAbstract() const;

    void setAbstract(bool bAbstract);

    void setInPaste(bool bInPaste = true);

    virtual bool showProperties(int page = 0, bool assoc = false);

    virtual bool resolveRef();

    /**
     * This method saves the XMI attributes of each specific model class.
     * It needs to be implemented by each child class.
     * For creating the QDomElement and saving the common XMI parts,
     * it can use the save() method.
     */
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) = 0;

    virtual bool loadFromXMI( QDomElement & element );

    bool loadStereotype(QDomElement & element);

    bool getStatic() const;

    void setStatic(bool bStatic);

    virtual bool acceptAssociationType(Uml::Association_Type);

    QString getSecondaryId() const;

    void setSecondaryId(const QString& id);

    QString getSecondaryFallback() const;

    void setSecondaryFallback(const QString& id);

    QDomElement save( const QString &tag, QDomDocument & qDoc );

public slots:

    void emitModified();

signals:

    void modified();

protected:

    void init();

    void maybeSignalObjectCreated();

    virtual bool load( QDomElement& element );

    /**
     * The object's id.
     */
    Uml::IDType m_nId;

    /**
     * The object's documentation.
     */
    QString m_Doc;

    /**
     * The package the object belongs to if applicable.
     */
    UMLPackage* m_pUMLPackage;

    /**
     * The stereotype of the object if applicable.
     */
    UMLStereotype* m_pStereotype;

    /**
     * The objects name.
     */
    QString m_Name;

    /**
     * The objects type.
     */
    Uml::Object_Type m_BaseType;

    /**
     * The objects visibility.
     */
    Uml::Visibility m_Vis;

    /**
     * The state of whether the object is abstract or not.
     */
    bool m_bAbstract;

    /**
     * This attribute holds whether the UMLObject has instance scope
     * (false - the default) or classifier scope (true).
     */
    bool m_bStatic;

    /**
     * Caller sets this true when in paste operation.
     */
    bool m_bInPaste;

    /**
     * Auxiliary to maybeSignalObjectCreated().
     */
    bool m_bCreationWasSignalled;

    /**
     * Pointer to an associated object.
     * Only a few of the classes inheriting from UMLObject use this.
     * However, it needs to be here because of inheritance graph
     * disjunctness.
     */
    UMLObject* m_pSecondary;

    /**
     * xmi.id of the secondary object for intermediate use during
     * loading.  The secondary ID is resolved to the m_pSecondary
     * in the course of resolveRef() at the end of loading.
     */
    QString m_SecondaryId;

    /**
     * Last-chance backup for when m_SecondaryId is not found.
     * Used by Rose import: MDL files specify both a "quidu"
     * (which corresponds to m_SecondaryId) and the human readable
     * fully qualified target name of a reference.
     * In case the quidu is not found, the human readable name is
     * used which we store in m_SecondaryFallback.
     */
    QString m_SecondaryFallback;
};

#ifndef QT_NO_DEBUG_STREAM

/**
 * Print UML Object to kdgstream, so it can be used like
 *   kdWarn() << "This object shouldn't be here:" << illegalObject << endl;
 */
QDebug operator<< (QDebug s, const UMLObject& a);
#endif

#endif
