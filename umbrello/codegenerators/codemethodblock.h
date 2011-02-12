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

#ifndef CODEMETHODBLOCK_H
#define CODEMETHODBLOCK_H

#include "basictypes.h"
#include "ownedcodeblock.h"
#include "codeblockwithcomments.h"

#include <QtCore/QString>

class ClassifierCodeDocument;

/**
 * class CodeMethodBlock
 * A  common type of "code block" that occurs in OO code.
 * Note: keep the inheritance sequence: QObject needs to be first in inheritance list.
 */
class CodeMethodBlock : public OwnedCodeBlock, public CodeBlockWithComments
{
    friend class CodeGenObjectWithTextBlocks;
    Q_OBJECT
public:

    /**
     * Constructors
     */
    CodeMethodBlock ( ClassifierCodeDocument * doc, UMLObject * parentObj,
                      const QString & body = "", const QString & comment = "");

    /**
     * Empty Destructor
     */
    virtual ~CodeMethodBlock ( );

    virtual QString toString ( ) const;

    QString getStartMethodText () const;

    QString getEndMethodText () const;

    CodeDocument * getParentDocument();

    /**
     * This is the method called from within syncToparent()
     * to update the *body* of the method.
     * It is only called if the method is Auto-generated.
     */
    virtual void updateContent ( ) = 0;

protected:

    virtual void release ();

    void setStartMethodText (const QString &value);

    void setEndMethodText (const QString &value);

    /**
     * This is the method called from within syncToparent().
     * To update the start and end Method text. It is called
     * whether or not the method is Auto or User generated.
     */
    virtual void updateMethodDeclaration ( ) = 0;

    virtual void setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement);

    virtual void setAttributesFromNode ( QDomElement & element);

    virtual void setAttributesFromObject (TextBlock * obj);

private:

    QString m_startMethod;
    QString m_endMethod;

public slots:

    virtual void syncToParent();
};

#endif // CODEMETHODBLOCK_H
