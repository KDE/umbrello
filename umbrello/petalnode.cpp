/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
              << QString().fill(QLatin1Char('.'), level).toLatin1().constData()
              ;
    }

    ~Indenter()
    {
        --level;
        *this << "\n"
              << QString().fill(QLatin1Char(','), level).toLatin1().constData()
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

/**
 * Return the documentation from a petal node with carriage
 * return handling and removed surrounding quotation marks
 * if present.
 */
QString PetalNode::documentation() const
{
    QString s = findAttribute(QLatin1String("documentation")).string.trimmed();
    if (s.isEmpty())
        return s;
    s.replace(QLatin1String("\\n"), QLatin1String("\n"));
    if (s.startsWith(QLatin1Char('\"')) && s.endsWith(QLatin1Char('\"')))
        return s.mid(1 ,s.length()-2);
    else
        return s;
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
