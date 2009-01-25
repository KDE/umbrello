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

#ifndef HIERARCHICALCODEBLOCK_H
#define HIERARCHICALCODEBLOCK_H

#include "codegenobjectwithtextblocks.h"
#include "codeblockwithcomments.h"
#include "codeclassfieldlist.h"

#include <QtCore/QMap>
#include <QtCore/QString>

class HierarchicalCodeBlock : public CodeBlockWithComments, public CodeGenObjectWithTextBlocks
{
    friend class CodeGenObjectWithTextBlocks;
public:

    explicit HierarchicalCodeBlock ( CodeDocument * doc , const QString &startString = "", const QString &endString = "", const QString &comment = "");

    /**
     * Empty Destructor
     */
    virtual ~HierarchicalCodeBlock ( );

    void setEndText ( const QString &new_var );

    QString getEndText ( ) const;

    bool addTextBlock ( TextBlock * add_object );

    bool insertTextBlock (TextBlock * newBlock, TextBlock * existingBlock, bool after = true);

    bool removeTextBlock ( TextBlock * remove_object );

    void setStartText ( const QString &text );

    QString getStartText ( ) const;

    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    virtual void loadFromXMI ( QDomElement & root );

    virtual QString toString ( ) const;

    QString getUniqueTag();
    QString getUniqueTag( const QString& prefix );

    void addCodeClassFieldMethods ( CodeClassFieldList &list );

    virtual CodeBlock * newCodeBlock();
    virtual CodeBlockWithComments * newCodeBlockWithComments();
    virtual HierarchicalCodeBlock * newHierarchicalCodeBlock();

protected:

    virtual void release ();

    virtual void setAttributesOnNode (QDomDocument & doc, QDomElement & elem );

    virtual void setAttributesFromNode ( QDomElement & element);

    virtual void setAttributesFromObject (TextBlock * obj);

    TextBlock * findCodeClassFieldTextBlockByTag ( const QString &tag );

private:

    QString m_startText;
    QString m_endText;

    QString childTextBlocksToString() const;
    void initAttributes ( ) ;

};

#endif // HIERARCHICALCODEBLOCK_H
