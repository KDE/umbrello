/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLOBJECT_H
#define UMLOBJECT_H

//qt includes
#include <qobject.h>
#include <qstring.h>
#include <qdom.h>

#include "umlnamespace.h"
#include "umlpackagelist.h"

class kdbgstream;
class UMLStereotype;
class UMLObject;

/**
 * This class is the non-graphical version of @ref UMLWidget.  These are
 * created and maintained in the class @ref UMLDoc.  This class holds all
 * the generic information needed for all UML objects.
 *
 * @short The base class for UML objects.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLObject : public QObject {
    Q_OBJECT
public:

    /**
     * Creates a UMLObject.
     *
     * @param parent    The parent of the object.
     * @param name              The name of the object.
     * @param id                The ID of the object (optional.) If omitted
     *                  then a new ID will be assigned internally.
     */
    UMLObject(const UMLObject * parent, const QString &name, Uml::IDType id = Uml::id_None);

    /**
     * Creates a UMLObject.
     *
     * @param   parent          The parent of the object.
     */
    UMLObject(const UMLObject * parent);

    /**
     * Creates a UMLObject with a given name and unique ID.
     *
     * @param   name            The name of the object.
     * @param   id              The unique ID of the object.
     */
    explicit UMLObject(const QString &name = "" , Uml::IDType id = Uml::id_None);

    /**
     * Overloaded '==' operator
     */
    virtual bool operator==(UMLObject & rhs );

    /**
     * Standard deconstructor.
     */
    virtual ~UMLObject();

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto(UMLObject *rhs) const;

    /**
     * Make a clone of this object.
     * To be implemented by inheriting classes.
     */
    virtual UMLObject* clone() const = 0;

    /**
     * Returns the type of the object.
     *
     * @return  Returns the type of the object.
     */
    Uml::Object_Type getBaseType() const;

    /**
     * Set the type of the object.
     *
     * @param ot The Uml::Object_Type to set.
     */
    virtual void setBaseType(Uml::Object_Type ot);

    /**
     * Returns the ID of the object.
     *
     * @return  Returns the ID of the object.
     */
    virtual Uml::IDType getID() const;

    /**
     * Sets the documentation for the object.
     *
     * @param d The documentation for the object.
     */
    void setDoc(const QString &d);

    /**
     * Returns the documentation for the object.
     *
     * @return  Returns the documentation for the object.
     */
    QString getDoc() const;

    /**
     * Returns the visibility of the object.
     *
     * @return  Returns the visibility of the object.
     */
    Uml::Visibility getVisibility() const;

    /**
     * Sets the visibility of the object.
     *
     * @param s The visibility of the object.
     */
    virtual void setVisibility(Uml::Visibility s);

    /**
     * Sets the classes stereotype name.
     * Internally uses setUMLStereotype().
     *
     * @param _name     Sets the classes stereotype name.
     */
    void setStereotype(const QString &_name);

    /**
     * Sets the class' UMLStereotype. Adjusts the reference counts
     * at the previously set stereotype and at the new stereotype.
     * If the previously set UMLStereotype's reference count drops
     * to zero then the UMLStereotype is removed at the UMLDoc and
     * it is then physically deleted.
     *
     * @param s Sets the classes UMLStereotype.
     */
    void setUMLStereotype(UMLStereotype *s);

    /**
     * Sets the classes Package.
     * DEPRECATED - use SetUMLPackage instead.
     *
     * @param _name     The classes Package name.
     */
    void setPackage(const QString &_name);

    /**
     * Sets the UMLPackage in which this class is located.
     *
     * @param pPkg              Pointer to the class' UMLPackage.
     */
    void setUMLPackage(UMLPackage* pPkg);

    /**
     * Returns the classes UMLStereotype object.
     *
     * @return  Returns the classes UMLStereotype object.
     */
    const UMLStereotype * getUMLStereotype();

    /**
     * Returns the classes stereotype name.
     * Returns an empty string if no stereotype object is set.
     *
     * @param includeAdornments Include surrounding angled brackets
     *                          "«" and "»".
     * @return  Returns the classes stereotype name.
     */
    QString getStereotype(bool includeAdornments = false) const;

    /**
     * Return the package(s) in which this UMLObject is contained
     * as a text.
     *
     * @param separator Separator string for joining together the
     *                  individual package prefixes (optional.)
     *                  If no separator is given then the separator
     *                  of the currently selected language is used.
     * @param includeRoot  Whether to prefix the root folder name.
     *                     Default: false.
     * @return  The UMLObject's enclosing package(s) as a text.
     */
    QString getPackage(const QString& separator = QString::null,
                       bool includeRoot = false);

    /**
     * Return a list of the packages in which this class is embedded.
     * The outermost package is first in the list.
     *
     * @param includeRoot  Whether to prefix the root folder name.
     *                     Default: false.
     * @return  UMLPackageList of the containing packages.
     */
    UMLPackageList getPackages(bool includeRoot = false) const;

    /**
     * Returns the UMLPackage that this class is located in.
     *
     * @return  Pointer to the UMLPackage of this class.
     */
    UMLPackage* getUMLPackage();

    /**
     * Assigns a new Id to the object
     */
    virtual void setID(Uml::IDType NewID);

    /**
     * Returns a copy of m_Name
     */
    QString getName() const;

    /**
     * Set the UMLObject's name
     */
    virtual void setName(const QString &strName);

    /**
     * Returns the fully qualified name, i.e. all package prefixes and then m_Name.
     *
     * @param separator  The separator string to use (optional.)
     *                   If not given then the separator is chosen according
     *                   to the currently selected active programming language
     *                   of import and code generation.
     * @param includeRoot  Whether to prefix the root folder name to the FQN.
     *                     See UMLDoc::getRootFolder(). Default: false.
     * @return  The fully qualified name of this UMLObject.
     */
    virtual QString getFullyQualifiedName(const QString& separator = QString::null,
                                          bool includeRoot = false) const;

    /**
     * Returns the abstract state of the object.
     */
    bool getAbstract() const;

    /**
     * Sets the abstract state of the object.
     */
    void setAbstract(bool bAbstract);

    /**
     * Sets the abstract state of the object.
     */
    void setInPaste(bool bInPaste = true);

    /**
     * This method is called if you wish to see the properties of a
     * UMLObject.  A dialog box will be displayed from which you
     * can change the object's properties.
     *
     * @param page              The page to show.
     * @param assoc             Whether to show association page.
     * @return          True if we modified the object.
     */
    bool showProperties(int page = 0, bool assoc = false);

    /**
     * Resolve referenced objects (if any.)
     * Needs to be called after all UML objects are loaded from file.
     * This needs to be done after all model objects are loaded because
     * some of the xmi.id's might be forward references, i.e. they may
     * identify model objects which were not yet loaded at the point of
     * reference.
     * The default implementation attempts resolution of the m_SecondaryId.
     *
     * @return  True for success.
     */
    virtual bool resolveRef();

    /**
     * This method saves the XMI attributes of each specific model class.
     * It needs to be implemented by each child class.
     * For creating the QDomElement and saving the common XMI parts,
     * it can use the save() method.
     */
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) = 0;

    /**
     * This method loads the generic parts of the XMI common to most model
     * classes.  It is not usually reimplemented by child classes.
     * Instead, it invokes the load() method which implements the loading
     * of the specifics of each child class.
     *
     * @param element   The QDomElement from which to load.
     */
    virtual bool loadFromXMI( QDomElement & element );

    /**
     * Analyzes the given QDomElement for a reference to a stereotype.
     *
     * @param element    QDomElement to analyze.
     * @return        True if a stereotype reference was found, else false.
     */
    bool loadStereotype(QDomElement & element);

    /**
     * Returns true if this UMLObject has classifier scope,
     * otherwise false (the default).
     */
    bool getStatic() const;


    /**
     * Sets the value for m_bStatic.
     */
    void setStatic(bool bStatic);

    /**
     * This should be reimplemented by subclasses if they wish to
     * accept certain types of associations. Note that this only
     * tells if this UMLObject can accept the association
     * type. When creating an association another check is made to
     * see if the association is valid. For example a UMLClass
     * (UMLClassifier) can accept generalizations and should
     * return true. If while creating a generalization the
     * superclass is already subclassed from this, the association
     * is not valid and will not be created.  The default accepts
     * nothing (returns false)
     */
    virtual bool acceptAssociationType(Uml::Association_Type);

    /**
     * Return secondary ID. Required by resolveRef().
     */
    QString getSecondaryId() const;

    /**
     * Set the secondary ID.
     * Currently only required by petalTree2Uml(); all other setting of the
     * m_SecondaryID is internal to the UMLObject class hierarchy.
     */
    void setSecondaryId(const QString& id);

    /**
     * Return secondary ID fallback.
     * Required by resolveRef() for imported model files.
     */
    QString getSecondaryFallback() const;

    /**
     * Set the secondary ID fallback.
     * Currently only used by petalTree2Uml().
     */
    void setSecondaryFallback(const QString& id);

    /**
     * Auxiliary to saveToXMI.
     * Create a QDomElement with the given tag, and save the XMI attributes
     * that are common to all child classes to the newly created element.
     * This method does not need to be overridden by child classes.
     */
    QDomElement save( const QString &tag, QDomDocument & qDoc );

public slots:
    /**
     * Forces the emission of the modified signal.  Useful when
     * updating several attributes at a time: you can block the
     * signals, update all atts, and then force the signal.
     */
    void emitModified();

signals:
    /** Emitted when the UMLObject has changed. Note that some objects emit
      * this signal when one of its children changes, for example, a UMLClass
      * emits a modified() signal when one of its operation changes while the Operation
      * itself emits the corresponding signal as well.
      */
    void modified();

protected:
    /**
     * Initializes key variables of the class.
     */
    virtual void init();

    /**
     * Calls UMLDoc::signalUMLObjectCreated() if m_BaseType affords
     * doing so.
     */
    void maybeSignalObjectCreated();

    /**
     * Auxiliary to loadFromXMI.
     * This method is usually overridden by child classes.
     * It is responsible for loading the specific XMI structure
     * of the child class.
     */
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

/**
 * Print UML Object to kdgstream, so it can be used like
 *   kdWarn() << "This object shouldn't be here:" << illegalObject << endl;
 */
kdbgstream& operator<< (kdbgstream& s, const UMLObject& a);

#endif
