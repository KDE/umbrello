/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

/**
 * We carve the Java document up into sections as follows:
 * - header
 * - package declaration
 * - import statements
 * - class declaration
 * -   guts of the class (e.g. field decl, accessor methods, operations, dependent classes)
 */

// own header
#include "javaclassifiercodedocument.h"

// local includes
#include "javacodegenerator.h"
#include "javacodecomment.h"
#include "javaclassdeclarationblock.h"
#include "javacodeclassfielddeclarationblock.h"
#include "codegen_utils.h"
#include "umlclassifier.h"
#include "codegenerationpolicy.h"
#include "debug_utils.h"
#include "umlapp.h"

// qt includes
#include <QRegularExpression>

DEBUG_REGISTER(JavaClassifierCodeDocument)

JavaClassifierCodeDocument::JavaClassifierCodeDocument (UMLClassifier * classifier)
        : ClassifierCodeDocument (classifier)
{
    init();
}

JavaClassifierCodeDocument::~JavaClassifierCodeDocument ()
{
}

// Make it easier on ourselves
JavaCodeGenerationPolicy * JavaClassifierCodeDocument::getJavaPolicy() const
{
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    JavaCodeGenerationPolicy * policy = dynamic_cast<JavaCodeGenerationPolicy*>(pe);
    return policy;
}

//**
// * Get the dialog widget which allows user interaction with the object parameters.
// * @return      CodeDocumentDialog
// */
/*
CodeDocumentDialog JavaClassifierCodeDocument::getDialog ()
{
}
*/

bool JavaClassifierCodeDocument::forceDoc ()
{
    return UMLApp::app()->commonPolicy()->getCodeVerboseDocumentComments();
}

// We overwritten by Java language implementation to get lowercase path
QString JavaClassifierCodeDocument::getPath () const
{
    QString path = getPackage();

    // Replace all white spaces with blanks
    path = path.simplified();

    // Replace all blanks with underscore
    path.replace(QRegularExpression(QStringLiteral(" ")), QStringLiteral("_"));

    path.replace(QRegularExpression(QStringLiteral("\\.")),QStringLiteral("/"));
    path.replace(QRegularExpression(QStringLiteral("::")), QStringLiteral("/"));

    return path.toLower();
}

QString JavaClassifierCodeDocument::getJavaClassName (const QString &name) const
{
    return Codegen_Utils::capitalizeFirstLetter(CodeGenerator::cleanName(name));
}

// Initialize this java classifier code document
void JavaClassifierCodeDocument::init ()
{
    setFileExtension(QStringLiteral(".java"));

    //initCodeClassFields(); // this is dubious because it calls down to
                             // CodeGenFactory::newCodeClassField(this)
                             // but "this" is still in construction at that time.

    classDeclCodeBlock = nullptr;
    operationsBlock = nullptr;
    constructorBlock = nullptr;

    // this will call updateContent() as well as other things that sync our document.
    synchronize();
}

/**
 * @param       op
 */
// in the vanilla version, we just tack all operations on the end
// of the document
bool JavaClassifierCodeDocument::addCodeOperation (CodeOperation * op)
{
    if (!op->getParentOperation()->isLifeOperation())
        return operationsBlock == nullptr ? false : operationsBlock->addTextBlock(op);
    else
        return constructorBlock == nullptr ? false : constructorBlock->addTextBlock(op);
}

// Sigh. NOT optimal. The only reason that we need to have this
// is so we can create the JavaClassDeclarationBlock.
// would be better if we could create a handler interface that each
// codeblock used so all we have to do here is add the handler
// for "javaclassdeclarationblock"
void JavaClassifierCodeDocument::loadChildTextBlocksFromNode (QDomElement & root)
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
                    CodeComment * block = new JavaCodeComment(this);
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("JavaClassifierCodeDocument: loadFromXMI : unable to add codeComment");
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
                        logError0("JavaClassifierCodeDocument: loadFromXMI : unable to add codeclassfield child method");
                        // DON'T delete
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("codeblock")) {
                    CodeBlock * block = newCodeBlock();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("JavaClassifierCodeDocument: loadFromXMI : unable to add codeBlock");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("codeblockwithcomments")) {
                    CodeBlockWithComments * block = newCodeBlockWithComments();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("JavaClassifierCodeDocument: loadFromXMI : unable to add codeBlockwithcomments");
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
                        logError0("JavaClassifierCodeDocument: Unable to add hierarchicalcodeBlock");
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
                        CodeOperation * block = new JavaCodeOperation(this, op);
                        block->loadFromXMI(element);
                        if (addTextBlock(block)) {
                            loadCheckForChildrenOK= true;
                        } else {
                            logError0("JavaClassifierCodeDocument: Unable to add codeoperation");
                            block->deleteLater();
                        }
                    } else {
                        logError0("JavaClassifierCodeDocument: Unable to find operation create codeoperation");
                    }
                } else if (name == QStringLiteral("javaclassdeclarationblock")) {
                    JavaClassDeclarationBlock * block = getClassDecl();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("JavaClassifierCodeDocument: Unable to add java code declaration block");
                        // DON'T delete.
                        // block->deleteLater();
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else {
                    logDebug1("JavaClassifierCodeDocument::loadChildTextBlocksFromNode: Got strange tag in "
                              "text block stack:%1, ignoring", name);
                }

                node = element.nextSibling();
                element = node.toElement();
            }
            break;
        }

        tnode = telement.nextSibling();
        telement = tnode.toElement();
    }

    if (!loadCheckForChildrenOK) {
        logWarn1("loadChildBlocks : unable to initialize any child blocks in doc: %1", getFileName());
    }
}

JavaClassDeclarationBlock * JavaClassifierCodeDocument::getClassDecl()
{
    if (!classDeclCodeBlock) {
        classDeclCodeBlock = new JavaClassDeclarationBlock (this);
        classDeclCodeBlock->updateContent();
        classDeclCodeBlock->setTag(QStringLiteral("ClassDeclBlock"));
    }
    return classDeclCodeBlock;
}

void JavaClassifierCodeDocument::resetTextBlocks()
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
void JavaClassifierCodeDocument::updateContent()
{
    // Gather info on the various fields and parent objects of this class...
    UMLClassifier * c = getParentClassifier();
    Q_ASSERT(c != nullptr);
    CodeGenerationPolicy * commonPolicy = UMLApp::app()->commonPolicy();
    CodeGenPolicyExt * pe = UMLApp::app()->policyExt();
    JavaCodeGenerationPolicy * policy = dynamic_cast<JavaCodeGenerationPolicy*>(pe);

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
    pkgs.replace(QRegularExpression(QStringLiteral("::")), QStringLiteral("."));
    QString packageText = getPackage().isEmpty() ? QString() : QStringLiteral("package ") + pkgs + QLatin1Char(';') + endLine;
    CodeBlockWithComments * pblock = addOrUpdateTaggedCodeBlockWithComments(QStringLiteral("packages"), packageText, QString(), 0, false);
    if (packageText.isEmpty() && pblock->contentType() == CodeBlock::AutoGenerated)
        pblock->setWriteOutText(false);
    else
        pblock->setWriteOutText(true);

    // IMPORT CODEBLOCK
    //
    // Q: Why all utils? Aren't just List and Vector the only classes we are using?
    // A: doesn't matter at all; it is more readable to just include '*' and java compilers
    //    don't slow down or anything. (TZ)
    QString importStatement;
    if (hasObjectVectorClassFields())
        importStatement.append(QStringLiteral("import java.util.*;"));

    //only import classes in a different package from this class
    UMLPackageList imports;
    QMap<UMLPackage*, QString> packageMap; // so we don't repeat packages

    CodeGenerator::findObjectsRelated(c, imports);
    UMLPackageListIt importsIt(imports);
    while (importsIt.hasNext()) {
        UMLPackage* con = importsIt.next();
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
                importStatement.append(endLine+QStringLiteral("import "));
                if (!con->package().isEmpty())
                    importStatement.append(con->package()+QLatin1Char('.'));
                importStatement.append(CodeGenerator::cleanName(con->name())+QLatin1Char(';'));
            }
        }
    }
    // now, add/update the imports codeblock
    CodeBlockWithComments * iblock = addOrUpdateTaggedCodeBlockWithComments(QStringLiteral("imports"), importStatement, QString(), 0, false);
    if (importStatement.isEmpty() && iblock->contentType() == CodeBlock::AutoGenerated)
        iblock->setWriteOutText(false);
    else
        iblock->setWriteOutText(true);

    // CLASS DECLARATION BLOCK
    //

    // get the declaration block. If it is not already present, add it too
    JavaClassDeclarationBlock * myClassDeclCodeBlock = getClassDecl();
    addTextBlock(myClassDeclCodeBlock); // note: wont add if already present

    // NOW create document in sections..
    // now we want to populate the body of our class
    // our layout is the following general groupings of code blocks:

    // start java classifier document

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

    // end java classifier document


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
    HierarchicalCodeBlock * fieldDeclBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QStringLiteral("fieldsDecl"), QStringLiteral("Fields"), 1);

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
    HierarchicalCodeBlock * methodsBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock(QStringLiteral("methodsBlock"), QStringLiteral("Methods"), 1);

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
    HierarchicalCodeBlock * constBlock = methodsBlock->getHierarchicalCodeBlock(QStringLiteral("constructorMethods"), QStringLiteral("Constructors"), 1);
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
    QString JavaClassName = getJavaClassName(c->name());
    QString emptyConstStatement = QStringLiteral("public ") + JavaClassName + QStringLiteral(" () { }");
    CodeBlockWithComments * emptyConstBlock =
        constBlock->addOrUpdateTaggedCodeBlockWithComments(QStringLiteral("emptyconstructor"), emptyConstStatement, QStringLiteral("Empty Constructor"), 1, false);
    // Now, as an additional condition we only show the empty constructor block
    // IF it was desired to be shown
    if (parentIsClass() && pol->getAutoGenerateConstructors())
        emptyConstBlock->setWriteOutText(true);
    else
        emptyConstBlock->setWriteOutText(false);

    // METHODS subsection : ACCESSOR METHODS
    //

    // get/create the accessor codeblock
    HierarchicalCodeBlock * accessorBlock = methodsBlock->getHierarchicalCodeBlock(QStringLiteral("accessorMethods"), QStringLiteral("Accessor Methods"), 1);

    // set conditions for showing section comment
    CodeComment * accessComment = accessorBlock->getComment();
    if (!forceDoc() && !hasClassFields())
        accessComment->setWriteOutText(false);
    else
        accessComment->setWriteOutText(true);

    // now, 2 sub-sub sections in accessor block
    // add/update accessor methods for attributes
    HierarchicalCodeBlock * staticAccessors = accessorBlock->getHierarchicalCodeBlock(QStringLiteral("staticAccessorMethods"), QString(), 1);
    staticAccessors->getComment()->setWriteOutText(false); // never write block comment
    staticAccessors->addCodeClassFieldMethods(staticAttribClassFields);
    staticAccessors->addCodeClassFieldMethods(attribClassFields);

    // add/update accessor methods for associations
    HierarchicalCodeBlock * regularAccessors = accessorBlock->getHierarchicalCodeBlock(QStringLiteral("regularAccessorMethods"), QString(), 1);
    regularAccessors->getComment()->setWriteOutText(false); // never write block comment
    regularAccessors->addCodeClassFieldMethods(plainAssocClassFields);
    regularAccessors->addCodeClassFieldMethods(aggregationClassFields);
    regularAccessors->addCodeClassFieldMethods(compositionClassFields);

    // METHODS subsection : Operation methods (which arent constructors)
    //

    // get/create the operations codeblock
    operationsBlock = methodsBlock->getHierarchicalCodeBlock(QStringLiteral("operationMethods"), QStringLiteral("Operations"), 1);

    // set conditions for showing section comment
    CodeComment * ocomment = operationsBlock->getComment();
    if (!forceDoc() && !hasOperationMethods)
        ocomment->setWriteOutText(false);
    else
        ocomment->setWriteOutText(true);
}


