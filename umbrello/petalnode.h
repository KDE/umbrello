/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PETALNODE__H
#define PETALNODE__H

#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
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
        bool isEmpty() const { return (string.isEmpty() && node == 0); }
    };
    typedef QPair<QString, StringOrNode> NameValue;
    typedef QList<NameValue> NameValueList;

    enum NodeType { nt_object, nt_list };

    explicit PetalNode(NodeType nt);
    virtual ~PetalNode();

    // getters
    NodeType type() const;
    QStringList initialArgs() const;  // name and other initial args
    QString name() const;  // convenience function: equal to initialArgs().first()
    QString documentation() const;
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

    friend QDebug operator<<(QDebug out, const PetalNode &p);
};

QDebug operator<<(QDebug out, const PetalNode::StringOrNode &p);
QDebug operator<<(QDebug out, const PetalNode::NameValue &p);
QDebug operator<<(QDebug out, const PetalNode::NameValueList &p);
QDebug operator<<(QDebug out, const PetalNode &p);
#endif

