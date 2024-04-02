/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <brian.thomas@gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "xmlschemawriter.h"

#include "classifier.h"
#include "debug_utils.h"
#include "operation.h"
#include "umldoc.h"
#include "uml.h"  // Only needed for log{Warn,Error}

#include <KLocalizedString>
#include <KMessageBox>

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

/**
 * Constructor, initialises a couple of variables
 */
XMLSchemaWriter::XMLSchemaWriter()
{
    packageNamespaceTag = QString::fromLatin1("tns");
    packageNamespaceURI = QString::fromLatin1("http://foo.example.com/");
    schemaNamespaceTag = QString::fromLatin1("xs");
    schemaNamespaceURI = QString::fromLatin1("http://www.w3.org/2001/XMLSchema");
}

/**
 * Destructor, empty.
 */
XMLSchemaWriter::~XMLSchemaWriter()
{
}

/**
 * Returns "XMLSchema".
 */
Uml::ProgrammingLanguage::Enum XMLSchemaWriter::language() const
{
    return Uml::ProgrammingLanguage::XMLSchema;
}

/**
 * Call this method to generate XMLschema code for a UMLClassifier.
 * @param c the class to generate code for
 */
void XMLSchemaWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("XMLSchemaWriter::writeClass: Cannot write class of NULL classifier");
        return;
    }

    // find an appropriate name for our file
    QString fileName = findFileName(c,QStringLiteral(".xsd"));

    if (fileName.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    // check that we may open that file for writing
    QFile file;
    if (!openFile(file, fileName)) {
        emit codeGenerated(c, false);
        return;
    }

    QTextStream xs(&file);

    // set package namespace tag appropriately
    if (!c->package().isEmpty())
        packageNamespaceTag = c->package();

    // START WRITING

    // 0. FIRST THING: open the xml processing instruction. This MUST be
    // the first thing in the file
    xs << "<?xml version=\"1.0\"?>" << m_endl;

    // 1. create the header
    QString headerText = getHeadingFile(QStringLiteral(".xsd"));
    if (!headerText.isEmpty()) {
        headerText.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
        headerText.replace(QRegularExpression(QStringLiteral("%filepath%")), file.fileName());
    }
    if (!headerText.isEmpty())
        xs << headerText << m_endl;

    // 2. Open schema element node with appropriate namespace decl
    xs << "<" << makeSchemaTag(QStringLiteral("schema"));
    // common namespaces we know will be in the file..
    xs << " targetNamespace=\"" << packageNamespaceURI << packageNamespaceTag << "\"" << m_endl;
    xs << " xmlns:" << schemaNamespaceTag << "=\"" << schemaNamespaceURI << "\"";
    xs << " xmlns:" << packageNamespaceTag << "=\"" << packageNamespaceURI << packageNamespaceTag << "\"";

    xs << ">" << m_endl; // close opening declaration

    m_indentLevel++;

    // 3? IMPORT statements -- do we need to do anything here? I suppose if
    // our document has more than one package, which is possible, we are missing
    // the correct import statements. Leave that for later at this time.
    /*
    //only import classes in a different package as this class
    UMLPackageList imports;
    findObjectsRelated(c, imports);
    for (UMLPackage *con = imports.first(); con ; con = imports.next())
        if (con->getPackage() != c->getPackage())
                xs << "import " << con->getPackage() << "." << cleanName(con->getName()) << ";" << m_endl;
    */

    // 4. BODY of the schema.
    // start the writing by sending this classifier, the "root" for this particular
    // schema, to writeClassifier method, which will subsequently call itself on all
    // related classifiers and thus populate the schema.
    writeClassifier(c, xs);

    // 5. What remains is to make the root node declaration
    xs << m_endl;
    writeElementDecl(getElementName(c), getElementTypeName(c), xs);

    // 6. Finished: now we may close schema decl
    m_indentLevel--;
    xs << indent() << "</" << makeSchemaTag(QStringLiteral("schema")) << ">" << m_endl; // finished.. close schema node

    // tidy up. no dangling open files please..
    file.close();

    // bookkeeping for code generation
    emit codeGenerated(c, true);
    emit showGeneratedFile(file.fileName());

    // need to clear HERE, NOT in the destructor because we want each
    // schema that we write to have all related classes.
    writtenClassifiers.clear();
}

/**
 * Write an element declaration.
 */
void XMLSchemaWriter::writeElementDecl(const QString &elementName, const QString &elementTypeName, QTextStream &xs)
{
    if (forceDoc())
        writeComment(elementName + QStringLiteral(" is the root element, declared here."), xs);

    xs << indent() << "<" << makeSchemaTag(QStringLiteral("element"))
       << " name=\"" << elementName << "\""
       << " type=\"" << makePackageTag(elementTypeName) << "\""
       << "/>" << m_endl;
}

/**
 * Writes classifier's documentation then guts.
 */
void XMLSchemaWriter::writeClassifier(UMLClassifier *c, QTextStream &xs)
{
    // NO doing this 2 or more times.
    if (hasBeenWritten(c))
        return;

    xs << m_endl;

    // write documentation for class, if any, first
    if (forceDoc() || !c->doc().isEmpty())
        writeComment(c->doc(), xs);

    if (c->isAbstract() || c->isInterface())
        writeAbstractClassifier(c, xs); // if it is an interface or abstract class
    else
        writeConcreteClassifier(c, xs);
}

/**
 * Find all attributes for this classifier.
 */
UMLAttributeList XMLSchemaWriter::findAttributes(UMLClassifier *c)
{
    // sort attributes by Scope
    UMLAttributeList attribs;

    if (!c->isInterface()) {
        UMLAttributeList atl = c->getAttributeList();
        Q_FOREACH(UMLAttribute *at,  atl) {
            switch(at->visibility())
            {
              case Uml::Visibility::Public:
              case Uml::Visibility::Protected:
                attribs.append(at);
                break;
              case Uml::Visibility::Private:
                // DO NOTHING! no way to print in the schema
                break;
              default:
                break;
            }
        }
    }
    return attribs;
}

// We have to do 2 things with abstract classifiers (e.g. abstract classes and interfaces)
// which is to:
// 1) declare it as a complexType so it may be inherited (I can see an option here: to NOT write
//    this complexType declaration IF the classifier itself isnt inherited by or is inheriting
//    from anything because no other element will use this complexType).
// 2) Create a group so that elements, which obey the abstract class /interface may be placed in
//    aggregation with other elements (again, and option here to NOT write the group if no other
//    element use the interface in element aggregation)
//
void XMLSchemaWriter::writeAbstractClassifier (UMLClassifier *c, QTextStream &xs)
{
    // preparations
    UMLClassifierList subclasses = c->findSubClassConcepts(); // list of what inherits from us
    UMLClassifierList superclasses = c->findSuperClassConcepts(); // list of what we inherit from

    // write the main declaration
    writeConcreteClassifier (c, xs);
    writeGroupClassifierDecl (c, subclasses, xs);

    markAsWritten(c);

    // now go back and make sure all sub-classing nodes are declared
    if (subclasses.count() > 0)
    {
        QString elementName = getElementName(c);
        UMLAttributeList attribs = findAttributes(c);
        QStringList attribGroups = findAttributeGroups(c);

        writeAttributeGroupDecl(elementName, attribs, xs);

        // now write out inheriting classes, as needed
        Q_FOREACH(UMLClassifier * classifier, subclasses)
            writeClassifier(classifier, xs);
    }

    // write out any superclasses as needed
    Q_FOREACH(UMLClassifier *classifier, superclasses)
        writeClassifier(classifier, xs);
}

/**
 * Write a \<group\> declaration for this classifier. Used for interfaces to classes with
 * inheriting children.
 */
void XMLSchemaWriter::writeGroupClassifierDecl (UMLClassifier *c,
        UMLClassifierList subclasses,
        QTextStream &xs)
{
    // name of class, subclassing classifiers
    QString elementTypeName = getElementGroupTypeName(c);

    // start Writing node but only if it has subclasses? Nah..right now put in empty group
    xs << indent() << "<" << makeSchemaTag(QStringLiteral("group")) << " name=\"" << elementTypeName << "\">" << m_endl;
    m_indentLevel++;

    xs << indent() << "<" << makeSchemaTag(QStringLiteral("choice")) << ">" << m_endl;
    m_indentLevel++;

    Q_FOREACH(UMLClassifier *classifier, subclasses) {
        writeAssociationRoleDecl(classifier, QStringLiteral("1"), xs);
    }

    m_indentLevel--;
    xs << indent() << "</" << makeSchemaTag(QStringLiteral("choice")) << ">" << m_endl;

    m_indentLevel--;

    // finish node
    xs << indent() << "</" << makeSchemaTag(QStringLiteral("group")) << ">" << m_endl;
}

/**
 * Write a \<complexType\> declaration for this classifier.
 */
void XMLSchemaWriter::writeComplexTypeClassifierDecl (UMLClassifier *c,
        UMLAssociationList associations,
        UMLAssociationList aggregations,
        UMLAssociationList compositions,
        UMLClassifierList superclasses,
        QTextStream &xs)
{
    // Preparations
    //

    // sort attributes by Scope
    UMLAttributeList attribs = findAttributes(c);
    QStringList attribGroups = findAttributeGroups(c);

    // test for relevant associations
    bool hasAssociations = determineIfHasChildNodes(c);
    bool hasSuperclass = superclasses.count()> 0;
    bool hasAttributes = attribs.count() > 0 || attribGroups.count() > 0;

    // START WRITING

    // start body of element
    QString elementTypeName = getElementTypeName(c);

    xs << indent() << "<" << makeSchemaTag(QStringLiteral("complexType")) << " name=\"" << elementTypeName << "\"";

    if (hasAssociations || hasAttributes || hasSuperclass)
    {
        xs << ">" << m_endl;

        m_indentLevel++;

        if (hasSuperclass)
        {
            QString superClassName = getElementTypeName(superclasses.first());
            xs << indent() << "<" << makeSchemaTag(QStringLiteral("complexContent")) << ">" << m_endl;

            //PROBLEM: we only treat ONE superclass for inheritance.. bah.
            m_indentLevel++;
            xs << indent() << "<" << makeSchemaTag(QStringLiteral("extension")) << " base=\"" << makePackageTag(superClassName)
               <<"\"";
            if (hasAssociations || hasAttributes)
                xs << ">" << m_endl;
            else
                xs << "/>" << m_endl;

            m_indentLevel++;
        }

        if (hasAssociations)
        {
            // Child Elements (from most associations)
            //
            bool didFirstOne = false;
            didFirstOne = writeAssociationDecls(associations, true, didFirstOne, c->id(), xs);
            didFirstOne = writeAssociationDecls(aggregations, false, didFirstOne, c->id(), xs);
            didFirstOne = writeAssociationDecls(compositions, false, didFirstOne, c->id(), xs);

            if (didFirstOne) {
                m_indentLevel--;
                xs << indent() << "</" << makeSchemaTag(QStringLiteral("sequence")) << ">" << m_endl;
            }
        }

        // ATTRIBUTES
        //
        if (hasAttributes)
        {
            writeAttributeDecls(attribs, xs);
            for (int i= 0; i < attribGroups.count(); ++i) {
                xs << indent() << "<" << makeSchemaTag(QStringLiteral("attributeGroup")) << " ref=\""
                   << makePackageTag(attribGroups[i]) << "\"/>" << m_endl;
            }
        }

        if (hasSuperclass)
        {
            m_indentLevel--;

            if (hasAssociations || hasAttributes)
                xs << indent() << "</" << makeSchemaTag(QStringLiteral("extension")) << ">" << m_endl;

            m_indentLevel--;
            xs << indent() << "</" << makeSchemaTag(QStringLiteral("complexContent")) << ">" << m_endl;
        }

        // close this element decl
        m_indentLevel--;
        xs << indent() << "</" << makeSchemaTag(QStringLiteral("complexType")) << ">" << m_endl;

    } else
        xs << "/>" << m_endl; // empty node. just close this element decl
}

void XMLSchemaWriter::writeConcreteClassifier (UMLClassifier *c, QTextStream &xs)
{
    // preparations.. gather information about this classifier
    //
    UMLClassifierList superclasses = c->findSuperClassConcepts(); // list of what inherits from us
    UMLClassifierList subclasses = c->findSubClassConcepts(); // list of what we inherit from
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();
    // BAD! only way to get "general" associations.
    UMLAssociationList associations = c->getSpecificAssocs(Uml::AssociationType::Association);

    // write the main declaration
    writeComplexTypeClassifierDecl(c, associations, aggregations, compositions, superclasses, xs);

    markAsWritten(c);

    // Now write out any child def's
    writeChildObjsInAssociation(c, associations, xs);
    writeChildObjsInAssociation(c, aggregations, xs);
    writeChildObjsInAssociation(c, compositions, xs);

    // write out any superclasses as needed
    Q_FOREACH(UMLClassifier *classifier, superclasses)
        writeClassifier(classifier, xs);

    // write out any subclasses as needed
    Q_FOREACH(UMLClassifier *classifier, subclasses)
        writeClassifier(classifier, xs);
}

/**
 * Discover the string name of all the attribute groups (which are child nodes)
 * of this classifier (err.. element).
 * These exist for abstract classes only (which become xs:group nodes).
 */
QStringList XMLSchemaWriter::findAttributeGroups (UMLClassifier *c)
{
    // we need to look for any class we inherit from. IF these
    // have attributes, then we need to notice
    QStringList list;
    UMLClassifierList superclasses = c->findSuperClassConcepts(); // list of what inherits from us
    Q_FOREACH(UMLClassifier *classifier, superclasses)
    {
        if (classifier->isAbstract())
        {
            // only classes have attributes..
            if (!classifier->isInterface()) {
                UMLAttributeList attribs = c->getAttributeList();
                if (attribs.count() > 0)
                    list.append(getElementName(classifier) + QStringLiteral("AttribGroupType"));
            }
        }
    }
    return list;
}

/**
 * Find if the classifier would have any Child elements.
 */
bool XMLSchemaWriter::determineIfHasChildNodes(UMLClassifier *c)
{
    UMLObjectList aggList = findChildObjsInAssociations (c, c->getAggregations());
    UMLObjectList compList = findChildObjsInAssociations (c, c->getCompositions());
    UMLAssociationList associations = c->getSpecificAssocs(Uml::AssociationType::Association); // BAD! only way to get "general" associations.
    UMLObjectList assocList = findChildObjsInAssociations (c, associations);
    return aggList.count() > 0 || compList.count() > 0 || assocList.count() > 0;
}

/**
 * Find all the child objects in this association and make sure they get
 * written out (if they havent already been).
 */
void XMLSchemaWriter::writeChildObjsInAssociation (UMLClassifier *c,
        UMLAssociationList assoc,
        QTextStream &xs)
{
    UMLObjectList list = findChildObjsInAssociations (c, assoc);
    Q_FOREACH(UMLObject* obj, list) {
        UMLClassifier * thisClassifier = obj->asUMLClassifier();
        if (thisClassifier)
            writeClassifier(thisClassifier, xs);
    }
}

/**
 * Quick check to see if we have written the declaration for this classifier yet.
 */
bool XMLSchemaWriter::hasBeenWritten(UMLClassifier *c)
{
    if (writtenClassifiers.contains(c))
        return true;
    else
        return false;
}

/**
 * Mark a classifier as written, so it is not repeatedly re-declared in the schema.
 */
void XMLSchemaWriter::markAsWritten(UMLClassifier *c)
{
    writtenClassifiers.append(c);
}

/**
 * Writes the Attribute declarations.
 * @param attribs List of attributes
 * @param xs text stream
 */
void XMLSchemaWriter::writeAttributeDecls(UMLAttributeList &attribs, QTextStream &xs)
{
    Q_FOREACH(UMLAttribute* at, attribs) {
        writeAttributeDecl(at, xs);
    }
}

/**
 * Write an element attribute.
 */
void XMLSchemaWriter::writeAttributeDecl(UMLAttribute *attrib, QTextStream &xs)
{
    QString documentation = attrib->doc();
    QString typeName = fixTypeName(attrib->getTypeName());
    bool isStatic = attrib->isStatic();
    QString initialValue = fixInitialStringDeclValue(attrib->getInitialValue(), typeName);

    if (!documentation.isEmpty())
        writeComment(documentation, xs);

    xs << indent() << "<" << makeSchemaTag(QStringLiteral("attribute"))
       << " name=\"" << cleanName(attrib->name()) << "\""
       << " type=\"" << typeName << "\"";

    // default value?
    if (!initialValue.isEmpty())
    {
        // IF it is static, then we use "fixed", otherwise, we use "default" decl.
        // For the default decl, we _must_ use "optional" decl
        if (isStatic)
            xs << " use=\"required\" fixed=\"" << initialValue << "\"";
        else
            xs << " use=\"optional\" default=\"" << initialValue << "\"";
    }

    // finish decl
    xs << "/>" << m_endl;
}

/**
 * Find all attributes that  belong in group.
 */
void XMLSchemaWriter::writeAttributeGroupDecl (const QString &elementName, UMLAttributeList &attribs, QTextStream &xs)
{
    if (attribs.count()> 0) {

        // write a little documentation
        writeComment(QStringLiteral("attributes for element ") + elementName, xs);

        // open attribute group
        xs << indent() << "<" << makeSchemaTag(QStringLiteral("attributeGroup")) << " name=\"" << elementName << "AttribGroupType" << "\">" << m_endl;

        m_indentLevel++;

        Q_FOREACH(UMLAttribute *at, attribs)
        {
            writeAttributeDecl(at, xs);
        }

        m_indentLevel--;

        // close attrib group node
        xs << indent() << "</" << makeSchemaTag(QStringLiteral("attributeGroup")) << ">" << m_endl;
    }
}

/**
 * Writes a comment.
 */
void XMLSchemaWriter::writeComment(const QString &comment, QTextStream &xs)
{
    // in the case we have several line comment..
    // NOTE: this part of the method has the problem of adopting UNIX newline,
    // need to resolve for using with MAC/WinDoze eventually I assume
    QString indnt = indent();
    xs << indnt << "<!-- ";
    if (comment.contains(QRegularExpression(QStringLiteral("\n")))) {
        xs << m_endl;
        QStringList lines = comment.split(QLatin1Char('\n'));
        for (int i= 0; i < lines.count(); i++)
            xs << indnt << "     " << lines[i] << m_endl;

        xs << indnt << "-->" << m_endl;
    } else {
        // this should be more fancy in the future, breaking it up into 80 char
        // lines so that it doesn't look too bad
        xs << comment << " -->" << m_endl;
    }
}

/**
 * Searches a list of associations for appropriate ones to write out as attributes.
 * This works well for compositions, aggregations and self-associations but will
 * not work right for plain associations between 2 different classes.
 * all that matters here is roleA, the role served by the children of this class
 * in any composition or aggregation association. In full associations, I have only
 * considered the case of "self" association, so it shouldn't matter if we use role A or
 * B to find the child class as long as we don't use BOTH roles. I bet this will fail
 * badly for someone using a plain association between 2 different classes. THAT should
 * be done, but isnt yet (this is why I have left role b code in for now). -b.t.
 */
bool XMLSchemaWriter::writeAssociationDecls(UMLAssociationList associations,
        bool noRoleNameOK, bool didFirstOne, Uml::ID::Type id, QTextStream &xs)
{
    if (!associations.isEmpty())
    {
        bool printRoleA = false, printRoleB = false;

        Q_FOREACH(UMLAssociation *a, associations)
        {
            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.

            if (a->getObjectId(Uml::RoleType::A) == id && a->visibility(Uml::RoleType::B) != Uml::Visibility::Private)
                printRoleB = true;

            if (a->getObjectId(Uml::RoleType::B) == id && a->visibility(Uml::RoleType::A) != Uml::Visibility::Private)
                printRoleA = true;

            // First: we insert documentation for association IF it has either role
            // AND some documentation (!)
            if ((printRoleA || printRoleB) && !(a->doc().isEmpty()))
                writeComment(a->doc(), xs);

            // opening for sequence
            if (!didFirstOne && (printRoleA || printRoleB))
            {
                didFirstOne = true;
                xs << indent() << "<" << makeSchemaTag(QStringLiteral("sequence")) << ">" << m_endl;
                m_indentLevel++;
            }

            // print RoleB decl
            /*
            // As mentioned in the header comment for this method: this block of code is
            // commented out for now as it will only be needed if/when plain associations
            // between different classes are to be treated
            if (printRoleB)
            {
                UMLClassifier *classifierB = a->getObjectB()->asUMLClassifier();;
                if (classifierB) {
                        // ONLY write out IF there is a rolename given
                        // otherwise it is not meant to be declared
                        if (!a->getRoleNameB().isEmpty() || noRoleNameOK)
                                writeAssociationRoleDecl(classifierB, a->getMultiB(), xs);
                }
            }
            */

            // print RoleA decl
            if (printRoleA)
            {
                UMLClassifier *classifierA = a->getObject(Uml::RoleType::A)->asUMLClassifier();
                if (classifierA) {
                    // ONLY write out IF there is a rolename given
                    // otherwise it is not meant to be declared
                    if (!a->getRoleName(Uml::RoleType::A).isEmpty() || noRoleNameOK)
                        writeAssociationRoleDecl(classifierA, a->getMultiplicity(Uml::RoleType::A), xs);
                }
            }
        }

    }

    return didFirstOne;
}

/**
 * Find and return a list of child UMLObjects pointed to by the associations
 * in this UMLClassifier.
 */
UMLObjectList XMLSchemaWriter::findChildObjsInAssociations (UMLClassifier *c, UMLAssociationList associations)
{
    Uml::ID::Type id = c->id();
    UMLObjectList list;
    Q_FOREACH(UMLAssociation *a, associations)
    {
        if (a->getObjectId(Uml::RoleType::A) == id
                && a->visibility(Uml::RoleType::B) != Uml::Visibility::Private
                && !a->getRoleName(Uml::RoleType::B).isEmpty()
          )
            list.append(a->getObject(Uml::RoleType::B));

        if (a->getObjectId(Uml::RoleType::B) == id
                && a->visibility(Uml::RoleType::A) != Uml::Visibility::Private
                && !a->getRoleName(Uml::RoleType::A).isEmpty()
          )
            list.append(a->getObject(Uml::RoleType::A));
    }
    return list;
}

/**
 * Writes out an association as an attribute using Vector
 */
void XMLSchemaWriter::writeAssociationRoleDecl(UMLClassifier *c, const QString &multi, QTextStream &xs)
{
    bool isAbstract = c->isAbstract();
    bool isInterface = c->isInterface();

    QString elementName = getElementName(c);
    QString doc = c->doc();

    if (!doc.isEmpty())
        writeComment(doc, xs);

    // Min/Max Occurs is based on whether it is this a single element
    // or a List (maxoccurs>1). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    QString minOccurs = QStringLiteral("0");
    QString maxOccurs = QStringLiteral("unbounded");
    if (multi.isEmpty())
    {
        // in this case, association will only specify ONE element can exist
        // as a child
        minOccurs = QStringLiteral("1");
        maxOccurs = QStringLiteral("1");
    }
    else
    {
        QStringList values = multi.split(QRegularExpression(QStringLiteral("[^\\d{1,}|\\*]")));

        // could use some improvement here.. for sequences like "0..1, 3..5, 10" we
        // don't capture the whole "richness" of the multi. Instead we translate it
        // now to minOccurs="0" maxOccurs="10"
        if (values.count() > 0)
        {
            // populate both with the actual value as long as our value isnt an asterix
            // In that case, use special value (from above)
            if (values[0].contains(QRegularExpression(QStringLiteral("\\d{1,}"))))
                minOccurs = values[0]; // use first number in sequence

            if (values[values.count()-1].contains(QRegularExpression(QStringLiteral("\\d{1,}"))))
                maxOccurs = values[values.count()-1]; // use only last number in sequence
        }
    }

    // Now declare the class in the association as an element or group.
    //
    // In a semi-arbitrary way, we have decided to make abstract classes into
    // "groups" and concrete classes into "complexTypes".
    //
    // This is because about the only thing you can do with an abstract
    // class (err. node) is inherit from it with a "concrete" element. Therefore,
    // in this manner, we need a group node for abstract classes to lay out the child
    // element choices so that the child, concrete class may be plugged into whatever spot
    // it parent could go. The tradeoff is that "group" nodes may not be extended, so the
    // choices that you lay out here are it (e.g. no more nodes may inherit" from this group)
    //
    // The flipside for concrete classes is that we want to use them as elements in our document.
    // Unfortunately, about all you can do with complexTypes in terms of inheritance, is to
    // use these as the basis for a new node type. This is NOT full inheritance because the new
    // class (err. element node) wont be able to go into the document where it parent went without
    // you heavily editing the schema.
    //
    // Therefore, IF a group is abstract, but has no inheriting sub-classes, there are no choices, and it is nigh
    // on pointless to declare it as a group, in this special case, abstract classes get declared
    // as complexTypes.
    //
    // Of course, in OO methodology, you should be able to inherit from
    // any class, but schema just don't allow use to have full inheritance using either groups
    // or complexTypes. Thus we have taken a middle rode. If someone wants to key me into a
    // better way to represent this, I'd be happy to listen. =b.t.
    //
    // UPDATE: partial solution to the above: as of 13-Mar-2003 we now write BOTH a complexType
    //         AND a group declaration for interfaces AND classes which are inherited from.
    //
    if ((isAbstract || isInterface) && c->findSubClassConcepts().count() > 0)
        xs << indent() << "<" << makeSchemaTag(QStringLiteral("group"))
        <<" ref=\"" << makePackageTag(getElementGroupTypeName(c)) << "\"";
    else
        xs << indent() << "<" << makeSchemaTag(QStringLiteral("element"))
        <<" name=\"" << getElementName(c) << "\""
        <<" type=\"" << makePackageTag(getElementTypeName(c)) << "\"";

    // min/max occurs
    if (minOccurs != QStringLiteral("1"))
        xs << " minOccurs=\"" << minOccurs << "\"";

    if (maxOccurs != QStringLiteral("1"))
        xs << " maxOccurs=\"" << maxOccurs << "\"";

    // tidy up the node
    xs << "/>" << m_endl;
}

/**
 * Replaces `string' with `String' and `bool' with `boolean'
 * IF the type is "string" we need to declare it as
 * the XMLSchema Object "String" (there is no string primitive in XMLSchema).
 * Same thing again for "bool" to "boolean".
 */
QString XMLSchemaWriter::fixTypeName(const QString& string)
{
    //  string.replace(QRegularExpression("^string$"), schemaNamespaceTag + ":string");
    //  string.replace(QRegularExpression("^bool$"), schemaNamespaceTag + ":boolean");
    return schemaNamespaceTag + QLatin1Char(':') + string;
}

/**
 * Check that initial values of strings DON'T have quotes around them
 * (we get double quoting then)!!
 */
QString XMLSchemaWriter::fixInitialStringDeclValue(QString value, const QString &type)
{
    // check for strings only
    if (!value.isEmpty() && type == QStringLiteral("xs:string")) {
        if (!value.startsWith(QLatin1Char('"')))
            value.remove(0, 1);
        if (!value.endsWith(QLatin1Char('"')))
            value.remove(value.length(), 1);
    }
    return value;
}

/**
 * Find the element node name for this classifier.
 */
QString XMLSchemaWriter::getElementName(UMLClassifier *c)
{
    return cleanName(c->name());
}

/**
 * Find the element node "type" name. Used in the "complexType" which
 * might define that element node.
 */
QString XMLSchemaWriter::getElementTypeName(UMLClassifier *c)
{
    QString elementName = getElementName(c);
    return elementName + QStringLiteral("ComplexType");
}

/**
 * Find the group node "type" name. Used for elements which define an interface/are abstract.
 */
QString XMLSchemaWriter::getElementGroupTypeName(UMLClassifier *c)
{
    QString elementName = getElementName(c);
    return elementName + QStringLiteral("GroupType");
}

/**
 * Construct an element tag with the package namespace.
 */
QString XMLSchemaWriter::makePackageTag (QString tagName)
{
    tagName.prepend(packageNamespaceTag + QLatin1Char(':'));
    return tagName;
}

/**
 * Construct an element tag with the schema namespace.
 */
QString XMLSchemaWriter::makeSchemaTag (QString tagName)
{
    tagName.prepend(schemaNamespaceTag + QLatin1Char(':'));
    return tagName;
}

/**
 * Get list of reserved keywords.
 */
QStringList XMLSchemaWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords
          << QStringLiteral("ATTLIST")
          << QStringLiteral("CDATA")
          << QStringLiteral("DOCTYPE")
          << QStringLiteral("ELEMENT")
          << QStringLiteral("ENTITIES")
          << QStringLiteral("ENTITY")
          << QStringLiteral("ID")
          << QStringLiteral("IDREF")
          << QStringLiteral("IDREFS")
          << QStringLiteral("NMTOKEN")
          << QStringLiteral("NMTOKENS")
          << QStringLiteral("NOTATION")
          << QStringLiteral("PUBLIC")
          << QStringLiteral("SHORTREF")
          << QStringLiteral("SYSTEM")
          << QStringLiteral("USEMAP");
    }

    return keywords;
}

