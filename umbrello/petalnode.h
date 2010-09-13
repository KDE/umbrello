/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PETALNODE__H
#define PETALNODE__H

#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QStringList>

/**
 * Rose petal node - parse tree for model import
 *
 * A Rose petal node can be of type:
 *  + object    - initialArgs() contains the object type name and further
 *                initial arguments which depend on the exact object type
 *              - subordinate attributes are contained in attributes()
 *  + list      - initialArgs() contains the list type name
 *              - list elements are contained in attributes() but the name
 *                of each NameValue is empty.
 *  + value     - not represented as a node, instead the stripped down value
 *                is saved in the value string of the NameValue.
 *                Example: for the input
 *                  (value Text "This is some text")
 *                the following is saved in the value string of the NameValue:
 *                  "This is some text"
 *
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PetalNode
{
public:
    /**
     * Use `string' if it is not empty.
     * Use `node' if it is not NULL.
     * Either `string' is set, or `node', but never both.
     * (Perhaps this should be a union but that is ugly.)
     */
    struct StringOrNode {
        QString string;
        PetalNode *node;
        StringOrNode() { node = 0; }
        virtual ~StringOrNode() { }
        bool isEmpty() { return (string.isEmpty() && node == 0); }
    };
    typedef QPair<QString, StringOrNode> NameValue;
    typedef QList<NameValue> NameValueList;

    enum NodeType { nt_object, nt_list };

    PetalNode(NodeType nt);
    virtual ~PetalNode();

    // getters
    NodeType type() const;
    QStringList initialArgs() const;  // name and other initial args
    QString name() const;  // convenience function: equal to initialArgs().first()
    NameValueList attributes() const;

    // setters
    //void setType(NodeType nt);   see constructor
    void setInitialArgs(const QStringList& args);
    void setAttributes(NameValueList vl);

    // utilities
    StringOrNode findAttribute(const QString& name) const;

private:
    NodeType m_type;
    QStringList m_initialArgs;
    NameValueList m_attributes;
};

#endif

