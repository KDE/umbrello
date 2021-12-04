/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cppheadercodedocument.h"

// local includes
#include "cppcodegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "cppcodedocumentation.h"
#include "cppheadercodeaccessormethod.h"
#include "cppheadercodeoperation.h"
#include "cppheaderclassdeclarationblock.h"
#include "cppheadercodeclassfielddeclarationblock.h"
#include "debug_utils.h"
#include "umlpackagelist.h"
#include "package.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include "enum.h"
#include "uml.h"

// qt includes
#include <QRegExp>

/**
 * Constructor.
 */
CPPHeaderCodeDocument::CPPHeaderCodeDocument(UMLClassifier* concept)
  : ClassifierCodeDocument(concept)
{
    setFileExtension(QLatin1String(".h"));

    //initCodeClassFields(); // this is dubious because it calls down to
                             // CodeGenFactory::newCodeClassField(this)
                             // but "this" is still in construction at that time.

    m_classDeclCodeBlock = 0;
    m_publicBlock = 0;
    m_protectedBlock = 0;
    m_privateBlock = 0;
    m_namespaceBlock = 0;
    m_pubConstructorBlock = 0;
    m_protConstructorBlock = 0;
    m_privConstructorBlock = 0;
    m_pubOperationsBlock = 0;
    m_privOperationsBlock = 0;
    m_protOperationsBlock = 0;

    // this will call updateContent() as well as other things that sync our document.
    //synchronize();
}

/**
 * Destructor.
 */
CPPHeaderCodeDocument::~CPPHeaderCodeDocument()
{
    resetTextBlocks();
}

CPPHeaderClassDeclarationBlock * CPPHeaderCodeDocument::getClassDecl()
{
    if(!m_classDeclCodeBlock) {
        m_classDeclCodeBlock = new CPPHeaderClassDeclarationBlock (this); // was deleted before our load
        m_classDeclCodeBlock->updateContent();
        m_classDeclCodeBlock->setTag(QLatin1String("classDeclarationBlock"));
    }
    return m_classDeclCodeBlock;
}

// Sigh. NOT optimal. The only reason that we need to have this
// is so we can create the CPPHeaderClassDeclarationBlock.
// would be better if we could create a handler interface that each
// codeblock used so all we have to do here is add the handler
void CPPHeaderCodeDocument::loadChildTextBlocksFromNode (QDomElement & root)
{
    QDomNode tnode = root.firstChild();
    QDomElement telement = tnode.toElement();
    bool loadCheckForChildrenOK = false;
    while(!telement.isNull()) {
        QString nodeName = telement.tagName();

        if(nodeName == QLatin1String("textblocks")) {

            QDomNode node = telement.firstChild();
            QDomElement element = node.toElement();

            // if there is nothing to begin with, then we don't worry about it
            loadCheckForChildrenOK = element.isNull() ? true : false;

            while(!element.isNull()) {
                QString name = element.tagName();

                if(name == QLatin1String("codecomment")) {
                    CodeComment * block = new CPPCodeDocumentation(this);
                    block->loadFromXMI1(element);
                    if(!addTextBlock(block))
                    {
                        uError()<<"Unable to add codeComment to :"<<this;
                        delete block;
                    } else
                        loadCheckForChildrenOK= true;
                } else
                    if(name == QLatin1String("codeaccessormethod") ||
                            name == QLatin1String("ccfdeclarationcodeblock")) {
                        QString acctag = element.attribute(QLatin1String("tag"));
                        // search for our method in the
                        TextBlock * tb = findCodeClassFieldTextBlockByTag(acctag);
                        if(!tb || !addTextBlock(tb))
                        {
                            uError()<<"Unable to add codeclassfield child method to:"<<this;
                            // DON'T delete
                        } else
                            loadCheckForChildrenOK= true;

                    } else
                        if(name == QLatin1String("codeblock")) {
                            CodeBlock * block = newCodeBlock();
                            block->loadFromXMI1(element);
                            if(!addTextBlock(block))
                            {
                                uError()<<"Unable to add codeBlock to :"<<this;
                                delete block;
                            } else
                                loadCheckForChildrenOK= true;
                        } else
                            if(name == QLatin1String("codeblockwithcomments")) {
                                CodeBlockWithComments * block = newCodeBlockWithComments();
                                block->loadFromXMI1(element);
                                if(!addTextBlock(block))
                                {
                                    uError()<<"Unable to add codeBlockwithcomments to:"<<this;
                                    delete block;
                                } else
                                    loadCheckForChildrenOK= true;
                            } else
                                if(name == QLatin1String("header")) {
                                    // do nothing.. this is treated elsewhere
                                } else
                                    if(name == QLatin1String("hierarchicalcodeblock")) {
                                        HierarchicalCodeBlock * block = newHierarchicalCodeBlock();
                                        block->loadFromXMI1(element);
                                        if(!addTextBlock(block))
                                        {
                                            uError()<<"Unable to add hierarchicalcodeBlock to:"<<this;
                                            delete block;
                                        } else
                                            loadCheckForChildrenOK= true;
                                    } else
                                        if(name == QLatin1String("codeoperation")) {
                                            // find the code operation by id
                                            QString id = element.attribute(QLatin1String("parent_id"),QLatin1String("-1"));
                                            UMLObject * obj = UMLApp::app()->document()->findObjectById(Uml::ID::fromString(id));
                                            UMLOperation * op = obj->asUMLOperation();
                                            if(op) {
                                                CodeOperation * block = new CPPHeaderCodeOperation(this, op);
                                                block->updateMethodDeclaration();
                                                block->updateContent();
                                                block->loadFromXMI1(element);
                                                if(addTextBlock(block))
                                                    loadCheckForChildrenOK= true;
                                                else
                                                {
                                                    uError()<<"Unable to add codeoperation to:"<<this;
                                                    block->deleteLater();
                                                }
                                            } else
                                                uError()<<"Unable to find operation create codeoperation for:"<<this;
                                        }
                                        else
                                            if(name == QLatin1String("cppheaderclassdeclarationblock"))
                                            {
                                                CPPHeaderClassDeclarationBlock * block = getClassDecl();
                                                block->loadFromXMI1(element);
                                                // normally this would be populated by the following syncToparent
                                                // call, but we cant wait for it, so lets just do it now.
                                                m_namespaceBlock = getHierarchicalCodeBlock(QLatin1String("namespace"), QLatin1String("Namespace"), 0);

                                                if(!m_namespaceBlock || !m_namespaceBlock->addTextBlock(block))
                                                {
                                                    uError()<<"Error:cant add class declaration codeblock";
                                                    // DON'T delete/release block
                                                    // block->release();
                                                } else
                                                    loadCheckForChildrenOK= true;

                                            }
                // only needed for extreme debugging conditions (E.g. making new codeclassdocument loader)
                //else
                //uDebug()<<" LoadFromXMI: Got strange tag in text block stack:"<<name<<", ignoring";

                node = element.nextSibling();
                element = node.toElement();
            }
            break;
        }

        tnode = telement.nextSibling();
        telement = tnode.toElement();
    }

    if(!loadCheckForChildrenOK)
    {
        uWarning() << "loadChildBlocks : unable to initialize any child blocks in doc: " << getFileName() << " " << this;
    }

}

void CPPHeaderCodeDocument::resetTextBlocks()
{
    // all special pointers need to be zero'd out.
    if (m_classDeclCodeBlock) {
        delete m_classDeclCodeBlock;
        m_classDeclCodeBlock = 0;
    }
    if (m_publicBlock) {
        delete m_publicBlock;
        m_publicBlock = 0;
    }
    if (m_protectedBlock) {
        delete m_protectedBlock;
        m_protectedBlock = 0;
    }
    if (m_privateBlock) {
        delete m_privateBlock;
        m_privateBlock = 0;
    }
    if (m_namespaceBlock) {
        delete m_namespaceBlock;
        m_namespaceBlock = 0;
    }
    if (m_pubConstructorBlock) {
        delete m_pubConstructorBlock;
        m_pubConstructorBlock = 0;
    }
    if (m_protConstructorBlock) {
        delete m_protConstructorBlock;
        m_protConstructorBlock = 0;
    }
    if (m_privConstructorBlock) {
        delete m_privConstructorBlock;
        m_privConstructorBlock = 0;
    }
    if (m_pubOperationsBlock) {
        delete m_pubOperationsBlock;
        m_pubOperationsBlock = 0;
    }
    if (m_privOperationsBlock) {
        delete m_privOperationsBlock;
        m_privOperationsBlock = 0;
    }
    if (m_protOperationsBlock) {
        delete m_protOperationsBlock;
        m_protOperationsBlock = 0;
    }

    // now do the traditional release of child text blocks
    ClassifierCodeDocument::resetTextBlocks();
}

/**
 * Add a code operation to this cpp classifier code document.
 * In the vanilla version, we just tack all operations on the end
 * of the document.
 * @param op   the code operation
 * @return bool which is true IF the code operation was added successfully
 */
bool CPPHeaderCodeDocument::addCodeOperation(CodeOperation* op)
{
    if (op == 0) {
        uDebug() << "CodeOperation is null!";
        return false;;
    }
    Uml::Visibility::Enum scope = op->getParentOperation()->visibility();
    if(!op->getParentOperation()->isLifeOperation())
    {
        switch (scope) {
        default:
        case Uml::Visibility::Public:
            return (m_pubOperationsBlock == 0 ? false : m_pubOperationsBlock->addTextBlock(op));
            break;
        case Uml::Visibility::Protected:
            return (m_protOperationsBlock == 0 ? false : m_protOperationsBlock->addTextBlock(op));
            break;
        case Uml::Visibility::Private:
            return (m_privOperationsBlock == 0 ? false : m_privOperationsBlock->addTextBlock(op));
            break;
        }
    } else {
        switch (scope) {
        default:
        case Uml::Visibility::Public:
            return (m_pubConstructorBlock == 0 ? false : m_pubConstructorBlock->addTextBlock(op));
            break;
        case Uml::Visibility::Protected:
            return (m_protConstructorBlock == 0 ? false : m_protConstructorBlock->addTextBlock(op));
            break;
        case Uml::Visibility::Private:
            return (m_privConstructorBlock == 0 ? false : m_privConstructorBlock->addTextBlock(op));
            break;
        }
    }
}

/**
 * Save the XMI representation of this object
 * @return      bool    status of save
 */
/*
void CPPHeaderCodeDocument::saveToXMI1(QXmlStreamWriter& writer)
{
        writer.writeEmptyStartElement();

        setAttributesOnNode(writer);

        writer.writeEndElement();
}
*/

// This method will cause the class to rebuild its text representation.
// based on the parent classifier object.
// For any situation in which this is called, we are either building the code
// document up, or replacing/regenerating the existing auto-generated parts. As
// such, we will want to insert everything we reasonably will want
// during creation. We can set various parts of the document (esp. the
// comments) to appear or not, as needed.
void CPPHeaderCodeDocument::updateContent()
{
    // Gather info on the various fields and parent objects of this class...
    UMLClassifier * c = getParentClassifier();
    Q_ASSERT(c != 0);
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);

    // first, set the global flag on whether or not to show classfield info
    const CodeClassFieldList * cfList = getCodeClassFieldList();
    CodeClassFieldList::const_iterator it = cfList->begin();
    CodeClassFieldList::const_iterator end = cfList->end();
    for(; it != end; ++it)
        (*it)->setWriteOutMethods(policy->getAutoGenerateAccessors());

    // attribute-based ClassFields
    // we do it this way to have the static fields sorted out from regular ones
    CodeClassFieldList staticPublicAttribClassFields = getSpecificClassFields (CodeClassField::Attribute, true, Uml::Visibility::Public);
    CodeClassFieldList publicAttribClassFields = getSpecificClassFields (CodeClassField::Attribute, false, Uml::Visibility::Public);
    CodeClassFieldList staticProtectedAttribClassFields = getSpecificClassFields (CodeClassField::Attribute, true, Uml::Visibility::Protected);
    CodeClassFieldList protectedAttribClassFields = getSpecificClassFields (CodeClassField::Attribute, false, Uml::Visibility::Protected);
    CodeClassFieldList staticPrivateAttribClassFields = getSpecificClassFields (CodeClassField::Attribute, true, Uml::Visibility::Private);
    CodeClassFieldList privateAttribClassFields = getSpecificClassFields (CodeClassField::Attribute, false, Uml::Visibility::Private);

    // association-based ClassFields
    // don't care if they are static or not..all are lumped together
    CodeClassFieldList publicPlainAssocClassFields = getSpecificClassFields (CodeClassField::PlainAssociation, Uml::Visibility::Public);
    CodeClassFieldList publicAggregationClassFields = getSpecificClassFields (CodeClassField::Aggregation, Uml::Visibility::Public);
    CodeClassFieldList publicCompositionClassFields = getSpecificClassFields (CodeClassField::Composition, Uml::Visibility::Public);

    CodeClassFieldList protPlainAssocClassFields = getSpecificClassFields (CodeClassField::PlainAssociation, Uml::Visibility::Protected);
    CodeClassFieldList protAggregationClassFields = getSpecificClassFields (CodeClassField::Aggregation, Uml::Visibility::Protected);
    CodeClassFieldList protCompositionClassFields = getSpecificClassFields (CodeClassField::Composition, Uml::Visibility::Protected);

    CodeClassFieldList privPlainAssocClassFields = getSpecificClassFields (CodeClassField::PlainAssociation, Uml::Visibility::Private);
    CodeClassFieldList privAggregationClassFields = getSpecificClassFields (CodeClassField::Aggregation, Uml::Visibility::Private);
    CodeClassFieldList privCompositionClassFields = getSpecificClassFields (CodeClassField::Composition, Uml::Visibility::Private);

    bool hasOperationMethods = false;
    UMLOperationList list = c->getOpList();
    hasOperationMethods = ! list.isEmpty();

    bool hasNamespace = false;
    bool isEnumeration = false;
    bool isInterface = parentIsInterface();
    bool hasclassFields = hasClassFields();
    bool forcedoc = UMLApp::app()->commonPolicy()->getCodeVerboseDocumentComments();
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    UMLClassifierList superclasses = c->findSuperClassConcepts();


    // START GENERATING CODE/TEXT BLOCKS and COMMENTS FOR THE DOCUMENT
    //

    // Write the hash define stuff to prevent multiple parsing/inclusion of header
    QString cppClassName = CodeGenerator::cleanName(c->name());
    QString hashDefine = CodeGenerator::cleanName(c->name().toUpper().simplified());
    QString defText = QLatin1String("#ifndef ") + hashDefine + QLatin1String("_H") + endLine + QLatin1String("#define ") + hashDefine + QLatin1String("_H");
    addOrUpdateTaggedCodeBlockWithComments(QLatin1String("hashDefBlock"), defText, QString(), 0, false);

    // INCLUDE CODEBLOCK
    //
    // Q: Why all utils? Isnt just List and Vector the only classes we are using?
    // A: doesn't matter at all; its more readable to just include '*' and cpp compilers
    //    don't slow down or anything. (TZ)
    QString includeStatement;
    bool stringGlobal = policy->stringIncludeIsGlobal();
    QString sStartBrak = stringGlobal ? QLatin1String("<") : QLatin1String("\"");
    QString sEndBrak = stringGlobal ? QLatin1String(">") : QLatin1String("\"");
    includeStatement.append(QLatin1String("#include ") + sStartBrak + policy->getStringClassNameInclude() + sEndBrak + endLine);
    if (hasObjectVectorClassFields())
    {
        bool vecGlobal = policy->vectorIncludeIsGlobal();
        QString vStartBrak = vecGlobal ? QLatin1String("<") : QLatin1String("\"");
        QString vEndBrak = vecGlobal ? QLatin1String(">") : QLatin1String("\"");
        QString value =QLatin1String("#include ") + vStartBrak + policy->getVectorClassNameInclude() + vEndBrak;
        includeStatement.append(value + endLine);
    }

    //only include classes in a different package from this class
    UMLPackageList includes;
    QMap<UMLPackage *, QString> packageMap; // so we don't repeat packages

    CodeGenerator::findObjectsRelated(c, includes);
    foreach(UMLPackage* con, includes) {
        if (!con->isUMLDatatype() && !packageMap.contains(con)) {
            packageMap.insert(con, con->package());
            if(con != getParentClassifier())
                includeStatement.append(QLatin1String("#include \"") + CodeGenerator::cleanName(con->name().toLower()) + QLatin1String(".h\"") + endLine);
        }
    }
    // now, add/update the includes codeblock
    CodeBlockWithComments * inclBlock = addOrUpdateTaggedCodeBlockWithComments(QLatin1String("includes"), includeStatement, QString(), 0, false);
    if(includeStatement.isEmpty() && inclBlock->contentType() == CodeBlock::AutoGenerated)
        inclBlock->setWriteOutText(false);
    else
        inclBlock->setWriteOutText(true);

    // Using
    QString usingStatement;
    foreach(UMLClassifier* classifier, superclasses) {
        if(classifier->package()!=c->package() && !classifier->package().isEmpty()) {
            usingStatement.append(QLatin1String("using ") + CodeGenerator::cleanName(c->package()) + QLatin1String("::") + cleanName(c->name()) + QLatin1Char(';') + endLine);
        }
    }
    CodeBlockWithComments * usingBlock = addOrUpdateTaggedCodeBlockWithComments(QLatin1String("using"), usingStatement, QString(), 0, false);
    if(usingStatement.isEmpty() && usingBlock->contentType() == CodeBlock::AutoGenerated)
        usingBlock->setWriteOutText(false);
    else
        usingBlock->setWriteOutText(true);

    // namespace
    // This needs special treatment. We cant use "nowriteouttext" for this, as
    // that will prevent the class declaration from being written. Instead, we
    // check if "hasNamspace" is true or not, and then indent the remaining code
    // appropriately as well as set the start/end text of this namespace block.
    if (c->umlPackage() && policy->getPackageIsNamespace())
        hasNamespace = true;
    else
        hasNamespace = false;

    // set start/end text of namespace block
    m_namespaceBlock = getHierarchicalCodeBlock(QLatin1String("namespace"), QLatin1String("Namespace"), 0);
    if(hasNamespace) {
        UMLPackageList pkgList = c->packages();
        QString pkgs;
        UMLPackage *pkg;
        foreach (pkg, pkgList) {
            pkgs += QLatin1String("namespace ") + CodeGenerator::cleanName(pkg->name()) + QLatin1String(" { ");
        }
        m_namespaceBlock->setStartText(pkgs);
        QString closingBraces;
        foreach (pkg, pkgList) {
            closingBraces += QLatin1String("} ");
        }
        m_namespaceBlock->setEndText(closingBraces);
        m_namespaceBlock->getComment()->setWriteOutText(true);
    } else {
        m_namespaceBlock->setStartText(QString());
        m_namespaceBlock->setEndText(QString());
        m_namespaceBlock->getComment()->setWriteOutText(false);
    }

    // Enum types for include
    if (!isInterface) {
        QString enumStatement;
        QString indent = UMLApp::app()->commonPolicy()->getIndentation();
        const UMLEnum* e = c->asUMLEnum();
        if (e) {
            enumStatement.append(indent + QLatin1String("enum ") + cppClassName + QLatin1String(" {") + endLine);

            // populate
            UMLClassifierListItemList ell = e->getFilteredList(UMLObject::ot_EnumLiteral);
            for (UMLClassifierListItemListIt elit(ell) ; elit.hasNext() ;) {
                UMLClassifierListItem* el = elit.next();
                enumStatement.append(indent + indent);
                enumStatement.append(CodeGenerator::cleanName(el->name()));
                if (elit.hasNext()) {
                    el=elit.next();
                    enumStatement.append(QLatin1String(", ") + endLine);
                } else {
                    break;
                }
                enumStatement.append(endLine);
            }
            enumStatement.append(indent + QLatin1String("};"));
            isEnumeration = true;
        }
        m_namespaceBlock->addOrUpdateTaggedCodeBlockWithComments(QLatin1String("enums"), enumStatement, QString(), 0, false);
    }

    // CLASS DECLARATION BLOCK
    //

    // add the class declaration block to the namespace block.
    CPPHeaderClassDeclarationBlock * myClassDeclCodeBlock = getClassDecl();
    m_namespaceBlock->addTextBlock(myClassDeclCodeBlock); // note: wont add if already present

    // Is this really true?? hmm..
    if(isEnumeration)
        myClassDeclCodeBlock->setWriteOutText(false); // not written out IF its an enumeration class
    else
        myClassDeclCodeBlock->setWriteOutText(true);

    //
    // Main Sub-Blocks
    //

    // declare public, protected and private methods, attributes (fields).
    // set the start text ONLY if this is the first time we created the objects.
    bool createdPublicBlock = m_publicBlock == 0 ? true : false;
    m_publicBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QLatin1String("publicBlock"),QLatin1String("Public stuff"), 0);
    if (createdPublicBlock)
        m_publicBlock->setStartText(QLatin1String("public:"));

    bool createdProtBlock = m_protectedBlock == 0 ? true : false;
    m_protectedBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QLatin1String("protectedBlock"),QLatin1String("Protected stuff"), 0);
    if(createdProtBlock)
        m_protectedBlock->setStartText(QLatin1String("protected:"));

    bool createdPrivBlock = m_privateBlock == 0 ? true : false;
    m_privateBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QLatin1String("privateBlock"),QLatin1String("Private stuff"), 0);
    if(createdPrivBlock)
        m_privateBlock->setStartText(QLatin1String("private:"));

    //
    // * CLASS FIELD declaration section
    //

    // setup/get/create the field declaration code block
    //

    // public fields: Update the comment: we only set comment to appear under the following conditions
    HierarchicalCodeBlock * publicFieldDeclBlock = m_publicBlock->getHierarchicalCodeBlock(QLatin1String("publicFieldsDecl"), QLatin1String("Fields"), 1);
    CodeComment * pubFcomment = publicFieldDeclBlock->getComment();
    if (!forcedoc && !hasclassFields)
        pubFcomment->setWriteOutText(false);
    else
        pubFcomment->setWriteOutText(true);

    // protected fields: Update the comment: we only set comment to appear under the following conditions
    HierarchicalCodeBlock * protectedFieldDeclBlock = m_protectedBlock->getHierarchicalCodeBlock(QLatin1String("protectedFieldsDecl"), QLatin1String("Fields"), 1);
    CodeComment * protFcomment = protectedFieldDeclBlock->getComment();
    if (!forcedoc && !hasclassFields)
        protFcomment->setWriteOutText(false);
    else
        protFcomment->setWriteOutText(true);

    // private fields: Update the comment: we only set comment to appear under the following conditions
    HierarchicalCodeBlock * privateFieldDeclBlock = m_privateBlock->getHierarchicalCodeBlock(QLatin1String("privateFieldsDecl"), QLatin1String("Fields"), 1);
    CodeComment * privFcomment = privateFieldDeclBlock->getComment();
    if (!forcedoc && !hasclassFields)
        privFcomment->setWriteOutText(false);
    else
        privFcomment->setWriteOutText(true);


    // now actually declare the fields within the appropriate HCodeBlock
    //

    // public
    declareClassFields(staticPublicAttribClassFields, publicFieldDeclBlock);
    declareClassFields(publicAttribClassFields, publicFieldDeclBlock);
    declareClassFields(publicPlainAssocClassFields, publicFieldDeclBlock);
    declareClassFields(publicAggregationClassFields, publicFieldDeclBlock);
    declareClassFields(publicCompositionClassFields, publicFieldDeclBlock);

    // protected
    declareClassFields(staticProtectedAttribClassFields, protectedFieldDeclBlock);
    declareClassFields(protectedAttribClassFields, protectedFieldDeclBlock);
    declareClassFields(protPlainAssocClassFields, protectedFieldDeclBlock);
    declareClassFields(protAggregationClassFields, protectedFieldDeclBlock);
    declareClassFields(protCompositionClassFields, protectedFieldDeclBlock);

    // private
    declareClassFields(staticPrivateAttribClassFields, privateFieldDeclBlock);
    declareClassFields(privateAttribClassFields, privateFieldDeclBlock);
    declareClassFields(privPlainAssocClassFields, privateFieldDeclBlock);
    declareClassFields(privAggregationClassFields, privateFieldDeclBlock);
    declareClassFields(privCompositionClassFields, privateFieldDeclBlock);

    //
    // METHODS section
    //

    // get/create the method codeblock

    // public methods
    HierarchicalCodeBlock * pubMethodsBlock = m_publicBlock->getHierarchicalCodeBlock(QLatin1String("pubMethodsBlock"), QString(), 1);
    CodeComment * pubMethodsComment = pubMethodsBlock->getComment();
    // set conditions for showing this comment
    if (!forcedoc && !hasclassFields && !hasOperationMethods)
        pubMethodsComment->setWriteOutText(false);
    else
        pubMethodsComment->setWriteOutText(true);

    // protected methods
    HierarchicalCodeBlock * protMethodsBlock = m_protectedBlock->getHierarchicalCodeBlock(QLatin1String("protMethodsBlock"), QString(), 1);
    CodeComment * protMethodsComment = protMethodsBlock->getComment();
    // set conditions for showing this comment
    if (!forcedoc && !hasclassFields && !hasOperationMethods)
        protMethodsComment->setWriteOutText(false);
    else
        protMethodsComment->setWriteOutText(true);

    // private methods
    HierarchicalCodeBlock * privMethodsBlock = m_privateBlock->getHierarchicalCodeBlock(QLatin1String("privMethodsBlock"), QString(), 1);
    CodeComment * privMethodsComment = privMethodsBlock->getComment();
    // set conditions for showing this comment
    if (!forcedoc && !hasclassFields && !hasOperationMethods)
        privMethodsComment->setWriteOutText(false);
    else
        privMethodsComment->setWriteOutText(true);


    // METHODS sub-section : constructor methods
    //
    CodeGenerationPolicy *pol = UMLApp::app()->commonPolicy();

    // setup/get/create the constructor codeblocks

    // public
    m_pubConstructorBlock = pubMethodsBlock->getHierarchicalCodeBlock(QLatin1String("constructionMethods"), QLatin1String("Constructors"), 1);
    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * pubConstComment = m_pubConstructorBlock->getComment();
    if (!forcedoc && (isInterface || !pol->getAutoGenerateConstructors()))
        pubConstComment->setWriteOutText(false);
    else
        pubConstComment->setWriteOutText(true);

    // protected
    m_protConstructorBlock = protMethodsBlock->getHierarchicalCodeBlock(QLatin1String("constructionMethods"), QLatin1String("Constructors"), 1);
    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * protConstComment = m_protConstructorBlock->getComment();
    if (!forcedoc && (isInterface || !pol->getAutoGenerateConstructors()))
        protConstComment->setWriteOutText(false);
    else
        protConstComment->setWriteOutText(true);

    // private
    m_privConstructorBlock = privMethodsBlock->getHierarchicalCodeBlock(QLatin1String("constructionMethods"), QLatin1String("Constructors"), 1);
    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * privConstComment = m_privConstructorBlock->getComment();
    if (!forcedoc && (isInterface || !pol->getAutoGenerateConstructors()))
        privConstComment->setWriteOutText(false);
    else
        privConstComment->setWriteOutText(true);

    // add/get the empty constructor. I guess since there is no
    // meta-data to state what the scope of this method is, we will make it
    // "public" as a default. This might present problems if the user wants
    // to move the block into the "private" or "protected" blocks.
    QString emptyConstStatement = cppClassName + QLatin1String(" () { }");

    // search for this first in the entire document. IF not present, put
    // it in the public constructor method block
    TextBlock * emptyConstTb = findTextBlockByTag(QLatin1String("emptyconstructor"), true);
    CodeBlockWithComments * emptyConstBlock = dynamic_cast<CodeBlockWithComments*>(emptyConstTb);
    if(!emptyConstBlock)
        emptyConstBlock = m_pubConstructorBlock->addOrUpdateTaggedCodeBlockWithComments(QLatin1String("emptyconstructor"), emptyConstStatement, QLatin1String("Empty Constructor"), 1, false);

    // Now, as an additional condition we only show the empty constructor block
    // IF it was desired to be shown
    if(!isInterface && pol->getAutoGenerateConstructors())
        emptyConstBlock->setWriteOutText(true);
    else
        emptyConstBlock->setWriteOutText(false);


    // METHODS subsection : ACCESSOR METHODS
    //

    // get/create the accessor codeblock

    // public
    HierarchicalCodeBlock * pubAccessorBlock = pubMethodsBlock->getHierarchicalCodeBlock(QLatin1String("accessorMethods"), QLatin1String("Accessor Methods"), 1);
    // set conditions for showing section comment
    CodeComment * pubAccessComment = pubAccessorBlock->getComment();
    if (!forcedoc && !hasclassFields)
        pubAccessComment->setWriteOutText(false);
    else
        pubAccessComment->setWriteOutText(true);

    // protected
    HierarchicalCodeBlock * protAccessorBlock = protMethodsBlock->getHierarchicalCodeBlock(QLatin1String("accessorMethods"), QLatin1String("Accessor Methods"), 1);
    // set conditions for showing section comment
    CodeComment * protAccessComment = protAccessorBlock->getComment();
    if (!forcedoc && !hasclassFields)
        protAccessComment->setWriteOutText(false);
    else
        protAccessComment->setWriteOutText(true);

    // private
    HierarchicalCodeBlock * privAccessorBlock = privMethodsBlock->getHierarchicalCodeBlock(QLatin1String("accessorMethods"), QLatin1String("Accessor Methods"), 1);
    // set conditions for showing section comment
    CodeComment * privAccessComment = privAccessorBlock->getComment();
    // We've to copy the private accessorMethods to the public block
    if (!forcedoc && !hasclassFields)
        privAccessComment->setWriteOutText(false);
    else
        privAccessComment->setWriteOutText(true);

    // now, 2 sub-sub sections in accessor block
    // add/update accessor methods for attributes
    HierarchicalCodeBlock * pubStaticAccessors = pubAccessorBlock->getHierarchicalCodeBlock(QLatin1String("pubStaticAccessorMethods"), QString(), 1);
    HierarchicalCodeBlock * pubRegularAccessors = pubAccessorBlock->getHierarchicalCodeBlock(QLatin1String("pubRegularAccessorMethods"), QString(), 1);
    pubStaticAccessors->getComment()->setWriteOutText(false); // never write block comment
    pubRegularAccessors->getComment()->setWriteOutText(false); // never write block comment

    HierarchicalCodeBlock * protStaticAccessors = protAccessorBlock->getHierarchicalCodeBlock(QLatin1String("protStaticAccessorMethods"), QString(), 1);
    HierarchicalCodeBlock * protRegularAccessors = protAccessorBlock->getHierarchicalCodeBlock(QLatin1String("protRegularAccessorMethods"), QString(), 1);
    protStaticAccessors->getComment()->setWriteOutText(false); // never write block comment
    protRegularAccessors->getComment()->setWriteOutText(false); // never write block comment

    HierarchicalCodeBlock * privStaticAccessors = privAccessorBlock->getHierarchicalCodeBlock(QLatin1String("privStaticAccessorMethods"), QString(), 1);
    HierarchicalCodeBlock * privRegularAccessors = privAccessorBlock->getHierarchicalCodeBlock(QLatin1String("privRegularAccessorMethods"), QString(), 1);
    privStaticAccessors->getComment()->setWriteOutText(false); // never write block comment
    privRegularAccessors->getComment()->setWriteOutText(false); // never write block comment

    // now add in accessors as appropriate

    // public stuff
    pubStaticAccessors->addCodeClassFieldMethods(staticPublicAttribClassFields);
    pubRegularAccessors->addCodeClassFieldMethods(publicAttribClassFields);

    // generate accessors as public
    if (policy && policy->getAccessorsArePublic())
    {
        pubRegularAccessors->addCodeClassFieldMethods(privateAttribClassFields);
        pubRegularAccessors->addCodeClassFieldMethods(protectedAttribClassFields);
    }

    pubRegularAccessors->addCodeClassFieldMethods(publicPlainAssocClassFields);
    pubRegularAccessors->addCodeClassFieldMethods(publicAggregationClassFields);
    pubRegularAccessors->addCodeClassFieldMethods(publicCompositionClassFields);

    // protected stuff
    protStaticAccessors->addCodeClassFieldMethods(staticProtectedAttribClassFields);

    // accessors are public so we don't have to create it here
    if (policy && !policy->getAccessorsArePublic())
        protRegularAccessors->addCodeClassFieldMethods(protectedAttribClassFields);

    protRegularAccessors->addCodeClassFieldMethods(protPlainAssocClassFields);
    protRegularAccessors->addCodeClassFieldMethods(protAggregationClassFields);
    protRegularAccessors->addCodeClassFieldMethods(protCompositionClassFields);

    // private stuff
    privStaticAccessors->addCodeClassFieldMethods(staticPrivateAttribClassFields);

    // accessors are public so we don't have to create it here
    if (policy && !policy->getAccessorsArePublic())
        privRegularAccessors->addCodeClassFieldMethods(privateAttribClassFields);

    privRegularAccessors->addCodeClassFieldMethods(privPlainAssocClassFields);
    privRegularAccessors->addCodeClassFieldMethods(privAggregationClassFields);
    privRegularAccessors->addCodeClassFieldMethods(privCompositionClassFields);


    // METHODS subsection : Operation methods (e.g. methods derive from operations but which arent constructors)
    //

    // setup/get/create the operations codeblock

    // public
    m_pubOperationsBlock = pubMethodsBlock->getHierarchicalCodeBlock(QLatin1String("operationMethods"), QLatin1String("Operations"), 1);
    // set conditions for showing section comment
    CodeComment * pubOcomment = m_pubOperationsBlock->getComment();
    if (!forcedoc && !hasOperationMethods)
        pubOcomment->setWriteOutText(false);
    else
        pubOcomment->setWriteOutText(true);

    //protected
    m_protOperationsBlock = protMethodsBlock->getHierarchicalCodeBlock(QLatin1String("operationMethods"), QLatin1String("Operations"), 1);
    // set conditions for showing section comment
    CodeComment * protOcomment = m_protOperationsBlock->getComment();
    if (!forcedoc && !hasOperationMethods)
        protOcomment->setWriteOutText(false);
    else
        protOcomment->setWriteOutText(true);

    //private
    m_privOperationsBlock = privMethodsBlock->getHierarchicalCodeBlock(QLatin1String("operationMethods"), QLatin1String("Operations"), 1);
    // set conditions for showing section comment
    CodeComment * privOcomment = m_privOperationsBlock->getComment();
    if (!forcedoc && !hasOperationMethods)
        privOcomment->setWriteOutText(false);
    else
        privOcomment->setWriteOutText(true);

    // Operations
    //
    // nothing to do here.. "updateOperations" in parent class puts things
    // in the right place using the "addCodeOperation" method we defined in this class

    // FINISH up with hash def block close
    QString defTextEnd = QLatin1String("#endif //") + hashDefine + QLatin1String("_H");
    addOrUpdateTaggedCodeBlockWithComments(QLatin1String("hashDefBlockEnd"), defTextEnd, QString(), 0, false);
}

