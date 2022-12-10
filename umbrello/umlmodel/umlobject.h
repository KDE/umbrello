/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLOBJECT_H
#define UMLOBJECT_H

#include "basictypes.h"
#include "icon_utils.h"
#include "umlpackagelist.h"

//qt includes
#include <QDomDocument>
#include <QDomElement>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QXmlStreamWriter>

class UMLActor;
class UMLArtifact;
class UMLAssociation;
class UMLAttribute;
class UMLCanvasObject;
class UMLCategory;
class UMLCheckConstraint;
class UMLClassifier;
class UMLClassifierListItem;
class UMLClassifierSet;
class UMLComponent;
class UMLDatatype;
class UMLEntity;
class UMLEntityAttribute;
class UMLEntityConstraint;
class UMLEnum;
class UMLEnumLiteral;
class UMLFolder;
class UMLForeignKeyConstraint;
class UMLInstance;
class UMLInstanceAttribute;
class UMLNode;
class UMLOperation;
class UMLPackage;
class UMLPort;
class UMLRole;
class UMLStereotype;
class UMLTemplate;
class UMLUniqueConstraint;
class UMLUseCase;
class UMLObjectPrivate;

/**
 * This class is the non-graphical version of @ref UMLWidget.  These are
 * created and maintained in the class @ref UMLDoc.  This class holds all
 * the generic information needed for all UML objects.
 *
 * @ref clone needs to be implemented by each child class.
 *
 * @ref saveToXMI saves the XMI attributes of each specific model class.
 * It needs to be implemented by each child class.
 * For creating the XMI element and saving the common XMI parts,
 * it can use the save1() method.
 *
 * @short The base class for UML objects.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLObject : public QObject
{
    Q_OBJECT
    Q_ENUMS(ObjectType)

public:
    enum ObjectType
    {
        ot_Unknown = -1,
        ot_UMLObject  = 100,
        ot_Actor,
        ot_UseCase,
        ot_Package,
        ot_Interface,
        ot_Datatype,
        ot_Enum,
        ot_Class,
        ot_Instance,
        ot_Association,
        ot_Attribute,
        ot_Operation,
        ot_EnumLiteral,
        ot_Template,
        ot_Component,
        ot_Artifact,
        ot_Node,
        ot_Stereotype,
        ot_Role,
        ot_Entity,
        ot_EntityAttribute,
        ot_Folder,
        ot_EntityConstraint,
        ot_UniqueConstraint,
        ot_ForeignKeyConstraint,
        ot_CheckConstraint,
        ot_Category,
        ot_Port,
        ot_InstanceAttribute,
        ot_SubSystem  ///< no real object type
    };

    static QString toString(ObjectType ot);
    static QString toI18nString(ObjectType t);
    static Icon_Utils::IconType toIcon(ObjectType t);

    explicit UMLObject(const UMLObject& other);
    explicit UMLObject(UMLObject* parent, const QString& name, Uml::ID::Type id = Uml::ID::None);
    explicit UMLObject(UMLObject* parent);
    explicit UMLObject(const QString& name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLObject();

    bool operator==(const UMLObject & rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    virtual void setBaseType(ObjectType ot);
    ObjectType baseType() const;
    QStringLiteral baseTypeStr() const;

    virtual void setID(Uml::ID::Type NewID);
    virtual Uml::ID::Type id() const;

    QString doc() const;
    bool hasDoc() const;
    void setDoc(const QString &d);

    void setVisibility(Uml::Visibility::Enum visibility);
    void setVisibilityCmd(Uml::Visibility::Enum visibility);
    Uml::Visibility::Enum visibility() const;

    void setStereotype(const QString &_name);
    void setStereotypeCmd(const QString &_name);
    QString stereotype(bool includeAdornments = false) const;

    void setUMLStereotype(UMLStereotype *stereo);
    UMLStereotype *umlStereotype() const;

    QStringList& tags();

    QString package(const QString& separator = QString(),
                    bool includeRoot = false) const;

    UMLPackageList packages(bool includeRoot = false) const;

    bool setUMLPackage(UMLPackage* pPkg);
    UMLPackage* umlPackage() const;

    void setUMLParent(UMLObject* parent);
    UMLObject* umlParent() const;

    virtual void setName(const QString &strName);
    virtual void setNameCmd(const QString &strName) ;
    QString name() const;

    virtual QString fullyQualifiedName(const QString& separator = QString(),
                                       bool includeRoot = false) const;

    void setAbstract(bool bAbstract);
    bool isAbstract() const;

    virtual bool showPropertiesDialog(QWidget* parent = 0);

    virtual bool resolveRef();

    virtual void saveToXMI(QXmlStreamWriter& writer);
    virtual bool loadFromXMI(QDomElement & element);

    bool loadStereotype(QDomElement & element);

    void setStatic(bool bStatic);
    bool isStatic() const;

    virtual bool acceptAssociationType(Uml::AssociationType::Enum) const;  //:TODO: check if this is really needed here

    void setSecondaryId(const QString& id);
    QString secondaryId() const;

    void setSecondaryFallback(const QString& id);
    QString secondaryFallback() const;

    void save1(QXmlStreamWriter& writer, const QString& type, const QString& tag = QString());

    friend QDebug operator<< (QDebug out, const UMLObject& obj);

    bool isUMLActor()                const { return baseType() == ot_Actor; }
    bool isUMLArtifact()             const { return baseType() == ot_Artifact; }
    bool isUMLAssociation()          const { return baseType() == ot_Association; }
    bool isUMLAttribute()            const { return baseType() == ot_Attribute; }
    //bool isUMLCanvasObject()         const { return baseType() == ot_CanvasObject; }
    bool isUMLCategory()             const { return baseType() == ot_Category; }
    bool isUMLCheckConstraint()      const { return baseType() == ot_CheckConstraint; }
    bool isUMLClassifier()           const { return baseType() == ot_Class; }
    bool isUMLComponent()            const { return baseType() == ot_Component; }
    bool isUMLDatatype()             const { return baseType() == ot_Datatype; }
    bool isUMLEntity()               const { return baseType() == ot_Entity; }
    bool isUMLEntityAttribute()      const { return baseType() == ot_EntityAttribute; }
    bool isUMLEntityConstraint()     const { return baseType() == ot_EntityConstraint; }
    bool isUMLEnum()                 const { return baseType() == ot_Enum; }
    bool isUMLEnumLiteral()          const { return baseType() == ot_EnumLiteral; }
    bool isUMLFolder()               const { return baseType() == ot_Folder; }
    bool isUMLForeignKeyConstraint() const { return baseType() == ot_ForeignKeyConstraint; }
    bool isUMLInstance()             const { return baseType() == ot_Instance; }
    bool isUMLInstanceAttribute()    const { return baseType() == ot_InstanceAttribute; }
    bool isUMLNode()                 const { return baseType() == ot_Node; }
    bool isUMLObject()               const { return baseType() == ot_UMLObject; }
    bool isUMLOperation()            const { return baseType() == ot_Operation; }
    bool isUMLPackage()              const { return baseType() == ot_Package; }
    bool isUMLPort()                 const { return baseType() == ot_Port; }
    bool isUMLRole()                 const { return baseType() == ot_Role; }
    bool isUMLStereotype()           const { return baseType() == ot_Stereotype; }
    bool isUMLTemplate()             const { return baseType() == ot_Template; }
    bool isUMLUniqueConstraint()     const { return baseType() == ot_UniqueConstraint; }
    bool isUMLUseCase()              const { return baseType() == ot_UseCase; }

    UMLActor                 * asUMLActor();
    UMLArtifact              * asUMLArtifact();
    UMLAssociation           * asUMLAssociation();
    UMLAttribute             * asUMLAttribute();
    UMLCanvasObject          * asUMLCanvasObject();
    UMLCategory              * asUMLCategory();
    UMLCheckConstraint       * asUMLCheckConstraint();
    UMLClassifier            * asUMLClassifier();
    UMLClassifierListItem    * asUMLClassifierListItem();
    UMLClassifierSet         * asUMLClassifierSet();
    UMLComponent             * asUMLComponent();
    UMLDatatype              * asUMLDatatype();
    UMLEntity                * asUMLEntity();
    UMLEntityAttribute       * asUMLEntityAttribute();
    UMLEntityConstraint      * asUMLEntityConstraint();
    UMLEnum                  * asUMLEnum();
    UMLEnumLiteral           * asUMLEnumLiteral();
    UMLFolder                * asUMLFolder();
    UMLForeignKeyConstraint  * asUMLForeignKeyConstraint();
    UMLInstance              * asUMLInstance();
    UMLInstanceAttribute     * asUMLInstanceAttribute();
    UMLNode                  * asUMLNode();
    UMLObject                * asUMLObject();
    UMLOperation             * asUMLOperation();
    UMLPackage               * asUMLPackage();
    UMLPort                  * asUMLPort();
    UMLRole                  * asUMLRole();
    UMLStereotype            * asUMLStereotype();
    UMLTemplate              * asUMLTemplate();
    UMLUniqueConstraint      * asUMLUniqueConstraint();
    UMLUseCase               * asUMLUseCase();

    const UMLActor                 * asUMLActor()                const;
    const UMLArtifact              * asUMLArtifact()             const;
    const UMLAssociation           * asUMLAssociation()          const;
    const UMLAttribute             * asUMLAttribute()            const;
    const UMLCanvasObject          * asUMLCanvasObject()         const;
    const UMLCategory              * asUMLCategory()             const;
    const UMLCheckConstraint       * asUMLCheckConstraint()      const;
    const UMLClassifier            * asUMLClassifier()           const;
    const UMLClassifierListItem    * asUMLClassifierListItem()   const;
    const UMLClassifierSet         * asUMLClassifierSet()        const;
    const UMLComponent             * asUMLComponent()            const;
    const UMLDatatype              * asUMLDatatype()             const;
    const UMLEntity                * asUMLEntity()               const;
    const UMLEntityAttribute       * asUMLEntityAttribute()      const;
    const UMLEntityConstraint      * asUMLEntityConstraint()     const;
    const UMLEnum                  * asUMLEnum()                 const;
    const UMLEnumLiteral           * asUMLEnumLiteral()          const;
    const UMLFolder                * asUMLFolder()               const;
    const UMLForeignKeyConstraint  * asUMLForeignKeyConstraint() const;
    const UMLInstance              * asUMLInstance()             const;
    const UMLInstanceAttribute     * asUMLInstanceAttribute()    const;
    const UMLNode                  * asUMLNode()                 const;
    const UMLObject                * asUMLObject()               const;
    const UMLOperation             * asUMLOperation()            const;
    const UMLPackage               * asUMLPackage()              const;
    const UMLPort                  * asUMLPort()                 const;
    const UMLRole                  * asUMLRole()                 const;
    const UMLStereotype            * asUMLStereotype()           const;
    const UMLTemplate              * asUMLTemplate()             const;
    const UMLUniqueConstraint      * asUMLUniqueConstraint()     const;
    const UMLUseCase               * asUMLUseCase()              const;

public slots:
    void emitModified();

signals:
    void modified();

protected:
    void init();

    void maybeSignalObjectCreated();

    void save1end(QXmlStreamWriter& writer);

    virtual bool load1(QDomElement& element);

    Uml::ID::Type          m_nId;          ///< object's id
    QString                m_Doc;          ///< object's documentation
    QPointer<UMLStereotype> m_pStereotype;  ///< stereotype of the object if applicable
    QString                m_name;         ///< objects name
    ObjectType             m_BaseType;     ///< objects type
    Uml::Visibility::Enum  m_visibility;   ///< objects visibility
    bool                   m_bAbstract;    ///< state of whether the object is abstract or not
    bool                   m_bStatic;      ///< flag for instance scope
    bool        m_bCreationWasSignalled;   ///< auxiliary to maybeSignalObjectCreated()
    QPointer<UMLObject>    m_pSecondary;   ///< pointer to an associated object
                                           ///< Only a few of the classes inheriting from UMLObject use this.
                                           ///< However, it needs to be here because of inheritance graph
                                           ///< disjunctness.
    QString                m_SecondaryId;  ///< xmi.id of the secondary object for intermediate use during
                                           ///< loading. The secondary ID is resolved to the m_pSecondary
                                           ///< in the course of resolveRef() at the end of loading.
    QString           m_SecondaryFallback; ///< Last-chance backup for when m_SecondaryId is not found.
                                           ///< Used by Rose import: MDL files specify both a "quidu"
                                           ///< (which corresponds to m_SecondaryId) and the human readable
                                           ///< fully qualified target name of a reference.
                                           ///< In case the quidu is not found, the human readable name is
                                           ///< used which we store in m_SecondaryFallback.
    QStringList m_TaggedValues;            ///< Concrete values of UMLStereotype::AttributeDefs if a
                                           ///< stereotype is applied and has attributes.
                                           ///< The order of elements is the same as in
                                           ///< UMLStereotype::AttributeDefs.
                                           ///< At most N_STEREOATTRS are used (see dialogs/n_stereoattrs.h)
    UMLObjectPrivate *m_d;                 ///< private data
    friend class ObjectsModel;
};

#endif
