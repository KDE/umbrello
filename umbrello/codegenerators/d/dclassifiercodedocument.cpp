/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dclassifiercodedocument.h"

// local includes
#include "dcodegenerator.h"
#include "dcodecomment.h"
#include "dclassdeclarationblock.h"
#include "dcodeclassfielddeclarationblock.h"
#include "codegen_utils.h"
#include "classifier.h"
#include "codegenerationpolicy.h"
#include "debug_utils.h"
#include "uml.h"

// qt includes
#include <QRegExp>

/**
 * Constructor.
 */
DClassifierCodeDocument::DClassifierCodeDocument(UMLClassifier * concept)
        : ClassifierCodeDocument(concept)
{
    init();
}

/**
 * Empty Destructor.
 */
DClassifierCodeDocument::~DClassifierCodeDocument()
{
}

// Make it easier on ourselves
DCodeGenerationPolicy * DClassifierCodeDocument::getDPolicy()
{
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    DCodeGenerationPolicy * policy = dynamic_cast<DCodeGenerationPolicy*>(pe);
    return policy;
}

//    /**
//     * Get the dialog widget which allows user interaction with the object parameters.
//     * @return  CodeDocumentDialog
//     */
/*
CodeDocumentDialog DClassifierCodeDocument::getDialog()
{
}
*/

bool DClassifierCodeDocument::forceDoc()
{
    return UMLApp::app()->commonPolicy()->getCodeVerboseDocumentComments();
}

// We overwritten by D language implementation to get lowercase path
QString DClassifierCodeDocument::getPath() const
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

QString DClassifierCodeDocument::getDClassName(const QString &name) const
{
    return Codegen_Utils::capitalizeFirstLetter(CodeGenerator::cleanName(name));
}

// Initialize this d classifier code document
void DClassifierCodeDocument::init()
{
    setFileExtension(QLatin1String(".d"));

    //initCodeClassFields(); // this is dubious because it calls down to
                             // CodeGenFactory::newCodeClassField(this)
                             // but "this" is still in construction at that time.

    classDeclCodeBlock = 0;
    operationsBlock = 0;
    constructorBlock = 0;

    // this will call updateContent() as well as other things that sync our document.
    synchronize();
}

/**
 * Add a code operation to this d classifier code document.
 * In the vanilla version, we just tack all operations on the end
 * of the document.
 * @param op   the code operation
 * @return bool which is true IF the code operation was added successfully
 */
bool DClassifierCodeDocument::addCodeOperation(CodeOperation * op)
{
    if (!op->getParentOperation()->isLifeOperation())
        return operationsBlock->addTextBlock(op);
    else
        return constructorBlock->addTextBlock(op);
}

/**
 * Need to overwrite this for d since we need to pick up the
 * d class declaration block.
 * Sigh. NOT optimal. The only reason that we need to have this
 * is so we can create the DClassDeclarationBlock.
 * would be better if we could create a handler interface that each
 * codeblock used so all we have to do here is add the handler
 * for "dclassdeclarationblock".
 */
void DClassifierCodeDocument::loadChildTextBlocksFromNode(QDomElement & root)
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
                    CodeComment * block = new DCodeComment(this);
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("loadFromXMI : unable to add codeComment");
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
                        logError0("loadFromXMI : unable to add codeclassfield child method");
                        // DON'T delete
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QLatin1String("codeblock")) {
                    CodeBlock * block = newCodeBlock();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("loadFromXMI : unable to add codeBlock");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QLatin1String("codeblockwithcomments")) {
                    CodeBlockWithComments * block = newCodeBlockWithComments();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("loadFromXMI : unable to add codeBlockwithcomments");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QLatin1String("header")) {
                    // do nothing.. this is treated elsewhere
                } else if (name == QLatin1String("hierarchicalcodeblock")) {
                    HierarchicalCodeBlock * block = newHierarchicalCodeBlock();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("Unable to add hierarchicalcodeBlock");
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
                        CodeOperation * block = new DCodeOperation(this, op);
                        block->loadFromXMI(element);
                        if (addTextBlock(block)) {
                            loadCheckForChildrenOK= true;
                        } else {
                            logError0("Unable to add codeoperation");
                            block->deleteLater();
                        }
                    } else {
                        logError0("Unable to find operation create codeoperation");
                    }
                } else if (name == QLatin1String("dclassdeclarationblock")) {
                    DClassDeclarationBlock * block = getClassDecl();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("Unable to add d code declaration block");
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
        logWarn1("loadChildBlocks : unable to initialize any child blocks in doc: %1", getFileName());
    }
}

DClassDeclarationBlock * DClassifierCodeDocument::getClassDecl()
{
    if (!classDeclCodeBlock)
    {
        classDeclCodeBlock = new DClassDeclarationBlock (this);
        classDeclCodeBlock->updateContent();
        classDeclCodeBlock->setTag(QLatin1String("ClassDeclBlock"));
    }
    return classDeclCodeBlock;
}

/**
 * Reset/clear our inventory of textblocks in this document.
 */
void DClassifierCodeDocument::resetTextBlocks()
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
void DClassifierCodeDocument::updateContent()
{
    // Gather info on the various fields and parent objects of this class...
    UMLClassifier * c = getParentClassifier();
    Q_ASSERT(c != 0);
    CodeGenerationPolicy * commonPolicy = UMLApp::app()->commonPolicy();
    CodeGenPolicyExt * pe = UMLApp::app()->policyExt();
    DCodeGenerationPolicy * policy = dynamic_cast<DCodeGenerationPolicy*>(pe);

    // first, set the global flag on whether or not to show classfield info
    // This depends on whether or not we have attribute/association classes
    const CodeClassFieldList * cfList = getCodeClassFieldList();
    CodeClassFieldList::const_iterator it = cfList->begin();
    CodeClassFieldList::const_iterator end = cfList->end();
    for (; it != end; ++it) {
        CodeClassField * field = *it;
        if (field->parentIsAttribute())
            field->setWriteOutMethods(policy->getAutoGenerateAttribAccessors());
        else
            field->setWriteOutMethods(policy->getAutoGenerateAssocAccessors());
    }

    // attribute-based ClassFields
    // we do it this way to have the static fields sorted out from regular ones
    CodeClassFieldList staticAttribClassFields = getSpecificClassFields (CodeClassField::Attribute, true);
    CodeClassFieldList attribClassFields = getSpecificClassFields (CodeClassField::Attribute, false);
    // association-based ClassFields
    // don't care if they are static or not..all are lumped together
    CodeClassFieldList plainAssocClassFields = getSpecificClassFields (CodeClassField::PlainAssociation);
    CodeClassFieldList aggregationClassFields = getSpecificClassFields (CodeClassField::Aggregation);
    CodeClassFieldList compositionClassFields = getSpecificClassFields (CodeClassField::Composition);

    bool isInterface = parentIsInterface();
    bool hasOperationMethods = false;

    UMLOperationList list = c->getOpList();
    hasOperationMethods = ! list.isEmpty();

    QString endLine = commonPolicy->getNewLineEndingChars(); // a shortcut..so we don't have to call this all the time

    //
    // START GENERATING CODE/TEXT BLOCKS and COMMENTS FOR THE DOCUMENT
    //

    //
    // PACKAGE CODE BLOCK
    //
    QString pkgs = getPackage();
    pkgs.replace(QRegExp(QLatin1String("::")), QLatin1String("."));
    QString packageText = getPackage().isEmpty() ? QString() : QString(QLatin1String("package ")+pkgs+QLatin1Char(';')+endLine);
    CodeBlockWithComments * pblock = addOrUpdateTaggedCodeBlockWithComments(QLatin1String("packages"), packageText, QString(), 0, false);
    if (packageText.isEmpty() && pblock->contentType() == CodeBlock::AutoGenerated)
        pblock->setWriteOutText(false);
    else
        pblock->setWriteOutText(true);

    // IMPORT CODEBLOCK
    //
    // Q: Why all utils? Aren't just List and Vector the only classes we are using?
    // A: doesn't matter at all; it is more readable to just include '*' and d compilers
    //    don't slow down or anything. (TZ)
    QString importStatement;
    if (hasObjectVectorClassFields())
        importStatement.append(QLatin1String("import d.util.*;"));

    //only import classes in a different package from this class
    UMLPackageList imports;
    QMap<UMLPackage*, QString> packageMap; // so we don't repeat packages

    CodeGenerator::findObjectsRelated(c, imports);
    for (UMLPackageListIt importsIt(imports); importsIt.hasNext();) {
        UMLPackage *con = importsIt.next();
        // NO (default) datatypes in the import statement.. use defined
        // ones whould be possible, but no idea how to do that...at least for now.
        // Dynamic casting is slow..not an optimal way to do this.
        if (!packageMap.contains(con) && !con->isUMLDatatype())
        {
            packageMap.insert(con, con->package());

            // now, we DON'T need to import classes that are already in our own package
            // (that is, IF a package is specified). Otherwise, we should have a declaration.
            if (con->package() != c->package() ||
                    (c->package().isEmpty() && con->package().isEmpty()))
            {
                importStatement.append(endLine+QLatin1String("import "));
                if (!con->package().isEmpty())
                    importStatement.append(con->package()+QLatin1Char('.'));
                importStatement.append(CodeGenerator::cleanName(con->name())+QLatin1Char(';'));
            }
        }
    }
    // now, add/update the imports codeblock
    CodeBlockWithComments * iblock = addOrUpdateTaggedCodeBlockWithComments(QLatin1String("imports"), importStatement, QString(), 0, false);
    if (importStatement.isEmpty() && iblock->contentType() == CodeBlock::AutoGenerated)
        iblock->setWriteOutText(false);
    else
        iblock->setWriteOutText(true);

    // CLASS DECLARATION BLOCK
    //

    // get the declaration block. If it is not already present, add it too
    DClassDeclarationBlock * myClassDeclCodeBlock = getClassDecl();
    addTextBlock(myClassDeclCodeBlock); // note: wont add if already present

    // NOW create document in sections..
    // now we want to populate the body of our class
    // our layout is the following general groupings of code blocks:

    // start d classifier document

    // header comment

    // package code block

    // import code block

    // class declaration

    //   section:
    //   - class field declaration section comment
    //   - class field declarations (0+ codeblocks)

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

    // end d classifier document


    // Q: Why use the more complicated scheme of arranging code blocks within codeblocks?
    // A: This will allow us later to preserve the format of our document so that if
    //    codeblocks are added, they may be easily added in the correct place, rather than at
    //    the end of the document, or by using a difficult algorithm to find the location of
    //    the last appropriate code block sibling (which may not exist.. for example user adds
    //    a constructor operation, but there currently are no constructor code blocks
    //    within the document).

    //
    // * CLASS FIELD declaration section
    //

    // get/create the field declaration code block
    HierarchicalCodeBlock * fieldDeclBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QLatin1String("fieldsDecl"), QLatin1String("Fields"), 1);

    // Update the comment: we only set comment to appear under the following conditions
    CodeComment * fcomment = fieldDeclBlock->getComment();
    if (isInterface || (!forceDoc() && !hasClassFields()))
        fcomment->setWriteOutText(false);
    else
        fcomment->setWriteOutText(true);

    // now actually declare the fields within the appropriate HCodeBlock
    declareClassFields(staticAttribClassFields, fieldDeclBlock);
    declareClassFields(attribClassFields, fieldDeclBlock);
    declareClassFields(plainAssocClassFields, fieldDeclBlock);
    declareClassFields(aggregationClassFields, fieldDeclBlock);
    declareClassFields(compositionClassFields, fieldDeclBlock);

    //
    // METHODS section
    //

    // get/create the method codeblock
    HierarchicalCodeBlock * methodsBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QLatin1String("methodsBlock"), QLatin1String("Methods"), 1);

    // Update the section comment
    CodeComment * methodsComment = methodsBlock->getComment();
    // set conditions for showing this comment
    if (!forceDoc() && !hasClassFields() && !hasOperationMethods)
        methodsComment->setWriteOutText(false);
    else
        methodsComment->setWriteOutText(true);

    // METHODS sub-section : constructor methods
    //

    // get/create the constructor codeblock
    HierarchicalCodeBlock * constBlock = methodsBlock->getHierarchicalCodeBlock(QLatin1String("constructorMethods"), QLatin1String("Constructors"), 1);
    constructorBlock = constBlock; // record this codeblock for later, when operations are updated

    // special conditions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * constComment = constBlock->getComment();
    CodeGenerationPolicy *pol = UMLApp::app()->commonPolicy();
    if (!forceDoc() && (isInterface || !pol->getAutoGenerateConstructors()))
        constComment->setWriteOutText(false);
    else
        constComment->setWriteOutText(true);

    // add/get the empty constructor
    QString DClassName = getDClassName(c->name());
    QString emptyConstStatement = QLatin1String("public ")+DClassName+QLatin1String(" () { }");
    CodeBlockWithComments * emptyConstBlock =
        constBlock->addOrUpdateTaggedCodeBlockWithComments(QLatin1String("emptyconstructor"), emptyConstStatement, QLatin1String("Empty Constructor"), 1, false);
    // Now, as an additional condition we only show the empty constructor block
    // IF it was desired to be shown
    if (parentIsClass() && pol->getAutoGenerateConstructors())
        emptyConstBlock->setWriteOutText(true);
    else
        emptyConstBlock->setWriteOutText(false);

    // METHODS subsection : ACCESSOR METHODS
    //

    // get/create the accessor codeblock
    HierarchicalCodeBlock * accessorBlock = methodsBlock->getHierarchicalCodeBlock(QLatin1String("accessorMethods"), QLatin1String("Accessor Methods"), 1);

    // set conditions for showing section comment
    CodeComment * accessComment = accessorBlock->getComment();
    if (!forceDoc() && !hasClassFields())
        accessComment->setWriteOutText(false);
    else
        accessComment->setWriteOutText(true);

    // now, 2 sub-sub sections in accessor block
    // add/update accessor methods for attributes
    HierarchicalCodeBlock * staticAccessors = accessorBlock->getHierarchicalCodeBlock(QLatin1String("staticAccessorMethods"), QString(), 1);
    staticAccessors->getComment()->setWriteOutText(false); // never write block comment
    staticAccessors->addCodeClassFieldMethods(staticAttribClassFields);
    staticAccessors->addCodeClassFieldMethods(attribClassFields);

    // add/update accessor methods for associations
    HierarchicalCodeBlock * regularAccessors = accessorBlock->getHierarchicalCodeBlock(QLatin1String("regularAccessorMethods"), QString(), 1);
    regularAccessors->getComment()->setWriteOutText(false); // never write block comment
    regularAccessors->addCodeClassFieldMethods(plainAssocClassFields);
    regularAccessors->addCodeClassFieldMethods(aggregationClassFields);
    regularAccessors->addCodeClassFieldMethods(compositionClassFields);

    // METHODS subsection : Operation methods (which arent constructors)
    //

    // get/create the operations codeblock
    operationsBlock = methodsBlock->getHierarchicalCodeBlock(QLatin1String("operationMethods"), QLatin1String("Operations"), 1);

    // set conditions for showing section comment
    CodeComment * ocomment = operationsBlock->getComment();
    if (!forceDoc() && !hasOperationMethods)
        ocomment->setWriteOutText(false);
    else
        ocomment->setWriteOutText(true);

}

