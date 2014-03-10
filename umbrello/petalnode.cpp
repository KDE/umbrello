/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "petalnode.h"

#include "debug_utils.h"

// todo avoid '"' on QString output

QString a;

class Indenter : public QDebug
{
public:
    Indenter(QDebug &out, const char *className)
        : QDebug(out)
    {
        level++;
        *this << className
              << "(\n"
              << QString().fill('.', level).toAscii().constData()
              ;
    }

    ~Indenter()
    {
        --level;
        *this << "\n"
              << QString().fill(',', level).toAscii().constData()
              << ")";
              ;
    }

    static int level;
};

int Indenter::level = 0;

QDebug operator<<(QDebug _out, const PetalNode::StringOrNode &p)
{
    Indenter out(_out.nospace(), "PetalNode::StringOrNode");
    if (!p.string.isEmpty())
        out << "string: " << p.string;
    if (p.node)
        out << "node: " << *p.node;
    return out;
}

QDebug operator<<(QDebug _out, const PetalNode::NameValue &p)
{
    Indenter out(_out.nospace(), "PetalNode::NameValue");
    out << "name: " << p.first
        << "value: " << p.second;
    return out;
}

QDebug operator<<(QDebug _out, const PetalNode::NameValueList &p)
{
    Indenter out(_out.nospace(), "PetalNode::NameValueList");
    for (int i = 0; i < p.count(); ++i) {
        out << i << ": " << p[i];
    }
    return out;
}

PetalNode::PetalNode(NodeType nt)
{
    m_type = nt;
}

PetalNode::~PetalNode()
{
}

PetalNode::NodeType PetalNode::type() const
{
    return m_type;
}

QStringList PetalNode::initialArgs() const
{
    return m_initialArgs;
}

QString PetalNode::name() const
{
    if (m_initialArgs.count() == 0)
        return QString();
    return m_initialArgs.first();
}

PetalNode::NameValueList PetalNode::attributes() const
{
    return m_attributes;
}

/*
void PetalNode::setType(PetalNode::NodeType t)
{
    m_type = t;
}
*/

void PetalNode::setInitialArgs(const QStringList& args)
{
    m_initialArgs = args;
}

void PetalNode::setAttributes(PetalNode::NameValueList vl)
{
    m_attributes = vl;
}

/**
 * Find an attribute by name.
 * @return  The value of the attribute. StringOrNode::isEmpty() returns true
 *          if the name could not be found.
 */
PetalNode::StringOrNode PetalNode::findAttribute(const QString& name) const
{
    for (int i = 0; i < m_attributes.count(); i++) {
        if (m_attributes[i].first == name)
            return m_attributes[i].second;
    }
    return StringOrNode();
}

QDebug operator<<(QDebug _out, const PetalNode &p)
{
    Indenter out(_out.nospace(), "PetalNode");
    out << "type: " << p.type()
        << "name: " << p.name()
        << "attributes: " << p.attributes();
    return out;
}
