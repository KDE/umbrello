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

#ifndef CODEDOCUMENT_H
#define CODEDOCUMENT_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>

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

    CodeDocument ( );
    virtual ~CodeDocument ( );

    void setFileExtension ( const QString &new_var );
    QString getFileExtension ( ) const;

    void setFileName ( const QString &new_var );
    QString getFileName ( ) const;

    void setPackage ( UMLPackage *new_var );
    QString getPackage ( ) const;

    virtual QString getPath ( );

    void setID ( const QString &new_id);
    QString getID ( ) const;

    void setWriteOutCode ( bool new_var );
    bool getWriteOutCode ( );

    void setHeader ( CodeComment * comment );
    CodeComment * getHeader ( );

    bool insertTextBlock (TextBlock * newBlock, TextBlock * existingBlock, bool after = true);

    TextBlock * findTextBlockByTag( const QString &tag , bool descendIntoChildren = false);

    virtual QString toString ( );

    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );
    virtual void loadFromXMI ( QDomElement & root );

    virtual CodeBlock * newCodeBlock ( );
    virtual HierarchicalCodeBlock * newHierarchicalCodeBlock ( );
    virtual CodeBlockWithComments * newCodeBlockWithComments ( );

    virtual QString getUniqueTag( const QString& prefix = QString("") );

    QString cleanName ( const QString &name );

    virtual void synchronize();

    virtual void updateContent();

    friend QDebug operator<<(QDebug os, const CodeDocument& obj);

protected:

    virtual void setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement);

    virtual void setAttributesFromNode ( QDomElement & element);

    // these next 2 are needed by child hierarchical code blocks so
    // that when they call getUniqueTag, we really get a unique tag
    // Also, it allows 'findTextBlockByTag' To find any tagged text block
    // anywhere in the document, whether directly owned by the document OR
    // by some child hierarchical textblock
    void addChildTagToMap ( const QString &tag, TextBlock * tb);
    void removeChildTagFromMap ( const QString &tag );

    void updateHeader ();

    void resetTextBlocks();

    virtual TextBlock * findCodeClassFieldTextBlockByTag( const QString &tag );

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

    // for recording all of the textblocks held by child hierarchical codeblocks
    QMap<QString, TextBlock *> m_childTextBlockTagMap;

};

#endif // CODEDOCUMENT_H
