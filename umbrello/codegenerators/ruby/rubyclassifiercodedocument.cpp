/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "rubyclassifiercodedocument.h"

// local includes
#include "codegen_utils.h"
#include "classifier.h"
#include "debug_utils.h"
#include "rubycodegenerator.h"
#include "rubycodecomment.h"
#include "rubyclassdeclarationblock.h"
#include "rubycodeclassfielddeclarationblock.h"
#include "rubycodeoperation.h"
#include "uml.h"

// qt includes
#include <QRegExp>

/**
 * Constructor.
 */
RubyClassifierCodeDocument::RubyClassifierCodeDocument(UMLClassifier * concept)
        : ClassifierCodeDocument(concept)
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
RubyCodeGenerationPolicy * RubyClassifierCodeDocument::getRubyPolicy()
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
QString RubyClassifierCodeDocument::getPath()
{
    QString path = getPackage();

    // Replace all white spaces with blanks
    path = path.simplified();

    // Replace all blanks with underscore
    path.replace(QRegExp(QLatin1String(" ")), QLatin1String("_"));

    path.replace(QRegExp(QLatin1String("\\.")),QLatin1String("/"));
    path.replace(QRegExp(QLatin1String("::")), QLatin1String("/"));

    path = path.toLower();

    return path;
}

QString RubyClassifierCodeDocument::getRubyClassName(const QString &name)
{
    CodeGenerator *g = UMLApp::app()->generator();
    return Codegen_Utils::capitalizeFirstLetter(g->cleanName(name));
}

// Initialize this ruby classifier code document
void RubyClassifierCodeDocument::init()
{
    setFileExtension(QLatin1String(".rb"));

    //initCodeClassFields(); // this is dubious because it calls down to
                             // CodeGenFactory::newCodeClassField(this)
                             // but "this" is still in construction at that time.

    classDeclCodeBlock = 0;
    publicBlock = 0;
    protectedBlock = 0;
    privateBlock = 0;
    pubConstructorBlock = 0;
    protConstructorBlock = 0;
    privConstructorBlock = 0;
    pubOperationsBlock = 0;
    privOperationsBlock = 0;
    protOperationsBlock = 0;

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

        if (nodeName == QLatin1String("textblocks")) {

            QDomNode node = telement.firstChild();
            QDomElement element = node.toElement();

            // if there is nothing to begin with, then we don't worry about it
            loadCheckForChildrenOK = element.isNull() ? true : false;

            while (!element.isNull()) {
                QString name = element.tagName();

                if (name == QLatin1String("codecomment")) {
                    CodeComment * block = new RubyCodeComment(this);
                    block->loadFromXMI1(element);
                    if (!addTextBlock(block)) {
                        uError()<<"loadFromXMI1 : unable to add codeComment to :"<<this;
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QLatin1String("codeaccessormethod") ||
                           name == QLatin1String("ccfdeclarationcodeblock")) {
                    QString acctag = element.attribute(QLatin1String("tag"));
                    // search for our method in the
                    TextBlock * tb = findCodeClassFieldTextBlockByTag(acctag);
                    if (!tb || !addTextBlock(tb)) {
                        uError()<<"loadFromXMI1 : unable to add codeclassfield child method to:"<<this;
                        // DON'T delete
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QLatin1String("codeblock")) {
                    CodeBlock * block = newCodeBlock();
                    block->loadFromXMI1(element);
                    if (!addTextBlock(block)) {
                        uError()<<"loadFromXMI1 : unable to add codeBlock to :"<<this;
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QLatin1String("codeblockwithcomments")) {
                    CodeBlockWithComments * block = newCodeBlockWithComments();
                    block->loadFromXMI1(element);
                    if (!addTextBlock(block)) {
                        uError()<<"loadFromXMI1 : unable to add codeBlockwithcomments to:"<<this;
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QLatin1String("header")) {
                    // do nothing.. this is treated elsewhere
                } else if (name == QLatin1String("hierarchicalcodeblock")) {
                    HierarchicalCodeBlock * block = newHierarchicalCodeBlock();
                    block->loadFromXMI1(element);
                    if (!addTextBlock(block)) {
                        uError()<<"Unable to add hierarchicalcodeBlock to:"<<this;
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QLatin1String("codeoperation")) {
                    // find the code operation by id
                    QString id = element.attribute(QLatin1String("parent_id"), QLatin1String("-1"));
                    UMLObject * obj = UMLApp::app()->document()->findObjectById(Uml::ID::fromString(id));
                    UMLOperation * op = obj->asUMLOperation();
                    if (op) {
                        CodeOperation * block = new RubyCodeOperation(this, op);
                        block->loadFromXMI1(element);
                        if (addTextBlock(block)) {
                            loadCheckForChildrenOK= true;
                        } else {
                            uError()<<"Unable to add codeoperation to:"<<this;
                            block->deleteLater();
                        }
                    } else {
                        uError()<<"Unable to find operation create codeoperation for:"<<this;
                    }
                } else if (name == QLatin1String("rubyclassdeclarationblock")) {
                    RubyClassDeclarationBlock * block = getClassDecl();
                    block->loadFromXMI1(element);
                    if (!addTextBlock(block)) {
                        uError()<<"Unable to add ruby code declaration block to:"<<this;
                        // DON'T delete.
                        // block->deleteLater();
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else {
                    uDebug()<<" LoadFromXMI: Got strange tag in text block stack:"<<name<<", ignoring";
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
        uWarning()<<" loadChildBlocks : unable to initialize any child blocks in doc: " << getFileName() << " " << this;
    }
}

RubyClassDeclarationBlock * RubyClassifierCodeDocument::getClassDecl()
{
    if (!classDeclCodeBlock)
    {
        classDeclCodeBlock = new RubyClassDeclarationBlock (this);
        classDeclCodeBlock->updateContent();
        classDeclCodeBlock->setTag(QLatin1String("ClassDeclBlock"));
    }
    return classDeclCodeBlock;
}

/**
 * Reset/clear our inventory of textblocks in this document.
 */
void RubyClassifierCodeDocument::resetTextBlocks()
{
    // all special pointers to text blocks need to be zero'd out
    operationsBlock = 0;
    constructorBlock = 0;
    classDeclCodeBlock = 0;

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
    Q_ASSERT(c != 0);
    if (c) {
        UMLOperationList list = c->getOpList();
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
    bool createdPublicBlock = publicBlock == 0 ? true : false;
    publicBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QLatin1String("publicBlock"), QLatin1String("Public Items"), 0);
    if (createdPublicBlock)
        publicBlock->setStartText(QLatin1String("public"));

    bool createdProtBlock = protectedBlock == 0 ? true : false;
    protectedBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QLatin1String("protectedBlock"), QLatin1String("Protected Items"), 0);
    if (createdProtBlock)
        protectedBlock->setStartText(QLatin1String("protected"));

    bool createdPrivBlock = privateBlock == 0 ? true : false;
    privateBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QLatin1String("privateBlock"), QLatin1String("Private Items"), 0);
    if (createdPrivBlock)
        privateBlock->setStartText(QLatin1String("private"));

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
    HierarchicalCodeBlock * pubMethodsBlock = publicBlock->getHierarchicalCodeBlock(QLatin1String("pubMethodsBlock"), QString(), 1);
    CodeComment * pubMethodsComment = pubMethodsBlock->getComment();
    bool forceDoc = pol->getCodeVerboseDocumentComments();
    // set conditions for showing this comment
    if (!forceDoc && !hasClassFields() && !hasOperationMethods)
        pubMethodsComment->setWriteOutText(false);
    else
        pubMethodsComment->setWriteOutText(true);

    // protected methods
    HierarchicalCodeBlock * protMethodsBlock = protectedBlock->getHierarchicalCodeBlock(QLatin1String("protMethodsBlock"), QString(), 1);
    CodeComment * protMethodsComment = protMethodsBlock->getComment();
    // set conditions for showing this comment
    if (!forceDoc && !hasClassFields() && !hasOperationMethods)
        protMethodsComment->setWriteOutText(false);
    else
        protMethodsComment->setWriteOutText(true);

    // private methods
    HierarchicalCodeBlock * privMethodsBlock = privateBlock->getHierarchicalCodeBlock(QLatin1String("privMethodsBlock"), QString(), 1);
    CodeComment * privMethodsComment = privMethodsBlock->getComment();
    // set conditions for showing this comment
    if (!forceDoc && !hasClassFields() && !hasOperationMethods)
        privMethodsComment->setWriteOutText(false);
    else
        privMethodsComment->setWriteOutText(true);

    // METHODS sub-section : constructor methods
    //

    // public
    pubConstructorBlock = pubMethodsBlock->getHierarchicalCodeBlock(QLatin1String("constructionMethods"), QLatin1String("Constructors"), 1);
    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * pubConstComment = pubConstructorBlock->getComment();
    if (!forceDoc && (isInterface || !pol->getAutoGenerateConstructors()))
        pubConstComment->setWriteOutText(false);
    else
        pubConstComment->setWriteOutText(true);

    // protected
    protConstructorBlock = protMethodsBlock->getHierarchicalCodeBlock(QLatin1String("constructionMethods"), QLatin1String("Constructors"), 1);
    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * protConstComment = protConstructorBlock->getComment();
    if (!forceDoc && (isInterface || !pol->getAutoGenerateConstructors()))
        protConstComment->setWriteOutText(false);
    else
        protConstComment->setWriteOutText(true);

    // private
    privConstructorBlock = privMethodsBlock->getHierarchicalCodeBlock(QLatin1String("constructionMethods"), QLatin1String("Constructors"), 1);
    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * privConstComment = privConstructorBlock->getComment();
    if (!forceDoc && (isInterface || !pol->getAutoGenerateConstructors()))
        privConstComment->setWriteOutText(false);
    else
        privConstComment->setWriteOutText(true);

    // get/create the accessor codeblock
    // public
    HierarchicalCodeBlock * pubAccessorBlock = pubMethodsBlock->getHierarchicalCodeBlock(QLatin1String("accessorMethods"), QLatin1String("Accessor Methods"), 1);
    // set conditions for showing section comment
    CodeComment * pubAccessComment = pubAccessorBlock->getComment();
    if (!forceDoc && !hasClassFields())
        pubAccessComment->setWriteOutText(false);
    else
        pubAccessComment->setWriteOutText(true);

    // protected
    HierarchicalCodeBlock * protAccessorBlock = protMethodsBlock->getHierarchicalCodeBlock(QLatin1String("accessorMethods"), QLatin1String("Accessor Methods"), 1);
    // set conditions for showing section comment
    CodeComment * protAccessComment = protAccessorBlock->getComment();
    if (!forceDoc && !hasClassFields())
        protAccessComment->setWriteOutText(false);
    else
        protAccessComment->setWriteOutText(true);

    // private
    HierarchicalCodeBlock * privAccessorBlock = privMethodsBlock->getHierarchicalCodeBlock(QLatin1String("accessorMethods"), QLatin1String("Accessor Methods"), 1);
    // set conditions for showing section comment
    CodeComment * privAccessComment = privAccessorBlock->getComment();
    if (!forceDoc && !hasClassFields())
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
    pubOperationsBlock = pubMethodsBlock->getHierarchicalCodeBlock(QLatin1String("operationMethods"), QLatin1String("Operations"), 1);
    // set conditions for showing section comment
    CodeComment * pubOcomment = pubOperationsBlock->getComment();
    if (!forceDoc && !hasOperationMethods)
        pubOcomment->setWriteOutText(false);
    else
        pubOcomment->setWriteOutText(true);

    //protected
    protOperationsBlock = protMethodsBlock->getHierarchicalCodeBlock(QLatin1String("operationMethods"), QLatin1String("Operations"), 1);
    // set conditions for showing section comment
    CodeComment * protOcomment = protOperationsBlock->getComment();
    if (!forceDoc && !hasOperationMethods)
        protOcomment->setWriteOutText(false);
    else
        protOcomment->setWriteOutText(true);

    //private
    privOperationsBlock = privMethodsBlock->getHierarchicalCodeBlock(QLatin1String("operationMethods"), QLatin1String("Operations"), 1);
    // set conditions for showing section comment
    CodeComment * privOcomment = privOperationsBlock->getComment();
    if (!forceDoc && !hasOperationMethods)
        privOcomment->setWriteOutText(false);
    else
        privOcomment->setWriteOutText(true);
}


