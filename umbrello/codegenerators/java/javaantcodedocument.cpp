/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "javaantcodedocument.h"

// local includes
#include "debug_utils.h"
#include "javacodegenerator.h"
#include "xmlcodecomment.h"
#include "xmlelementcodeblock.h"
#include "codegenfactory.h"
#include "umldoc.h"
#include "umlapp.h"

// qt includes
#include <QRegularExpression>
#include <QXmlStreamWriter>

DEBUG_REGISTER(JavaANTCodeDocument)

JavaANTCodeDocument::JavaANTCodeDocument ()
{
    setFileName(QStringLiteral("build")); // default name
    setFileExtension(QStringLiteral(".xml"));
    setID(QStringLiteral("ANTDOC")); // default id tag for this type of document
}

JavaANTCodeDocument::~JavaANTCodeDocument ()
{
}

//**
// * create a new CodeBlockWithComments object belonging to this CodeDocument.
// * @return      CodeBlockWithComments
// */
/*
CodeBlockWithComments * JavaANTCodeDocument::newCodeBlockWithComments ()
{
        return new XMLElementCodeBlock(this,"empty");
}
*/

HierarchicalCodeBlock * JavaANTCodeDocument::newHierarchicalCodeBlock ()
{
    return new XMLElementCodeBlock(this, QStringLiteral("empty"));
}

// Sigh. NOT optimal. The only reason that we need to have this
// is so we can create the XMLNodes, if needed.
// would be better if we could create a handler interface that each
// codeblock used so all we have to do here is add the handler
void JavaANTCodeDocument::loadChildTextBlocksFromNode (QDomElement & root)
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
                    CodeComment * block = new XMLCodeComment(this);
                    block->loadFromXMI(element);
                    if (!addTextBlock(block))
                    {
                        logError0("JavaANTCodeDocument: Unable to add codeComment");
                        delete block;
                    } else
                        loadCheckForChildrenOK= true;
                } else if (name == QStringLiteral("codeaccessormethod") ||
                           name == QStringLiteral("ccfdeclarationcodeblock")) {
                    QString acctag = element.attribute(QStringLiteral("tag"));
                    // search for our method in the
                    TextBlock * tb = findCodeClassFieldTextBlockByTag(acctag);
                    if (!tb || !addTextBlock(tb)) {
                        logError0("JavaANTCodeDocument: Unable to add codeclassfield child method");
                        // DON'T delete
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("codeblock")) {
                    CodeBlock * block = newCodeBlock();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("JavaANTCodeDocument: Unable to add codeBlock");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("codeblockwithcomments")) {
                    CodeBlockWithComments * block = newCodeBlockWithComments();
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("JavaANTCodeDocument: Unable to add codeBlockwithcomments");
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
                        logError0("JavaANTCodeDocument: Unable to add hierarchicalcodeBlock");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else if (name == QStringLiteral("codeoperation")) {
                    // find the code operation by id
                    QString id = element.attribute(QStringLiteral("parent_id"),QStringLiteral("-1"));
                    UMLObject * obj = UMLApp::app()->document()->findObjectById(Uml::ID::fromString(id));
                    const UMLOperation * op = obj->asUMLOperation();
                    if (op) {
                        CodeOperation  *block = nullptr;
                        logWarn0("TODO: implement CodeGenFactory::newCodeOperation() for JavaANTCodeDocument");
                        break;  // remove when above is implemented
                        block->loadFromXMI(element);
                        if (addTextBlock(block)) {
                            loadCheckForChildrenOK= true;
                        } else {
                            logError0("JavaANTCodeDocument: Unable to add codeoperation");
                            block->deleteLater();
                        }
                    } else {
                        logError0("JavaANTCodeDocument: Unable to find operation create codeoperation");
                    }
                } else if (name == QStringLiteral("xmlelementblock")) {
                    QString xmltag = element.attribute(QStringLiteral("nodeName"),QStringLiteral("UNKNOWN"));
                    XMLElementCodeBlock * block = new XMLElementCodeBlock(this, xmltag);
                    block->loadFromXMI(element);
                    if (!addTextBlock(block)) {
                        logError0("Unable to add XMLelement to Java ANT document");
                        delete block;
                    } else {
                        loadCheckForChildrenOK= true;
                    }
                } else {
                    logDebug1("JavaANTCodeDocument::loadChildTextBlocksFromNode: Got strange tag in "
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
        logWarn1("loadChildBlocks : unable to initialize any child blocks in doc %1", getFileName());
    }

}

/** set the class attributes of this object from
 * the passed element node.
 */
void JavaANTCodeDocument::setAttributesFromNode (QDomElement & root)
{
    // superclass save
    CodeDocument::setAttributesFromNode(root);

    // now set local attributes
    // setPackage(root.attribute("package"));
}

/**
 * load params from the appropriate XMI element node.
 */
void JavaANTCodeDocument::loadFromXMI (QDomElement & root)
{
    setAttributesFromNode(root);
}

/** set attributes of the node that represents this class
 * in the XMI document.
 */
void JavaANTCodeDocument::setAttributesOnNode (QXmlStreamWriter& writer)
{
    // superclass call
    CodeDocument::setAttributesOnNode(writer);

    // now set local attributes/fields
    //FIX
}

/**
 * Save the XMI representation of this object
 */
void JavaANTCodeDocument::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("codedocument"));

    setAttributesOnNode(writer);

    writer.writeEndElement();
}

// we add in our code blocks that describe how to generate
// the project here...
void JavaANTCodeDocument::updateContent()
{
    // FIX : fill in more content based on classes
    // which exist
    CodeBlockWithComments * xmlDecl = getCodeBlockWithComments(QStringLiteral("xmlDecl"), QString(), 0);
    xmlDecl->setText(QStringLiteral("<?xml version=\"1.0\"?>"));
    addTextBlock(xmlDecl);

    XMLElementCodeBlock * rootNode = new XMLElementCodeBlock(this, QStringLiteral("project"), QStringLiteral("Java ANT build document"));
    rootNode->setTag(QStringLiteral("projectDecl"));
    addTextBlock(rootNode);

    // <project name="XDF" default="help" basedir=".">
    //HierarchicalCodeBlock * projDecl = xmlDecl->getHierarchicalCodeBlock("projectDecl", "Java ANT build document", 1);

    // set some global properties for the build
    /*
      <!-- set global properties for this build -->
      <!-- paths -->
      <property name="docApi.dir"  value="docs/api"/>
      <property name="path" value="gov/nasa/gsfc/adc/xdf"/>
      <property name="src" value="src/${path}/"/>
      <property name="top" value="."/>
      <property name="build" value="${top}/gov"/>
      <property name="buildDir" value="${path}"/>
      <!-- compiler directives -->
      <property name="build.compiler" value="modern"/>
      <property name="useDeprecation" value="no"/>
      <property name="jarname" value="${project}.jar"/>
    */
}

// We overwritten by Java language implementation to get lowercase path
QString JavaANTCodeDocument::getPath () const
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


