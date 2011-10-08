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

// own header
#include "codedocument.h"

// local includes
#include "codegenerator.h"
#include "debug_utils.h"
#include "package.h"
#include "umldoc.h"
#include "uml.h"

// qt includes
#include <QtCore/QRegExp>
#include <QtCore/QDateTime>

/**
 * Constructor.
 */
CodeDocument::CodeDocument () : CodeGenObjectWithTextBlocks(this),
    m_lastTagIndex(0), m_filename(QString()), m_fileExtension(QString()),
    m_ID(QString()), m_pathName(QString()), m_package(NULL), m_writeOutCode(true)  
{
    setHeader(new CodeComment(this));
    //  m_dialog = new CodeDocumentDialog( );
}

/**
 * Destructor.
 */
CodeDocument::~CodeDocument ( )
{
    // delete all the text blocks we have
    while (!m_textblockVector.isEmpty()) {
        delete m_textblockVector.takeFirst();
    }
    delete m_header;
}

/**
 * Set the complete value (name plus any extension) of m_filename.
 * @param new_var   the new value of m_filename
 */
void CodeDocument::setFileName ( const QString &new_var )
{
    m_filename = new_var;
}

/**
 * Get the value of m_filename. This name is the "complete" filename,
 * meaning that it contains both the file name plus any extension (e.g. "file.cpp").
 * @return   the value of m_filename
 */
QString CodeDocument::getFileName ( ) const
{
    return m_filename;
}

/**
 * Set the value of m_fileExtension.
 * @param new_var   the new value of m_fileExtension
 */
void CodeDocument::setFileExtension ( const QString &new_var )
{
    m_fileExtension = new_var;
    updateHeader(); // because we are using new heading file
}

/**
 * Get the value of m_fileExtension.
 * @return   the value of m_fileExtension
 */
QString CodeDocument::getFileExtension( ) const
{
    return m_fileExtension;
}

/**
 * Set the value of m_package.
 * @param new_var   the new value of m_package
 */
void CodeDocument::setPackage ( UMLPackage *new_var )
{
    m_package = new_var;
}

/**
 * Get the value of the path to this code document.
 * @return the value of m_pathName
 */
QString CodeDocument::getPath ( )
{
    QString path = getPackage();

    // Replace all white spaces with blanks
    path = path.simplified();

    // Replace all blanks with underscore
    path.replace(QRegExp(" "), "_");

    // this allows multiple directory paths (ala Java, some other languages)
    // in from the package specification
    path.replace(QRegExp("\\."),"/"); // Simple hack!.. but this is more or less language
    // dependant and should probably be commented out.
    // Still, as a general default it may be useful -b.t.
    return path;
}

/**
 * Get the value of the package of this code document.
 * @return   the value of m_pathName
 */
QString CodeDocument::getPackage ( ) const
{
    if (m_package)
        return m_package->name();
    return QString();
}

/**
 * Set the value of m_ID.
 * @param new_id   the new value of m_ID
 */
void CodeDocument::setID ( const QString &new_id )
{
    m_ID = new_id;
}

/**
 * Get the value of m_ID.
 * @return   the value of m_ID
 */
QString CodeDocument::getID ( ) const
{
    return m_ID;
}

/**
 * Set the value of m_writeOutCode.
 * Whether or not to write out this code document and any codeblocks, etc that it
 * owns.
 * @param new_var   the new value of m_writeOutCode
 */
void CodeDocument::setWriteOutCode ( bool new_var )
{
    m_writeOutCode = new_var;
}

/**
 * Get the value of m_writeOutCode.
 * Whether or not to write out this code document and any codeblocks, etc that it
 * owns.
 * @return   the value of m_writeOutCode
 */
bool CodeDocument::getWriteOutCode ( )
{
    return m_writeOutCode;
}

/**
 * Set a Header comment object.
 * @param comment   the comment for the header
 */
void CodeDocument::setHeader ( CodeComment * comment )
{
    m_header = comment;
}

/**
 * Get the Header comment object.
 * @return   the comment for the header
 */
CodeComment * CodeDocument::getHeader ( )
{
    return m_header;
}

/**
 * Return a unique and currently unallocated text block tag for this document.
 * @param prefix   the prefix to add
 * @return         the just created unique tag
 */
QString CodeDocument::getUniqueTag ( const QString& prefix )
{
    QString tag = prefix ;
    if(tag.isEmpty())
        tag += "tblock";

    tag = tag + "_0";
    int number = m_lastTagIndex;
    for ( ; findTextBlockByTag(tag, true); ++number) {
        tag = prefix + '_' + QString::number(number);
    }
    m_lastTagIndex = number;
    return tag;
}

/**
 * Insert a new text block after the existing text block. Returns
 * false if it cannot insert the textblock.
 * @param newBlock        the text block to insert
 * @param existingBlock   the place where to insert
 * @param after           at the index of the existingBlock or after
 * @return                the success status
 */
bool CodeDocument::insertTextBlock(TextBlock * newBlock, TextBlock * existingBlock, bool after)
{
    if (!newBlock || !existingBlock)
        return false;

    QString tag = existingBlock->getTag();
    if (!findTextBlockByTag(tag, true))
        return false;

    int index = m_textblockVector.indexOf(existingBlock);
    if (index < 0)
    {
        // may be hiding in child hierarchical codeblock
        foreach (TextBlock* tb, m_textblockVector)
        {
            HierarchicalCodeBlock * hb = dynamic_cast<HierarchicalCodeBlock*>(tb);
            if (hb && hb->insertTextBlock(newBlock, existingBlock, after))
                return true; // found, and inserted, otherwise keep going
        }
        // ugh. where is the child block?
        uWarning() << " Warning: couldnt insert text block (tag:" << newBlock->getTag()
                   << "). Reference text block (tag:" << existingBlock->getTag() << ") not found.";
        return false;
    }

    // if we get here.. it was in this object so insert

    // check for tag FIRST
    QString new_tag = newBlock->getTag();

    // assign a tag if one doesn't already exist
    if (new_tag.isEmpty())
    {
        new_tag = getUniqueTag();
        newBlock->setTag(new_tag);
    }

    if (m_textBlockTagMap.contains(new_tag))
        return false; // return false, we already have some object with this tag in the list
    else
        m_textBlockTagMap.insert(new_tag, newBlock);

    if (after)
        index++;

    m_textblockVector.insert(index, newBlock);
    return true;
}

/**
 * A little utility method which calls CodeGenerator::cleanName.
 * @param name   the cleanable name
 * @return       the cleaned name
 */
QString CodeDocument::cleanName ( const QString &name )
{
    return CodeGenerator::cleanName(name);
}

/**
 * Update the header text of this codedocument
 * (text and status of the head comment).
 */
void CodeDocument::updateHeader ()
{
    //try to find a heading file (license, coments, etc) then extract its text
    QString headingText = UMLApp::app()->commonPolicy()->getHeadingFile(getFileExtension());

    headingText.replace(QRegExp("%filename%"),getFileName()+getFileExtension());
    headingText.replace(QRegExp("%filepath%"),getPath());
    headingText.replace(QRegExp("%time%"), QTime::currentTime().toString());
    headingText.replace(QRegExp("%date%"), QDate::currentDate().toString());

    getHeader()->setText(headingText);

    // update the write out status of the header
    if (UMLApp::app()->commonPolicy()->getIncludeHeadings())
        getHeader()->setWriteOutText(true);
    else
        getHeader()->setWriteOutText(false);
}

/**
 * Create the string representation of this object.
 * @return   the created string
 */
QString CodeDocument::toString ( )
{
    // IF the whole document is turned "Off" then don't bother
    // checking individual code blocks, just send back empty string
    if (!getWriteOutCode())
        return QString();

    QString content = getHeader()->toString();

    // update the time/date

    // comments, import, package codeblocks go next
    TextBlockList * items = getTextBlockList();
    foreach (TextBlock* c, *items)
    {
        if (c->getWriteOutText()) {
            QString str = c->toString();
            if (!str.isEmpty())
                content.append(str);
        }
    }
    return content;
}

/**
 * Cause this code document to synchronize to current generator policy.
 */
void CodeDocument::synchronize()
{
    updateContent();
}

/**
 * Reset/clear our inventory of textblocks in this document.
 * Need to overload method to be able to clear the childTextBlockMap.
 */
void CodeDocument::resetTextBlocks()
{
    CodeGenObjectWithTextBlocks::resetTextBlocks();
    m_childTextBlockTagMap.clear();
}

/**
 * Load params from the appropriate XMI element node.
 * @param root   the starting point for loading
 */
void CodeDocument::loadFromXMI ( QDomElement & root )
{
    setAttributesFromNode(root);
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeDocument::setAttributesOnNode ( QDomDocument & doc, QDomElement & docElement)
{
    // superclass call
    CodeGenObjectWithTextBlocks::setAttributesOnNode(doc,docElement);

    // now set local attributes/fields
    docElement.setAttribute("fileName", getFileName());
    docElement.setAttribute("fileExt", getFileExtension());
    Uml::IDType pkgId = Uml::id_None;
    if (m_package)
        pkgId = m_package->id();
    docElement.setAttribute("package", ID2STR(pkgId));
    docElement.setAttribute("writeOutCode", getWriteOutCode() ? "true" : "false");
    docElement.setAttribute("id", getID());

    // set the a header
    // which we will store in its own separate child node block
    QDomElement commElement = doc.createElement( "header" );
    getHeader()->saveToXMI(doc, commElement); // comment
    docElement.appendChild( commElement);

    // doc codePolicy?
    // FIX: store ONLY if different from the parent generator
    // policy.. something which is not possible right now. -b.t.
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void CodeDocument::setAttributesFromNode ( QDomElement & root)
{
    // now set local attributes
    setFileName(root.attribute("fileName",""));
    setFileExtension(root.attribute("fileExt",""));
    QString pkgStr = root.attribute("package","");
    if (!pkgStr.isEmpty() && pkgStr != "-1") {
        UMLDoc *umldoc = UMLApp::app()->document();
        if (pkgStr.contains( QRegExp("\\D") )) {
            // suspecting pre-1.5.3 file format where the package name was
            // saved instead of the package ID.
            UMLObject *o = umldoc->findUMLObject(pkgStr);
            m_package = dynamic_cast<UMLPackage*>(o);
        }
        if (m_package == NULL) {
            UMLObject *o = umldoc->findObjectById(STR2ID(pkgStr));
            m_package = dynamic_cast<UMLPackage*>(o);
        }
    }
    setWriteOutCode(root.attribute("writeOutCode","true") == "true" ? true : false);
    setID(root.attribute("id",""));

    // load comment now
    // by looking for our particular child element
    QDomNode node = root.firstChild();
    QDomElement element = node.toElement();
    while ( !element.isNull() ) {
        QString tag = element.tagName();
        if ( tag == "header" ) {
            QDomNode cnode = element.firstChild();
            QDomElement celem = cnode.toElement();
            getHeader()->loadFromXMI(celem);
            break;
        }
        node = element.nextSibling();
        element = node.toElement();
    }

    // a rare case where the super-class load is AFTER local attributes
    CodeGenObjectWithTextBlocks::setAttributesFromNode(root);
}

/**
 * Save the XMI representation of this object.
 * @param doc   the xmi document
 * @param root  the starting point to append
 */
void CodeDocument::saveToXMI ( QDomDocument & doc, QDomElement & root )
{
    QDomElement docElement = doc.createElement( "codedocument" );
    setAttributesOnNode(doc, docElement);
    root.appendChild( docElement );
}

/**
 * Update the content of this code document.
 * This is where you should lay out your code document structure of textblocks
 * in the inheriting class, should it have any text in it.
 * Vanilla code documents don't have much to do.. override this with a different
 * version for your own documents.
 */
void CodeDocument::updateContent()
{
    updateHeader(); // doing this insures time/date stamp is at the time of this call
}

/**
 * Create a new CodeBlock object belonging to this CodeDocument.
 * @return      the just created CodeBlock
 */
CodeBlock * CodeDocument::newCodeBlock ( )
{
    return new CodeBlock(this);
}

/**
 * Create a new CodeBlockWithComments object belonging to this CodeDocument.
 * @return   the just created CodeBlockWithComments
 */
CodeBlockWithComments * CodeDocument::newCodeBlockWithComments ( )
{
    return new CodeBlockWithComments(this);
}

/**
 * Create a new HierarchicalCodeBlock object belonging to this CodeDocument.
 * @return   the just created HierarchicalCodeBlock
 */
HierarchicalCodeBlock * CodeDocument::newHierarchicalCodeBlock ( )
{
    HierarchicalCodeBlock *hb = new HierarchicalCodeBlock(this);
    //hb->update();
    return hb;
}

void CodeDocument::removeChildTagFromMap ( const QString &tag )
{
    m_childTextBlockTagMap.remove(tag);
}

void CodeDocument::addChildTagToMap ( const QString &tag, TextBlock * tb)
{
    m_childTextBlockTagMap.insert(tag, tb);
}

/**
 * Lookup a certain textblock by its tag value, returns NULL if it can not
 * find the TextBlock with such a tag. If descendIntoChildren is true, then
 * any child hierarchical textblocks will also be searched for a match.
 * @param tag                   the tag to look for
 * @param descendIntoChildren   look down the hierarchy
 * @return                      the found text block
 */
TextBlock * CodeDocument::findTextBlockByTag( const QString &tag , bool descendIntoChildren)
{
    //if we already know to which file this class was written/should be written, just return it.
    if (m_textBlockTagMap.contains(tag))
        return m_textBlockTagMap[tag];

    if (descendIntoChildren)
        if (m_childTextBlockTagMap.contains(tag))
            return m_childTextBlockTagMap[tag];

    return NULL;
}

/**
 * Have to implement this for CodeObjectWithTextBlocks.
 * Actually does not do anythying for a vannilla code document.
 */
TextBlock * CodeDocument::findCodeClassFieldTextBlockByTag ( const QString &tag )
{
    uWarning() << "Called findCodeClassFieldMethodByTag(" << tag << ") for a regular CodeDocument";
    return NULL;
}

QDebug operator<<(QDebug os, const CodeDocument& obj)
{
    os.nospace() << "CodeDocument: id=" << obj.getID()
       << " , file=" << obj.getFileName();  //:TODO: add all attributes
    return os.space();
}
