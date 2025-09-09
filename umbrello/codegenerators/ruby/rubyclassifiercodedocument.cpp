/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "rubyclassifiercodedocument.h"

// local includes
#include "codegen_utils.h"
#include "umlclassifier.h"
#include "debug_utils.h"
#include "rubycodegenerator.h"
#include "rubycodecomment.h"
#include "rubyclassdeclarationblock.h"
#include "rubycodeclassfielddeclarationblock.h"
#include "rubycodeoperation.h"
#include "umlapp.h"

DEBUG_REGISTER_DISABLED(RubyClassifierCodeDocument)

// qt includes
#include <QRegularExpression>

/**
 * Constructor.
 */
RubyClassifierCodeDocument::RubyClassifierCodeDocument(UMLClassifier * classifier)
        : ClassifierCodeDocument(classifier)
{
    init();
}

/**
 * Empty Destructor.
 */
RubyClassifierCodeDocument::~RubyClassifierCodeDocument()
{
}

/**
 * Make it easier on ourselves.
 */
RubyCodeGenerationPolicy * RubyClassifierCodeDocument::getRubyPolicy() const
{
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    RubyCodeGenerationPolicy * policy = dynamic_cast<RubyCodeGenerationPolicy*>(pe);
    return policy;
}

//**
// * Get the dialog widget which allows user interaction with the object parameters.
// * @return      CodeDocumentDialog
// */
/*
CodeDocumentDialog RubyClassifierCodeDocument::getDialog()
{
}
*/

/**
 * Overwritten by Ruby language implementation to get lowercase path.
 */
QString RubyClassifierCodeDocument::getPath() const
{
    QString path = getPackage();

    // Replace all white spaces with blanks
    path = path.simplified();

    // Replace all blanks with underscore
    path.replace(QRegularExpression(QStringLiteral(" ")), QStringLiteral("_"));

    path.replace(QRegularExpression(QStringLiteral("\\.")),QStringLiteral("/"));
    path.replace(QRegularExpression(QStringLiteral("::")), QStringLiteral("/"));

    path = path.toLower();

    return path;
}

QString RubyClassifierCodeDocument::getRubyClassName(const QString &name) const
{
    CodeGenerator *g = UMLApp::app()->generator();
    return Codegen_Utils::capitalizeFirstLetter(g->cleanName(name));
}

// Initialize this ruby classifier code document
void RubyClassifierCodeDocument::init()
{
    setFileExtension(QStringLiteral(".rb"));

    //initCodeClassFields(); // this is dubious because it calls down to
                             // CodeGenFactory::newCodeClassField(this)
                             // but "this" is still in construction at that time.

    classDeclCodeBlock = nullptr;
    publicBlock = nullptr;
    protectedBlock = nullptr;
    privateBlock = nullptr;
    pubConstructorBlock = nullptr;
    protConstructorBlock = nullptr;
    privConstructorBlock = nullptr;
    pubOperationsBlock = nullptr;
    privOperationsBlock = nullptr;
    protOperationsBlock = nullptr;

    // this will call updateContent() as well as other things that sync our document.
    synchronize();
}

/**
 * Add a code operation to this ruby classifier code document.
 * In the vanilla version, we just tack all operations on the end
 * of the document.
 * @param op   the code operation
 * @return bool which is true IF the code operation was added successfully
 */
bool RubyClassifierCodeDocument::addCodeOperation(CodeOperation * op)
{
    Uml::Visibility::Enum scope = op->getParentOperation()->visibility();
    if (!op->getParentOperation()->isConstructorOperation())
    {
        switch (scope) {
        default:
        case Uml::Visibility::Public:
            return pubOperationsBlock->addTextBlock(op);
            break;
        case Uml::Visibility::Protected:
            return protOperationsBlock->addTextBlock(op);
            break;
        case Uml::Visibility::Private:
            return privOperationsBlock->addTextBlock(op);
            break;
        }
    } else {
        switch (scope) {
        default:
        case Uml::Visibility::Public:
            return pubConstructorBlock->addTextBlock(op);
            break;
        case Uml::Visibility::Protected:
            return protConstructorBlock->addTextBlock(op);
            break;
        case Uml::Visibility::Private:
            return privConstructorBlock->addTextBlock(op);
            break;
        }
    }
}

/**
 * Need to overwrite this for ruby since we need to pick up the
 * ruby class declaration block.
 * Sigh. NOT optimal. The only reason that we need to have this
 * is so we can create the RubyClassDeclarationBlock.
 * would be better if we could create a handler interface that each
 * codeblock used so all we have to do here is add the handler
 * for "rubyclassdeclarationblock".
 */
void RubyClassifierCodeDocument::loadChildTextBlocksFromNode(QDomElement & root)
{
    QDomNode tnode = root.firstChild();
    QDomElement telement = tnode.toElement();
    bool loadCheckForChildrenOK = false;
    while (!telement.isNull()) {
        QString nodeName = telement.tagName();

        if (nodeName == QStringLiteral("textblocks")) {

            QDomNode node = telement.firstChild();
            QDomElement element = node.toElement();

            // if there is nothing to begin with, then we don't worry about it
            loadCheckForChildrenOK = element.isNull() ? true : false;

            while (!element.isNull()) {
                QString name = element.tagName();

                if (name == QStringLiteral("codecomment")) {
                    CodeComment * block = new RubyCodeComment(this);
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("RubyClassifierCodeDocument::loadChildTextBlocksFromNode : unable to add codeComment");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("codeaccessormethod") ||
                           name == QStringLiteral("ccfdeclarationcodeblock")) {
                    QString acctag = element.attribute(QStringLiteral("tag"));
                    // search for our method in the
                    TextBlock * tb = findCodeClassFieldTextBlockByTag(acctag);
                    if (!tb || !addTextBlock(tb)) {
                        logError0("RubyClassifierCodeDocument::loadChildTextBlocksFromNode : unable to add codeclassfield child method");
                        // DON'T delete
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("codeblock")) {
                    CodeBlock * block = newCodeBlock();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("RubyClassifierCodeDocument::loadChildTextBlocksFromNode : unable to add codeBlock");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("codeblockwithcomments")) {
                    CodeBlockWithComments * block = newCodeBlockWithComments();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("RubyClassifierCodeDocument::loadChildTextBlocksFromNode : unable to add codeBlockwithcomments");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("header")) {
                    // do nothing.. this is treated elsewhere
                } else if (name == QStringLiteral("hierarchicalcodeblock")) {
                    HierarchicalCodeBlock * block = newHierarchicalCodeBlock();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("RubyClassifierCodeDocument::loadChildTextBlocksFromNode: Unable to add hierarchicalcodeBlock");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("codeoperation")) {
                    // find the code operation by id
                    QString id = element.attribute(QStringLiteral("parent_id"), QStringLiteral("-1"));
                    UMLObject * obj = UMLApp::app()->document()->findObjectById(Uml::ID::fromString(id));
                    UMLOperation * op = obj->asUMLOperation();
                    if (op) {
                        CodeOperation * block = new RubyCodeOperation(this, op);
                        block->loadFromXMI(element);
                        if (addTextBlock(block)) {
                            loadCheckForChildrenOK= true;
                        } else {
                            logError0("RubyClassifierCodeDocument::loadChildTextBlocksFromNode: Unable to add codeoperation");
                            block->deleteLater();
                        }
                    } else {
                        logError0("RubyClassifierCodeDocument::loadChildTextBlocksFromNode: Unable to find operation create codeoperation");
                    }
                } else if (name == QStringLiteral("rubyclassdeclarationblock")) {
                    RubyClassDeclarationBlock * block = getClassDecl();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("RubyClassifierCodeDocument::loadChildTextBlocksFromNode: Unable to add ruby code declaration block");
                        // DON'T delete.
                        // block->deleteLater();
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else {
                    logDebug1("RubyClassifierCodeDocument::loadChildTextBlocksFromNode: Got strange tag in "
                              "text block stack: %1, ignoring", name);
                }

                node = element.nextSibling();
                element = node.toElement();
            }
            break;
        }

        tnode = telement.nextSibling();
        telement = tnode.toElement();
    }

    if (!loadCheckForChildrenOK)
    {
        logWarn1("RubyClassifierCodeDocument::loadChildTextBlocksFromNode: unable to initialize any "
                 "child blocks in doc: %1", getFileName());
    }
}

RubyClassDeclarationBlock * RubyClassifierCodeDocument::getClassDecl()
{
    if (!classDeclCodeBlock)
    {
        classDeclCodeBlock = new RubyClassDeclarationBlock (this);
        classDeclCodeBlock->updateContent();
        classDeclCodeBlock->setTag(QStringLiteral("ClassDeclBlock"));
    }
    return classDeclCodeBlock;
}

/**
 * Reset/clear our inventory of textblocks in this document.
 */
void RubyClassifierCodeDocument::resetTextBlocks()
{
    // all special pointers to text blocks need to be zero'd out
    operationsBlock = nullptr;
    constructorBlock = nullptr;
    classDeclCodeBlock = nullptr;

    // now do traditional release of text blocks.
    ClassifierCodeDocument::resetTextBlocks();
}

// This method will cause the class to rebuild its text representation.
// based on the parent classifier object.
// For any situation in which this is called, we are either building the code
// document up, or replacing/regenerating the existing auto-generated parts. As
// such, we will want to insert everything we reasonably will want
// during creation. We can set various parts of the document (esp. the
// comments) to appear or not, as needed.
void RubyClassifierCodeDocument::updateContent()
{
    // Gather info on the various fields and parent objects of this class...
    UMLClassifier * c = getParentClassifier();
    RubyCodeGenerator * gen = dynamic_cast<RubyCodeGenerator*>(UMLApp::app()->generator());

    // first, set the global flag on whether or not to show classfield info
    // This depends on whether or not we have attribute/association classes
    const CodeClassFieldList * cfList = getCodeClassFieldList();
    CodeClassFieldList::const_iterator it = cfList->begin();
    CodeClassFieldList::const_iterator end = cfList->end();
    for (; it != end; ++it) {
        CodeClassField * field = *it;
        if (field->parentIsAttribute())
            field->setWriteOutMethods(gen->getAutoGenerateAttribAccessors());
        else
            field->setWriteOutMethods(gen->getAutoGenerateAssocAccessors());
    }
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

    bool isInterface = parentIsInterface();
    bool hasOperationMethods = false;
    Q_ASSERT(c != nullptr);
    if (c) {
        UMLOperationList list = c->getOperationsList();
        hasOperationMethods = ! list.isEmpty();
    }
    CodeGenerationPolicy *pol = UMLApp::app()->commonPolicy();
    QString endLine = pol->getNewLineEndingChars(); // a shortcut..so we don't have to call this all the time

    //
    // START GENERATING CODE/TEXT BLOCKS and COMMENTS FOR THE DOCUMENT
    //


    // CLASS DECLARATION BLOCK
    //

    // get the declaration block. If it is not already present, add it too
    RubyClassDeclarationBlock * myClassDeclCodeBlock = getClassDecl();
    addTextBlock(myClassDeclCodeBlock); // note: wont add if already present

    // declare public, protected and private methods, attributes (fields).
    // set the start text ONLY if this is the first time we created the objects.
    bool createdPublicBlock = publicBlock == nullptr ? true : false;
    publicBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QStringLiteral("publicBlock"), QStringLiteral("Public Items"), 0);
    if (createdPublicBlock)
        publicBlock->setStartText(QStringLiteral("public"));

    bool createdProtBlock = protectedBlock == nullptr ? true : false;
    protectedBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QStringLiteral("protectedBlock"), QStringLiteral("Protected Items"), 0);
    if (createdProtBlock)
        protectedBlock->setStartText(QStringLiteral("protected"));

    bool createdPrivBlock = privateBlock == nullptr ? true : false;
    privateBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QStringLiteral("privateBlock"), QStringLiteral("Private Items"), 0);
    if (createdPrivBlock)
        privateBlock->setStartText(QStringLiteral("private"));

    // NOW create document in sections..
    // now we want to populate the body of our class
    // our layout is the following general groupings of code blocks:

    // start ruby classifier document

    // header comment

     // class declaration

    //   section:

    //   section:
    //   - methods section comment

    //     sub-section: constructor ops
    //     - constructor method section comment
    //     - constructor methods (0+ codeblocks)

    //     sub-section: accessors
    //     - accessor method section comment
    //     - static accessor methods (0+ codeblocks)
    //     - non-static accessor methods (0+ codeblocks)

    //     sub-section: non-constructor ops
    //     - operation method section comment
    //     - operations (0+ codeblocks)

    // end class declaration

    // end ruby classifier document


    // Q: Why use the more complicated scheme of arranging code blocks within codeblocks?
    // A: This will allow us later to preserve the format of our document so that if
    //    codeblocks are added, they may be easily added in the correct place, rather than at
    //    the end of the document, or by using a difficult algorithm to find the location of
    //    the last appropriate code block sibling (which may not exist.. for example user adds
    //    a constructor operation, but there currently are no constructor code blocks
    //    within the document).


    //
    // METHODS section
    //

    // get/create the method codeblock
    // public methods
    HierarchicalCodeBlock * pubMethodsBlock = publicBlock->getHierarchicalCodeBlock(QStringLiteral("pubMethodsBlock"), QString(), 1);
    CodeComment * pubMethodsComment = pubMethodsBlock->getComment();
    bool forceDoc = pol->getCodeVerboseDocumentComments();
    // set conditions for showing this comment
    if (!forceDoc && !hasClassFields() && !hasOperationMethods)
        pubMethodsComment->setWriteOutText(false);
    else
        pubMethodsComment->setWriteOutText(true);

    // protected methods
    HierarchicalCodeBlock * protMethodsBlock = protectedBlock->getHierarchicalCodeBlock(QStringLiteral("protMethodsBlock"), QString(), 1);
    CodeComment * protMethodsComment = protMethodsBlock->getComment();
    // set conditions for showing this comment
    if (!forceDoc && !hasClassFields() && !hasOperationMethods)
        protMethodsComment->setWriteOutText(false);
    else
        protMethodsComment->setWriteOutText(true);

    // private methods
    HierarchicalCodeBlock * privMethodsBlock = privateBlock->getHierarchicalCodeBlock(QStringLiteral("privMethodsBlock"), QString(), 1);
    CodeComment * privMethodsComment = privMethodsBlock->getComment();
    // set conditions for showing this comment
    if (!forceDoc && !hasClassFields() && !hasOperationMethods)
        privMethodsComment->setWriteOutText(false);
    else
        privMethodsComment->setWriteOutText(true);

    // METHODS sub-section : constructor methods
    //

    // public
    pubConstructorBlock = pubMethodsBlock->getHierarchicalCodeBlock(QStringLiteral("constructionMethods"), QStringLiteral("Constructors"), 1);
    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * pubConstComment = pubConstructorBlock->getComment();
    if (!forceDoc && (isInterface || !pol->getAutoGenerateConstructors()))
        pubConstComment->setWriteOutText(false);
    else
        pubConstComment->setWriteOutText(true);

    // protected
    protConstructorBlock = protMethodsBlock->getHierarchicalCodeBlock(QStringLiteral("constructionMethods"), QStringLiteral("Constructors"), 1);
    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * protConstComment = protConstructorBlock->getComment();
    if (!forceDoc && (isInterface || !pol->getAutoGenerateConstructors()))
        protConstComment->setWriteOutText(false);
    else
        protConstComment->setWriteOutText(true);

    // private
    privConstructorBlock = privMethodsBlock->getHierarchicalCodeBlock(QStringLiteral("constructionMethods"), QStringLiteral("Constructors"), 1);
    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * privConstComment = privConstructorBlock->getComment();
    if (!forceDoc && (isInterface || !pol->getAutoGenerateConstructors()))
        privConstComment->setWriteOutText(false);
    else
        privConstComment->setWriteOutText(true);

    // get/create the accessor codeblock
    // public
    HierarchicalCodeBlock * pubAccessorBlock = pubMethodsBlock->getHierarchicalCodeBlock(QStringLiteral("accessorMethods"), QStringLiteral("Accessor Methods"), 1);
    // set conditions for showing section comment
    CodeComment * pubAccessComment = pubAccessorBlock->getComment();
    if (!forceDoc && !hasClassFields())
        pubAccessComment->setWriteOutText(false);
    else
        pubAccessComment->setWriteOutText(true);

    // protected
    HierarchicalCodeBlock * protAccessorBlock = protMethodsBlock->getHierarchicalCodeBlock(QStringLiteral("accessorMethods"), QStringLiteral("Accessor Methods"), 1);
    // set conditions for showing section comment
    CodeComment * protAccessComment = protAccessorBlock->getComment();
    if (!forceDoc && !hasClassFields())
        protAccessComment->setWriteOutText(false);
    else
        protAccessComment->setWriteOutText(true);

    // private
    HierarchicalCodeBlock * privAccessorBlock = privMethodsBlock->getHierarchicalCodeBlock(QStringLiteral("accessorMethods"), QStringLiteral("Accessor Methods"), 1);
    // set conditions for showing section comment
    CodeComment * privAccessComment = privAccessorBlock->getComment();
    if (!forceDoc && !hasClassFields())
        privAccessComment->setWriteOutText(false);
    else
        privAccessComment->setWriteOutText(true);

    // now, 2 sub-sub sections in accessor block
    // add/update accessor methods for attributes
    HierarchicalCodeBlock * pubStaticAccessors = pubAccessorBlock->getHierarchicalCodeBlock(QStringLiteral("pubStaticAccessorMethods"), QString(), 1);
    HierarchicalCodeBlock * pubRegularAccessors = pubAccessorBlock->getHierarchicalCodeBlock(QStringLiteral("pubRegularAccessorMethods"), QString(), 1);
    pubStaticAccessors->getComment()->setWriteOutText(false); // never write block comment
    pubRegularAccessors->getComment()->setWriteOutText(false); // never write block comment

    HierarchicalCodeBlock * protStaticAccessors = protAccessorBlock->getHierarchicalCodeBlock(QStringLiteral("protStaticAccessorMethods"), QString(), 1);
    HierarchicalCodeBlock * protRegularAccessors = protAccessorBlock->getHierarchicalCodeBlock(QStringLiteral("protRegularAccessorMethods"), QString(), 1);
    protStaticAccessors->getComment()->setWriteOutText(false); // never write block comment
    protRegularAccessors->getComment()->setWriteOutText(false); // never write block comment

    HierarchicalCodeBlock * privStaticAccessors = privAccessorBlock->getHierarchicalCodeBlock(QStringLiteral("privStaticAccessorMethods"), QString(), 1);
    HierarchicalCodeBlock * privRegularAccessors = privAccessorBlock->getHierarchicalCodeBlock(QStringLiteral("privRegularAccessorMethods"), QString(), 1);
    privStaticAccessors->getComment()->setWriteOutText(false); // never write block comment
    privRegularAccessors->getComment()->setWriteOutText(false); // never write block comment
    // now add in accessors as appropriate

    // public stuff
    pubStaticAccessors->addCodeClassFieldMethods(staticPublicAttribClassFields);
    pubRegularAccessors->addCodeClassFieldMethods(publicAttribClassFields);
    pubRegularAccessors->addCodeClassFieldMethods(publicPlainAssocClassFields);
    pubRegularAccessors->addCodeClassFieldMethods(publicAggregationClassFields);
    pubRegularAccessors->addCodeClassFieldMethods(publicCompositionClassFields);

    // protected stuff
    protStaticAccessors->addCodeClassFieldMethods(staticProtectedAttribClassFields);
    protRegularAccessors->addCodeClassFieldMethods(protectedAttribClassFields);
    protRegularAccessors->addCodeClassFieldMethods(protPlainAssocClassFields);
    protRegularAccessors->addCodeClassFieldMethods(protAggregationClassFields);
    protRegularAccessors->addCodeClassFieldMethods(protCompositionClassFields);

    // private stuff
    privStaticAccessors->addCodeClassFieldMethods(staticPrivateAttribClassFields);
    privRegularAccessors->addCodeClassFieldMethods(privateAttribClassFields);
    privRegularAccessors->addCodeClassFieldMethods(privPlainAssocClassFields);
    privRegularAccessors->addCodeClassFieldMethods(privAggregationClassFields);
    privRegularAccessors->addCodeClassFieldMethods(privCompositionClassFields);

    // METHODS subsection : Operation methods (which aren't constructors)
    //

    // setup/get/create the operations codeblock

    // public
    pubOperationsBlock = pubMethodsBlock->getHierarchicalCodeBlock(QStringLiteral("operationMethods"), QStringLiteral("Operations"), 1);
    // set conditions for showing section comment
    CodeComment * pubOcomment = pubOperationsBlock->getComment();
    if (!forceDoc && !hasOperationMethods)
        pubOcomment->setWriteOutText(false);
    else
        pubOcomment->setWriteOutText(true);

    //protected
    protOperationsBlock = protMethodsBlock->getHierarchicalCodeBlock(QStringLiteral("operationMethods"), QStringLiteral("Operations"), 1);
    // set conditions for showing section comment
    CodeComment * protOcomment = protOperationsBlock->getComment();
    if (!forceDoc && !hasOperationMethods)
        protOcomment->setWriteOutText(false);
    else
        protOcomment->setWriteOutText(true);

    //private
    privOperationsBlock = privMethodsBlock->getHierarchicalCodeBlock(QStringLiteral("operationMethods"), QStringLiteral("Operations"), 1);
    // set conditions for showing section comment
    CodeComment * privOcomment = privOperationsBlock->getComment();
    if (!forceDoc && !hasOperationMethods)
        privOcomment->setWriteOutText(false);
    else
        privOcomment->setWriteOutText(true);
}


