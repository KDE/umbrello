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

#ifndef CODEDOCUMENT_H
#define CODEDOCUMENT_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>

#include "codegenerationpolicy.h"
#include "codegenobjectwithtextblocks.h"
#include "hierarchicalcodeblock.h"

class CodeBlockWithComments;
class CodeComment;
class TextBlock;
class UMLPackage;

/**
 * A document containing the code for one file.
 * "friend" status is needed for HBlock so it may call addChildTagToMap which
 * is protected.
 */
class CodeDocument : public QObject, public CodeGenObjectWithTextBlocks
{
    friend class HierarchicalCodeBlock;
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    CodeDocument ( );

    /**
     * Empty Destructor
     */
    virtual ~CodeDocument ( );

    /**
     * Set the value of m_fileExtension.
     * @param new_var   the new value of m_fileExtension
     */
    void setFileExtension ( const QString &new_var );

    /**
     * Get the value of m_fileExtension.
     * @return   the value of m_fileExtension
     */
    QString getFileExtension ( ) const;

    /**
     * Set the complete value (name plus any extension) of m_filename.
     * @param new_var   the new value of m_filename
     */
    void setFileName ( const QString &new_var );

    /**
     * Get the value of m_filename. This name is the "complete" filename,
     * meaning that it contains both the file name plus any extension (e.g. "file.cpp").
     * @return   the value of m_filename
     */
    QString getFileName ( ) const;

    /**
     * Set the value of m_package.
     * @param new_var   the new value of m_package
     */
    void setPackage ( UMLPackage *new_var );

    /**
     * Get the value of the package of this code document.
     * @return   the value of m_pathName
     */
    QString getPackage ( ) const;

    /**
     * Get the value of the path to this code document.
     * @return the value of m_pathName
     */
    virtual QString getPath ( );

    /**
     * Set the value of m_ID.
     * @param new_id   the new value of m_ID
     */
    void setID ( const QString &new_id);

    /**
     * Get the value of m_ID.
     * @return   the value of m_ID
     */
    QString getID ( ) const;

    /**
     * Set the value of m_writeOutCode.
     * Whether or not to write out this code document and any codeblocks, etc that it
     * owns.
     * @param new_var   the new value of m_writeOutCode
     */
    void setWriteOutCode ( bool new_var );

    /**
     * Get the value of m_writeOutCode.
     * Whether or not to write out this code document and any codeblocks, etc that it
     * owns.
     * @return   the value of m_writeOutCode
     */
    bool getWriteOutCode ( );

    /**
     * Set a Header comment object.
     * @param comment   the comment for the header
     */
    void setHeader ( CodeComment * comment );

    /**
     * Get the Header comment object.
     * @return   the comment for the header
     */
    CodeComment * getHeader ( );

    /**
     * Insert a new text block after the existing text block. Returns
     * false if it cannot insert the textblock.
     * @param newBlock        the text block to insert
     * @param existingBlock   the place where to insert
     * @param after           at the index of the existingBlock or after
     * @return                the success status
     */
    bool insertTextBlock (TextBlock * newBlock, TextBlock * existingBlock, bool after = true);

    /**
     * Lookup a certain textblock by its tag value, returns NULL if it can not
     * find the TextBlock with such a tag. If descendIntoChildren is true, then
     * any child hierarchical textblocks will also be searched for a match.
     * @param tag                   the tag to look for
     * @param descendIntoChildren   look down the hierarchy
     * @return                      the found text block
     */
    TextBlock * findTextBlockByTag( const QString &tag , bool descendIntoChildren = false);

    /**
     * Create the string representation of this object.
     * @return   the created string
     */
    virtual QString toString ( );

    /**
     * Save the XMI representation of this object.
     * @param doc   the xmi document
     * @param root  the starting point to append
     */
    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    /**
     * Load params from the appropriate XMI element node.
     * @param root   the starting point for loading
     */
    virtual void loadFromXMI ( QDomElement & root );

    /**
     * Create a new CodeBlock object belonging to this CodeDocument.
     * @return      the just created CodeBlock
     */
    virtual CodeBlock * newCodeBlock ( );

    /**
     * Create a new HierarchicalCodeBlock object belonging to this CodeDocument.
     * @return   the just created HierarchicalCodeBlock
     */
    virtual HierarchicalCodeBlock * newHierarchicalCodeBlock ( );

    /**
     * Create a new CodeBlockWithComments object belonging to this CodeDocument.
     * @return   the just created CodeBlockWithComments
     */
    virtual CodeBlockWithComments * newCodeBlockWithComments ( );

    /**
     * Return a unique and currently unallocated text block tag for this document.
     * @param prefix   the prefix to add
     * @return         the just created unique tag
     */
    virtual QString getUniqueTag( const QString& prefix = QString("") );

    /**
     * A little utility method which calls CodeGenerator::cleanName.
     * @param name   the cleanable name
     * @return       the cleaned name
     */
    QString cleanName ( const QString &name );

    /**
     * Cause this code document to synchronize to current generator policy.
     */
    virtual void synchronize();

    /**
     * Update the content of this code document.
     * This is where you should lay out your code document structure of textblocks
     * in the inheriting class, should it have any text in it.
     */
    virtual void updateContent();

protected:

    /**
     * Set attributes of the node that represents this class
     * in the XMI document.
     */
    virtual void setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement);

    /**
     * Set the class attributes of this object from
     * the passed element node.
     */
    virtual void setAttributesFromNode ( QDomElement & element);

    // these next 2 are needed by child hierarchical code blocks so
    // that when they call getUniqueTag, we really get a unique tag
    // Also, it allows 'findTextBlockByTag' To find any tagged text block
    // anywhere in the document, whether directly owned by the document OR
    // by some child hierarchical textblock
    void addChildTagToMap ( const QString &tag, TextBlock * tb);
    void removeChildTagFromMap ( const QString &tag );

    /**
     * Update the header text of this codedocument.
     */
    void updateHeader ();

    /**
     * Reset/clear our inventory of textblocks in this document.
     */
    void resetTextBlocks();

    /**
     * Have to implement this for CodeObjectWithTextBlocks.
     * Actually does not do anythying for a vannilla code document.
     */
    virtual TextBlock * findCodeClassFieldTextBlockByTag( const QString &tag );

    friend QTextStream& operator<<(QTextStream& os, const CodeDocument& obj);

private:

    int m_lastTagIndex;
    QString m_filename;
    QString m_fileExtension;
    QString m_ID;
    QString m_pathName;
    UMLPackage *m_package;

    bool m_writeOutCode; // Whether or not to write out this code document
                         // and any codeblocks, etc that it owns.

    CodeComment * m_header;

    // TextBlockList m_textblockVector;
    // QMap<QString, TextBlock *> m_textBlockTagMap;

    // for recording all of the textblocks held by child hierarchical codeblocks
    QMap<QString, TextBlock *> m_childTextBlockTagMap;

};

QTextStream& operator<<(QTextStream& os, const CodeDocument& obj);

#endif // CODEDOCUMENT_H
