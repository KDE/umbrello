/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
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
 * This class is the non-graphical version of @ref UMLWidget.  These are
 * created and maintained in the class @ref UMLDoc.  This class holds all
 * the generic information needed for all UML objects.
 *
 * @ref clone needs to be implemented by each child class.
 *
 * @ref saveToXMI saves the XMI attributes of each specific model class.
 * It needs to be implemented by each child class.
 * For creating the QDomElement and saving the common XMI parts,
 * it can use the save() method.
 *
 * @short The base class for UML objects.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLObject : public QObject
{
    Q_OBJECT
public:

    explicit UMLObject(UMLObject * parent, const QString &name, Uml::IDType id = Uml::id_None);
    explicit UMLObject(UMLObject * parent);
    explicit UMLObject(const QString &name = QString() , Uml::IDType id = Uml::id_None);
    virtual ~UMLObject();

    bool operator==(const UMLObject & rhs );

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const = 0;

    virtual void setBaseType(Uml::Object_Type ot);
    Uml::Object_Type getBaseType() const;

    virtual void setID(Uml::IDType NewID);
    virtual Uml::IDType getID() const;

    void setDoc(const QString &d);
    QString getDoc() const;

    void setVisibility(Uml::Visibility s);
    void setVisibilitycmd(Uml::Visibility s);
    Uml::Visibility getVisibility() const;

    void setStereotype(const QString &_name);
    void setStereotypecmd(const QString &_name);
    QString getStereotype(bool includeAdornments = false) const;

    void setUMLStereotype(UMLStereotype *stereo);
    const UMLStereotype * getUMLStereotype();

    void setPackage(const QString &_name);
    QString getPackage(const QString& separator = QString(),
                       bool includeRoot = false);

    void setUMLPackage(UMLPackage* pPkg);
    UMLPackage* getUMLPackage();

    UMLPackageList getPackages(bool includeRoot = false) const;

    virtual void setName(const QString &strName);
    void setNamecmd(const QString &strName) ;
    QString getName() const;

    virtual QString getFullyQualifiedName(const QString& separator = QString(),
                                          bool includeRoot = false) const;

    void setAbstract(bool bAbstract);
    bool getAbstract() const;

    void setInPaste(bool bInPaste = true);

    virtual bool showProperties(int page = 0, bool assoc = false);

    virtual bool resolveRef();

    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) = 0;
    virtual bool loadFromXMI( QDomElement & element );

    bool loadStereotype(QDomElement & element);

    void setStatic(bool bStatic);
    bool getStatic() const;

    virtual bool acceptAssociationType(Uml::Association_Type);

    void setSecondaryId(const QString& id);
    QString getSecondaryId() const;

    void setSecondaryFallback(const QString& id);
    QString getSecondaryFallback() const;

    QDomElement save( const QString &tag, QDomDocument & qDoc );

public slots:

    void emitModified();

signals:

    void modified();

protected:

    void init();

    void maybeSignalObjectCreated();

    virtual bool load( QDomElement& element );

    Uml::IDType      m_nId;          ///< The object's id.
    QString          m_Doc;          ///< The object's documentation. 
    UMLPackage*      m_pUMLPackage;  ///< The package the object belongs to if applicable.
    UMLStereotype*   m_pStereotype;  ///< The stereotype of the object if applicable.
    QString          m_Name;         ///< The objects name.
    Uml::Object_Type m_BaseType;     ///< The objects type.
    Uml::Visibility  m_Vis;          ///< The objects visibility.
    bool             m_bAbstract;    ///< The state of whether the object is abstract or not.
    bool             m_bStatic;      ///< Flag for instance scope.
    bool             m_bInPaste;     ///< Caller sets this true when in paste operation.
    bool  m_bCreationWasSignalled;   ///< Auxiliary to maybeSignalObjectCreated().

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
