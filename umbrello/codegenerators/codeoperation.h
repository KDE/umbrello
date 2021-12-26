/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEOPERATION_H
#define CODEOPERATION_H

#include "codemethodblock.h"
#include "operation.h"

#include <QString>

/**
 *
 */
class CodeOperation : public CodeMethodBlock
{
    Q_OBJECT
public:

    /**
     * Constructors
     */
    CodeOperation (ClassifierCodeDocument * doc, UMLOperation * parent,
                    const QString & body = QString(), const QString & comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~CodeOperation ();

    UMLOperation * getParentOperation();

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual void loadFromXMI (QDomElement & root);

    static QString findTag (UMLOperation * op) ;

    virtual void setAttributesFromObject (TextBlock * obj);

    virtual void updateMethodDeclaration() = 0;
    virtual void updateContent();

protected:

    // list of parameters used by this code operation.
    // QList<CodeParameter*> m_parameterVector;

    virtual void setAttributesOnNode (QXmlStreamWriter& writer);

    virtual void setAttributesFromNode (QDomElement & element);

private:
    void init (UMLOperation * parentOp);

};

#endif // CODEOPERATION_H
