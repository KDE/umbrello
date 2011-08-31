/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

/**
 * We carve the Java document up into sections as follows:
 * - header
 * - package declaration
 * - import statements
 * - class declaration
 * -   guts of the class (e.g. field decl, accessor methods, operations, dependant classes)
 */

// own header
#include "javaclassifiercodedocument.h"

// local includes
#include "javacodegenerator.h"
#include "javacodecomment.h"
#include "javaclassdeclarationblock.h"
#include "javacodeclassfielddeclarationblock.h"
#include "codegen_utils.h"
#include "classifier.h"
#include "codegenerationpolicy.h"
#include "debug_utils.h"
#include "uml.h"

// qt includes
#include <QtCore/QRegExp>

JavaClassifierCodeDocument::JavaClassifierCodeDocument ( UMLClassifier * concept )
        : ClassifierCodeDocument (concept)
{
    init();
}

JavaClassifierCodeDocument::~JavaClassifierCodeDocument ( )
{
}

// Make it easier on ourselves
JavaCodeGenerationPolicy * JavaClassifierCodeDocument::getJavaPolicy()
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
CodeDocumentDialog JavaClassifierCodeDocument::getDialog ( )
{
}
*/

bool JavaClassifierCodeDocument::forceDoc ()
{
    return UMLApp::app()->commonPolicy()->getCodeVerboseDocumentComments();
}

// We overwritten by Java language implementation to get lowercase path
QString JavaClassifierCodeDocument::getPath ( )
{
    QString path = getPackage();

    // Replace all white spaces with blanks
    path = path.simplified();

    // Replace all blanks with underscore
    path.replace(QRegExp(" "), "_");

    path.replace(QRegExp("\\."),"/");
    path.replace(QRegExp("::"), "/");

    path.toLower();

    return path;
}

QString JavaClassifierCodeDocument::getJavaClassName (const QString &name)
{
    return Codegen_Utils::capitalizeFirstLetter(CodeGenerator::cleanName(name));
}

// Initialize this java classifier code document
void JavaClassifierCodeDocument::init ( )
{
    setFileExtension(".java");

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
 * @param       op
 */
// in the vannilla version, we just tack all operations on the end
// of the document
bool JavaClassifierCodeDocument::addCodeOperation (CodeOperation * op )
{
    if(!op->getParentOperation()->isLifeOperation())
        return operationsBlock->addTextBlock(op);
    else
        return constructorBlock->addTextBlock(op);
}

// Sigh. NOT optimal. The only reason that we need to have this
// is so we can create the JavaClassDeclarationBlock.
// would be better if we could create a handler interface that each
// codeblock used so all we have to do here is add the handler
// for "javaclassdeclarationblock"
void JavaClassifierCodeDocument::loadChildTextBlocksFromNode ( QDomElement & root)
{
    QDomNode tnode = root.firstChild();
    QDomElement telement = tnode.toElement();
    bool loadCheckForChildrenOK = false;
    while( !telement.isNull() ) {
        QString nodeName = telement.tagName();

        if( nodeName == "textblocks" ) {

            QDomNode node = telement.firstChild();
            QDomElement element = node.toElement();

            // if there is nothing to begin with, then we don't worry about it
            loadCheckForChildrenOK = element.isNull() ? true : false;

            while( !element.isNull() ) {
                QString name = element.tagName();

                if( name == "codecomment" ) {
                    CodeComment * block = new JavaCodeComment(this);
                    block->loadFromXMI(element);
                    if(!addTextBlock(block))
                    {
                        uError()<<"loadFromXMI : unable to add codeComment to :"<<this;
                        delete block;
                    } else
                        loadCheckForChildrenOK= true;
                } else
                    if( name == "codeaccessormethod" ||
                            name == "ccfdeclarationcodeblock"
                      ) {
                        QString acctag = element.attribute("tag","");
                        // search for our method in the
                        TextBlock * tb = findCodeClassFieldTextBlockByTag(acctag);
                        if(!tb || !addTextBlock(tb))
                        {
                            uError()<<"loadFromXMI : unable to add codeclassfield child method to:"<<this;
                            // DON'T delete
                        } else
                            loadCheckForChildrenOK= true;

                    } else
                        if( name == "codeblock" ) {
                            CodeBlock * block = newCodeBlock();
                            block->loadFromXMI(element);
                            if(!addTextBlock(block))
                            {
                                uError()<<"loadFromXMI : unable to add codeBlock to :"<<this;
                                delete block;
                            } else
                                loadCheckForChildrenOK= true;
                        } else
                            if( name == "codeblockwithcomments" ) {
                                CodeBlockWithComments * block = newCodeBlockWithComments();
                                block->loadFromXMI(element);
                                if(!addTextBlock(block))
                                {
                                    uError()<<"loadFromXMI : unable to add codeBlockwithcomments to:"<<this;
                                    delete block;
                                } else
                                    loadCheckForChildrenOK= true;
                            } else
                                if( name == "header" ) {
                                    // do nothing.. this is treated elsewhere
                                } else
                                    if( name == "hierarchicalcodeblock" ) {
                                        HierarchicalCodeBlock * block = newHierarchicalCodeBlock();
                                        block->loadFromXMI(element);
                                        if(!addTextBlock(block))
                                        {
                                            uError()<<"Unable to add hierarchicalcodeBlock to:"<<this;
                                            delete block;
                                        } else
                                            loadCheckForChildrenOK= true;
                                    } else
                                        if( name == "codeoperation" ) {
                                            // find the code operation by id
                                            QString id = element.attribute("parent_id","-1");
                                            UMLObject * obj = UMLApp::app()->document()->findObjectById(STR2ID(id));
                                            UMLOperation * op = dynamic_cast<UMLOperation*>(obj);
                                            if(op) {
                                                CodeOperation * block = new JavaCodeOperation(this, op);
                                                block->loadFromXMI(element);
                                                if(addTextBlock(block))
                                                    loadCheckForChildrenOK= true;
                                                else
                                                {
                                                    uError()<<"Unable to add codeoperation to:"<<this;
                                                    block->deleteLater();
                                                }
                                            } else
                                                uError()<<"Unable to find operation create codeoperation for:"<<this;
                                        } else
                                            if( name == "javaclassdeclarationblock" )
                                            {
                                                JavaClassDeclarationBlock * block = getClassDecl();
                                                block->loadFromXMI(element);
                                                if(!addTextBlock(block))
                                                {
                                                    uError()<<"Unable to add java code declaration block to:"<<this;
                                                    // DON'T delete.
                                                    // block->deleteLater();
                                                } else
                                                    loadCheckForChildrenOK= true;
                                            }
                // This last item is only needed for extreme debugging conditions
                // (E.g. making new codeclassdocument loader)
                // else
                //        uDebug()<<" LoadFromXMI: Got strange tag in text block stack:"<<name<<", ignorning";

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
        CodeDocument * test = dynamic_cast<CodeDocument*>(this);
        if(test)
        {
            uWarning()<<" loadChildBlocks : unable to initialize any child blocks in doc: "<<test->getFileName()<<" "<<this;
        } else {
            HierarchicalCodeBlock * hb = dynamic_cast<HierarchicalCodeBlock*>(this);
            if(hb)
                uWarning()<<" loadChildBlocks : unable to initialize any child blocks in Hblock: "<<hb->getTag()<<" "<<this;
            else
                uDebug()<<" loadChildBlocks : unable to initialize any child blocks in UNKNOWN OBJ:"<<this;
        }
    }
}

JavaClassDeclarationBlock * JavaClassifierCodeDocument::getClassDecl()
{
    if(!classDeclCodeBlock)
    {
        classDeclCodeBlock = new JavaClassDeclarationBlock (this);
        classDeclCodeBlock->updateContent();
        classDeclCodeBlock->setTag("ClassDeclBlock");
    }
    return classDeclCodeBlock;
}

void JavaClassifierCodeDocument::resetTextBlocks()
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
// such, we will want to insert everything we resonablely will want
// during creation. We can set various parts of the document (esp. the
// comments) to appear or not, as needed.
void JavaClassifierCodeDocument::updateContent( )
{
    // Gather info on the various fields and parent objects of this class...
    UMLClassifier * c = getParentClassifier();
    CodeGenerationPolicy * commonPolicy = UMLApp::app()->commonPolicy();
    CodeGenPolicyExt * pe = UMLApp::app()->policyExt();
    JavaCodeGenerationPolicy * policy = dynamic_cast<JavaCodeGenerationPolicy*>(pe);

    // first, set the global flag on whether or not to show classfield info
    // This depends on whether or not we have attribute/association classes
    const CodeClassFieldList * cfList = getCodeClassFieldList();
    CodeClassFieldList::const_iterator it = cfList->begin();
    CodeClassFieldList::const_iterator end = cfList->end();
    for( ; it != end; ++it ) {
        CodeClassField * field = *it;
        if(field->parentIsAttribute())
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
    CodeClassFieldList plainAssocClassFields = getSpecificClassFields ( CodeClassField::PlainAssociation );
    CodeClassFieldList aggregationClassFields = getSpecificClassFields ( CodeClassField::Aggregation );
    CodeClassFieldList compositionClassFields = getSpecificClassFields ( CodeClassField::Composition );

    bool isInterface = parentIsInterface();
    bool hasOperationMethods = false;
    Q_ASSERT(c != NULL);
    if (c) {
        UMLOperationList list = c->getOpList();
        hasOperationMethods = ! list.isEmpty();
    }
    QString endLine = commonPolicy->getNewLineEndingChars(); // a shortcut..so we don't have to call this all the time

    //
    // START GENERATING CODE/TEXT BLOCKS and COMMENTS FOR THE DOCUMENT
    //

    //
    // PACKAGE CODE BLOCK
    //
    QString pkgs = getPackage();
    pkgs.replace(QRegExp("::"), ".");
    QString packageText = getPackage().isEmpty() ? "" : QString(QLatin1String("package ")+pkgs+QLatin1Char(';')+endLine);
    CodeBlockWithComments * pblock = addOrUpdateTaggedCodeBlockWithComments("packages", packageText, "", 0, false);
    if(packageText.isEmpty() && pblock->contentType() == CodeBlock::AutoGenerated)
        pblock->setWriteOutText(false);
    else
        pblock->setWriteOutText(true);

    // IMPORT CODEBLOCK
    //
    // Q: Why all utils? Aren't just List and Vector the only classes we are using?
    // A: doesn't matter at all; it is more readable to just include '*' and java compilers
    //    don't slow down or anything. (TZ)
    QString importStatement = "";
    if ( hasObjectVectorClassFields() )
        importStatement.append("import java.util.*;");

    //only import classes in a different package from this class
    UMLPackageList imports;
    QMap<UMLPackage*, QString> packageMap; // so we don't repeat packages

    CodeGenerator::findObjectsRelated(c,imports);
    UMLPackageListIt importsIt( imports );
    while(importsIt.hasNext()) {
        UMLPackage* con = importsIt.next();
        // NO (default) datatypes in the import statement.. use defined
        // ones whould be possible, but no idea how to do that...at least for now.
        // Dynamic casting is slow..not an optimal way to do this.
        if (!packageMap.contains(con) && con->baseType() != UMLObject::ot_Datatype)
        {
            packageMap.insert(con, con->package());

            // now, we DON'T need to import classes that are already in our own package
            // (that is, IF a package is specified). Otherwise, we should have a declaration.
            if (con->package() != c->package() ||
                    (c->package().isEmpty() && con->package().isEmpty()))
            {
                importStatement.append(endLine+"import ");
                if(!con->package().isEmpty())
                    importStatement.append(con->package()+'.');
                importStatement.append(CodeGenerator::cleanName(con->name())+';');
            }
        }
    }
    // now, add/update the imports codeblock
    CodeBlockWithComments * iblock = addOrUpdateTaggedCodeBlockWithComments("imports", importStatement, "", 0, false);
    if(importStatement.isEmpty() && iblock->contentType() == CodeBlock::AutoGenerated)
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
    HierarchicalCodeBlock * fieldDeclBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock("fieldsDecl", "Fields", 1);

    // Update the comment: we only set comment to appear under the following conditions
    CodeComment * fcomment = fieldDeclBlock->getComment();
    if (isInterface || (!forceDoc() && !hasClassFields()) )
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
    HierarchicalCodeBlock * methodsBlock = myClassDeclCodeBlock->getHierarchicalCodeBlock("methodsBlock", "Methods", 1);

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
    HierarchicalCodeBlock * constBlock = methodsBlock->getHierarchicalCodeBlock("constructorMethods", "Constructors", 1);
    constructorBlock = constBlock; // record this codeblock for later, when operations are updated

    // special condiions for showing comment: only when autogenerateding empty constructors
    // Although, we *should* check for other constructor methods too
    CodeComment * constComment = constBlock->getComment();
    CodeGenerationPolicy *pol = UMLApp::app()->commonPolicy();
    if (!forceDoc() && (isInterface || !pol->getAutoGenerateConstructors()))
        constComment->setWriteOutText(false);
    else
        constComment->setWriteOutText(true);

    // add/get the empty constructor
    QString JavaClassName = getJavaClassName(c->name());
    QString emptyConstStatement = "public "+JavaClassName+" ( ) { }";
    CodeBlockWithComments * emptyConstBlock =
        constBlock->addOrUpdateTaggedCodeBlockWithComments("emptyconstructor", emptyConstStatement, "Empty Constructor", 1, false);
    // Now, as an additional condition we only show the empty constructor block
    // IF it was desired to be shown
    if(parentIsClass() && pol->getAutoGenerateConstructors())
        emptyConstBlock->setWriteOutText(true);
    else
        emptyConstBlock->setWriteOutText(false);

    // METHODS subsection : ACCESSOR METHODS
    //

    // get/create the accessor codeblock
    HierarchicalCodeBlock * accessorBlock = methodsBlock->getHierarchicalCodeBlock("accessorMethods", "Accessor Methods", 1);

    // set conditions for showing section comment
    CodeComment * accessComment = accessorBlock->getComment();
    if (!forceDoc() && !hasClassFields())
        accessComment->setWriteOutText(false);
    else
        accessComment->setWriteOutText(true);

    // now, 2 sub-sub sections in accessor block
    // add/update accessor methods for attributes
    HierarchicalCodeBlock * staticAccessors = accessorBlock->getHierarchicalCodeBlock("staticAccessorMethods", "", 1);
    staticAccessors->getComment()->setWriteOutText(false); // never write block comment
    staticAccessors->addCodeClassFieldMethods(staticAttribClassFields);
    staticAccessors->addCodeClassFieldMethods(attribClassFields);

    // add/update accessor methods for associations
    HierarchicalCodeBlock * regularAccessors = accessorBlock->getHierarchicalCodeBlock("regularAccessorMethods", "", 1);
    regularAccessors->getComment()->setWriteOutText(false); // never write block comment
    regularAccessors->addCodeClassFieldMethods(plainAssocClassFields);
    regularAccessors->addCodeClassFieldMethods(aggregationClassFields);
    regularAccessors->addCodeClassFieldMethods(compositionClassFields);

    // METHODS subsection : Operation methods (which arent constructors)
    //

    // get/create the operations codeblock
    operationsBlock = methodsBlock->getHierarchicalCodeBlock("operationMethods", "Operations", 1);

    // set conditions for showing section comment
    CodeComment * ocomment = operationsBlock->getComment();
    if (!forceDoc() && !hasOperationMethods )
        ocomment->setWriteOutText(false);
    else
        ocomment->setWriteOutText(true);
}


#include "javaclassifiercodedocument.moc"
