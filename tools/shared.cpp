/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
    SPDX-FileCopyrightText: 2014-2020 Ralf Habacker <ralf.habacker@freenet.de>
*/

#include "shared.h"

#include <iostream>

#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QtDebug>

QDebug operator <<(QDebug out, const QXmlStreamAttribute &a)
{
    out << "QXmlStreamAttribute("
        << "prefix:" << a.prefix().toString()
        << "namespaceuri:" << a.namespaceUri().toString()
        << "name:" << a.name().toString()
        << " value:" << a.value().toString()
        << ")";
    return out;
}

using namespace std;

/**
 * Return list of xmi file attributes containing language information.
 *
 * @return list of xmi element attributes
 */
QStringList XMILanguagesAttributes()
{
    return QStringList() << "comment" << "documentation"
                         << "label" << "name"
                         << "pretext" << "posttext" << "text"
                         << "statename" << "activityname" << "instancename";
}

/**
 * Extract language related XML attributes from XMI file

 * @param fileName file to extract attributes from
 * @param attributes List with attribute names to extract
 * @param result map with extracted results
 * @return true successful extraction
 * @return false extraction failure
 */
bool extractAttributesFromXMI(const char *fileName, const QStringList &attributes, POMap &result)
{
    QFile file(fileName);
    QXmlStreamReader xmlReader;

    if(!file.open(QIODevice::ReadOnly))
        return false;

    xmlReader.setDevice(&file);
    while (!xmlReader.atEnd()) {
        QXmlStreamReader::TokenType type = xmlReader.readNext();
        if (type != QXmlStreamReader::StartElement)
            continue;

        foreach(const QString &attributeName, attributes) {
            if (!xmlReader.attributes().hasAttribute(attributeName))
                continue;

            QString value = xmlReader.attributes().value(attributeName).toString();
            if (value.isEmpty())
                continue;

            QString tagName = xmlReader.name().toString() + ':' + attributeName;
            QString key = value;
            if (result.contains(key)) {
                result[key].lineNumbers.append(xmlReader.lineNumber());
                result[key].tagNames.append(tagName);
            } else {
                POEntry entry;
                entry.tagNames.append(tagName);
                entry.value = value;
                entry.lineNumbers.append(xmlReader.lineNumber());
                result[key] = entry;
            }
        }
    }
    if (xmlReader.hasError()) {
          std::cerr << "Parsing failed." << std::endl;
          return false;
    }
    return true;
}

QString fromGetTextString(QString s)
{
    s.replace("\\n", "\n");
    s.replace("\\\"", "\"");
    return s;
}

QString toXMLCharacterEntities(QString s)
{
    s.replace("\\n", "&#xa");
    s.replace("\\\"", "&qout;");
    s.replace('&', "&amp;");
    s.replace('<', "&lt;");
    s.replace('>', "&gt;");
    return s;
}

QString escape(QString s)
{
    s.replace(QRegularExpression("\\\\"), "\\\\");
    s.replace(QRegularExpression("\""), "\\\"");
    return s;
}

QString toGetTextString(const QString &message)
{
    QStringList list = message.split('\n');
    QString line;
    QString result;

    if (list.count() == 1) {
        line = list.first();
        if (line.isEmpty())
            result += " \"\"\n";
        else
            result +=" \"" + escape(line) + "\"\n";
    } else {
        result += " \"\"\n";
        QStringList::ConstIterator last = list.constEnd();
        if (!list.isEmpty())
            --last;
        for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
            line = *it;
            if (!line.isEmpty()) {
                result += "      \"" + escape(line);
                if (it == last)
                    result += "\"\n";
                else
                    result += "\\n\"\n";
            } else {
                result += "      \"";
                if (it != last)
                    result += "\\n";
                result += "\"\n";
            }
        }
    }
    return result;
}

/**
 * Fetch Pot file.
 *
 * @param fileName file to parse
 * @param map returned map with parsed items
 * @return true parsed successfully
 * @return false parse failure
 */
bool fetchPoFile(const QString &fileName, TranslationMap &map)
{
    QFile file(fileName);

    if(!file.open(QIODevice::ReadOnly))
        return false;

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString key, value;
    bool multiLineID = false;
    bool multiLineValue = false;
    while (!in.atEnd()) {
        QString line = in.readLine();
        // handle multilines
        if (line.startsWith(QStringLiteral("msgid"))) {
            key = line.mid(7,line.length()-7-1);
            if (key.isEmpty())
                multiLineID = true;
        } else if (line.startsWith(QStringLiteral("msgstr"))) {
            value = line.mid(8, line.length()-8-1);
            if (multiLineID && !key.isEmpty() && value.isEmpty())
                multiLineValue = true;
            else
                map[fromGetTextString(key)] = fromGetTextString(value);
        } else if (multiLineID) {
            QString s = line.trimmed();
            if (s.isEmpty() || s[0] != '\"')
                multiLineID = false;
            else
                key += s.mid(1,s.length()-1);
        } else if (multiLineValue) {
            QString s = line.trimmed();
            if (s.isEmpty() || s[0] != '\"') {
                multiLineValue = false;
                multiLineID = false;
                map[fromGetTextString(key)] = fromGetTextString(value);
            }
            else
                value += s.mid(1,s.length()-1);
        }
    }
    return true;
}

bool applyTranslationToXMIFile(const char *fileName, const QStringList &attributes, TranslationMap &translations)
{

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    QXmlStreamReader reader(&file);
    QFile outFile;
    if (!outFile.open(stdout, QIODevice::WriteOnly))
        return false;
    QXmlStreamWriter writer(&outFile);
    writer.setAutoFormatting (true);
    writer.setAutoFormattingIndent(1);
    writer.setCodec(reader.documentEncoding().toLatin1().constData());

    while (!reader.atEnd())
    {
        QXmlStreamReader::TokenType type = reader.readNext();
        switch(type)
        {
        case QXmlStreamReader::ProcessingInstruction:
            writer.writeProcessingInstruction(reader.processingInstructionTarget().toString(), reader.processingInstructionData().toString());
            break;

        case QXmlStreamReader::DTD:
            writer.writeDTD(reader.text().toString());
            break;

        case QXmlStreamReader::StartDocument:
            writer.writeStartDocument(reader.documentVersion().toString());
            break;

        case QXmlStreamReader::StartElement:
        {
            writer.writeStartElement(reader.namespaceUri().toString(), reader.name().toString());
            if (reader.namespaceDeclarations().size() > 0)
            {
                QXmlStreamNamespaceDeclaration ns = reader.namespaceDeclarations().first();
                writer.writeNamespace(ns.namespaceUri().toString(), ns.prefix().toString());
            }
            const QXmlStreamAttributes& readerAttributes = reader.attributes();
            QXmlStreamAttributes writerAttributes;
            for(int index = 0; index < readerAttributes.size(); index++)
            {
                QXmlStreamAttribute attr = readerAttributes[index];
                QString name = attr.qualifiedName().toString();
                if (!attributes.contains(name)) {
                    writerAttributes.append(attr);
                    continue;
                }
                QString value = attr.value().toString();
                if (value.isEmpty()) {
                    writerAttributes.append(attr);
                    continue;
                }
                if (!translations.contains(value))
                {
                    cerr << "could not find translation for attribute '" << qPrintable(name) << "':'" << qPrintable(value) << "'" << std::endl;
                    continue;
                }
                QString newValue = translations[value];
                if (newValue.isEmpty()) {
                    writerAttributes.append(attr);
                    continue;
                }
                //cerr << name.toUtf8().data() << ":" << value.toUtf8().data() << "->" << newValue.toUtf8().data() << endl;
                QXmlStreamAttribute newAttribute(name, newValue);
                writerAttributes.append(newAttribute);
                //qDebug() << writerAttributes;
            }
            writer.writeAttributes(writerAttributes);
            //QString content = xmlReader.readElementText(QXmlStreamReader::SkipChildElements);
            //writer.writeCharacters(content);
            break;
        }

        case QXmlStreamReader::Characters:
            writer.writeCharacters(reader.text().toString());
            break;

        case QXmlStreamReader::Comment:
            writer.writeComment(reader.text().toString());
            break;

        case QXmlStreamReader::EndElement:
            writer.writeEndElement();
            break;

        case QXmlStreamReader::EndDocument:
           writer.writeEndDocument();
           break;

        default:
            break;
        }

    }
    outFile.close();
    return true;
}
