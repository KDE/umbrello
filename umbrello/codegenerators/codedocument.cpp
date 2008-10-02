/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codedocument.h"

// qt/kde includes
#include <QtCore/QRegExp>
#include <QtCore/QDateTime>
#include <kdebug.h>

// local includes
#include "codegenerator.h"
#include "package.h"
#include "umldoc.h"
#include "uml.h"

CodeDocument::CodeDocument () : CodeGenObjectWithTextBlocks(this),
    m_lastTagIndex(0), m_filename(QString()), m_fileExtension(QString()),
    m_ID(QString()), m_pathName(QString()), m_package(NULL), m_writeOutCode(true)  
{
    setHeader(new CodeComment(this));
    //  m_dialog = new CodeDocumentDialog( );
}

CodeDocument::~CodeDocument ( )
{
    // delete all the text blocks we have
    while (!m_textblockVector.isEmpty()) {
        delete m_textblockVector.takeFirst();
    }
    delete m_header;
}

void CodeDocument::setFileName ( const QString &new_var )
{
    m_filename = new_var;
}

QString CodeDocument::getFileName ( ) const
{
    return m_filename;
}

void CodeDocument::setFileExtension ( const QString &new_var )
{
    m_fileExtension = new_var;
    updateHeader(); // because we are using new heading file
}

QString CodeDocument::getFileExtension( ) const
{
    return m_fileExtension;
}

void CodeDocument::setPackage ( UMLPackage *new_var )
{
    m_package = new_var;
}

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

QString CodeDocument::getPackage ( ) const
{
    if (m_package)
        return m_package->getName();
    return QString();
}

void CodeDocument::setID ( const QString &new_var )
{
    m_ID = new_var;
}

QString CodeDocument::getID ( ) const
{
    return m_ID;
}

void CodeDocument::setWriteOutCode ( bool new_var )
{
    m_writeOutCode = new_var;
}

bool CodeDocument::getWriteOutCode ( )
{
    return m_writeOutCode;
}

void CodeDocument::setHeader ( CodeComment * header )
{
    m_header = header;
}

CodeComment * CodeDocument::getHeader ( )
{
    return m_header;
}

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

QString CodeDocument::cleanName ( const QString &name )
{
    return CodeGenerator::cleanName(name);
}

// update the text and status of the head comment
void CodeDocument::updateHeader ()
{
    //try to find a heading file (license, coments, etc) then extract its text
    QString headingText = UMLApp::app()->getCommonPolicy()->getHeadingFile(getFileExtension());

    headingText.replace(QRegExp("%filename%"),getFileName()+getFileExtension());
    headingText.replace(QRegExp("%filepath%"),getPath());
    headingText.replace(QRegExp("%time%"), QTime::currentTime().toString());
    headingText.replace(QRegExp("%date%"), QDate::currentDate().toString());

    getHeader()->setText(headingText);

    // update the write out status of the header
    if (UMLApp::app()->getCommonPolicy()->getIncludeHeadings())
        getHeader()->setWriteOutText(true);
    else
        getHeader()->setWriteOutText(false);
}

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

void CodeDocument::synchronize()
{
    updateContent();
}

// need to overload method to be able to clear the childTextBlockMap
void CodeDocument::resetTextBlocks()
{
    CodeGenObjectWithTextBlocks::resetTextBlocks();
    m_childTextBlockTagMap.clear();
}

void CodeDocument::loadFromXMI ( QDomElement & root )
{
    setAttributesFromNode(root);
}

void CodeDocument::setAttributesOnNode ( QDomDocument & doc, QDomElement & docElement)
{
    // superclass call
    CodeGenObjectWithTextBlocks::setAttributesOnNode(doc,docElement);

    // now set local attributes/fields
    docElement.setAttribute("fileName", getFileName());
    docElement.setAttribute("fileExt", getFileExtension());
    Uml::IDType pkgId = Uml::id_None;
    if (m_package)
        pkgId = m_package->getID();
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

void CodeDocument::setAttributesFromNode ( QDomElement & root)
{
    // now set local attributes
    setFileName(root.attribute("fileName",""));
    setFileExtension(root.attribute("fileExt",""));
    QString pkgStr = root.attribute("package","");
    if (!pkgStr.isEmpty() && pkgStr != "-1") {
        UMLDoc *umldoc = UMLApp::app()->getDocument();
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

void CodeDocument::saveToXMI ( QDomDocument & doc, QDomElement & root )
{
    QDomElement docElement = doc.createElement( "codedocument" );
    setAttributesOnNode(doc, docElement);
    root.appendChild( docElement );
}

// vanilla code documents don't have much
// to do.. override this with a different
// version for your own documents
void CodeDocument::updateContent()
{
    updateHeader(); // doing this insures time/date stamp is at the time of this call
}

CodeBlock * CodeDocument::newCodeBlock ( )
{
    return new CodeBlock(this);
}

CodeBlockWithComments * CodeDocument::newCodeBlockWithComments ( )
{
    return new CodeBlockWithComments(this);
}

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

TextBlock * CodeDocument::findCodeClassFieldTextBlockByTag ( const QString &tag )
{
    uWarning() << "Called findCodeClassFieldMethodByTag(" << tag << ") for a regular CodeDocument";
    return NULL;
}

QTextStream& operator<<(QTextStream& os, const CodeDocument& obj)
{
    os << "CodeDocument: id=" << obj.getID()
       << " , file=" << obj.getFileName();  //:TODO: add all attributes
    return os;
}
