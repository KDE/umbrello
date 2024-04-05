/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "helpers.h"
#include "item.h"

#include <QList>
#include <QVariant>
#include <QTextFormat>
#include <QStringList>

#include <language/duchain/declaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/integraltype.h>

#include <KTextEditor/ConfigInterface>
#include <KTextEditor/Document>

using namespace KDevelop;
namespace Php
{

void createArgumentList(const NormalDeclarationCompletionItem& item, QString& ret, QList<QVariant>* highlighting,
                        bool phpTypeHinting)
{
    ///@todo also highlight the matches of the previous arguments, they are given by ViableFunction
    Declaration* dec(item.declaration().data());

    int textFormatStart = 0;
    QTextFormat normalFormat(QTextFormat::CharFormat);
    QTextFormat highlightFormat; //highlightFormat is invalid, so kate uses the match-quality dependent color.

    AbstractFunctionDeclaration* decl = dynamic_cast<AbstractFunctionDeclaration*>(dec);
    FunctionType::Ptr functionType = dec->type<FunctionType>();

    if (functionType && decl) {

        QVector<Declaration*> parameters;
        if (DUChainUtils::getArgumentContext(dec))
            parameters = DUChainUtils::getArgumentContext(dec)->localDeclarations();

        uint defaultParamNum = 0;

        int firstDefaultParam = parameters.count() - decl->defaultParametersSize();

        ret = '(';
        bool first = true;
        int num = 0;

        Q_FOREACH(Declaration* dec, parameters) {
            if (first)
                first = false;
            else
                ret += QLatin1String(", ");

            bool doHighlight = false;
            QTextFormat doFormat = normalFormat;

//       if( num < f.matchedArguments )
//       {
            doHighlight = true;
            doFormat = QTextFormat(QTextFormat::CharFormat);

//         if( parameterConversion != conversions.end() ) {
//           //Interpolate the color
//           quint64 badMatchColor = 0xff7777ff; //Full blue
//           quint64 goodMatchColor = 0xff77ff77; //Full green
//
//           uint totalColor = (badMatchColor*(Cpp::MaximumConversionResult-(*parameterConversion).rank) + goodMatchColor*(*parameterConversion).rank)/Cpp::MaximumConversionResult;
//
//           doFormat.setBackground( QBrush(totalColor) );
//
//           ++parameterConversion;
//         }
//       }

            if (doHighlight) {
                if (highlighting && ret.length() != textFormatStart) {
                    //Add a default-highlighting for the passed text
                    *highlighting <<  QVariant(textFormatStart);
                    *highlighting << QVariant(ret.length() - textFormatStart);
                    *highlighting << QVariant(normalFormat);
                    textFormatStart = ret.length();
                }
            }

            if (num < functionType->arguments().count()) {
                if (AbstractType::Ptr type = functionType->arguments().at(num)) {
                    // when php-like type hinting is requested only add types for arrays and classes
                    if (!phpTypeHinting
                            || (type->whichType() == AbstractType::TypeIntegral
                                && type.cast<IntegralType>()->dataType() == IntegralType::TypeArray)
                            || type->whichType() == AbstractType::TypeStructure) {
                        ret += type->toString() + ' ';
                    }
                }
            }

            ret += '$' + dec->identifier().toString();

            if (doHighlight) {
                if (highlighting && ret.length() != textFormatStart) {
                    *highlighting <<  QVariant(textFormatStart);
                    *highlighting << QVariant(ret.length() - textFormatStart);
                    *highlighting << doFormat;
                    textFormatStart = ret.length();
                }
            }


            if (num >= firstDefaultParam) {
                ret += " = " + decl->defaultParameters()[defaultParamNum].str();
                ++defaultParamNum;
            }

            ++num;
        }
        ret += ')';

        if (highlighting && ret.length() != textFormatStart) {
            *highlighting <<  QVariant(textFormatStart);
            *highlighting << QVariant(ret.length());
            *highlighting << normalFormat;
            textFormatStart = ret.length();
        }
        return;
    }
}

QStringList getMethodTokens(QString text)
{
    QStringList tokens;


    text = text.trimmed();
    if (text.endsWith(QStringLiteral("function"), Qt::CaseInsensitive)) {
        tokens << QStringLiteral("function");
        text = text.left(text.length() - 8);
    }

    QStringList possibleTokens;
    possibleTokens << QStringLiteral("private");
    possibleTokens << QStringLiteral("public");
    possibleTokens << QStringLiteral("protected");
    possibleTokens << QStringLiteral("static");
    possibleTokens << QStringLiteral("abstract");
    possibleTokens << QStringLiteral("final");

    while (!possibleTokens.isEmpty()) {
        bool foundAnything = false;
        text = text.trimmed();
        Q_FOREACH(const QString &token, possibleTokens) {
            if (text.endsWith(token, Qt::CaseInsensitive)) {
                tokens << token;
                text = text.left(text.length() - token.length());
                foundAnything = true;
                possibleTokens.removeOne(token);
                break;
            }
        }
        if (!foundAnything) {
            break;
        }
    }

    return tokens;
}

const QString indentString(KTextEditor::Document *document)
{
    KTextEditor::ConfigInterface *iface = qobject_cast<KTextEditor::ConfigInterface *>(document);
    if (!iface)
        return QStringLiteral("    ");
    if (iface->configValue(QStringLiteral("replace-tabs")).toBool()) {
        QVariant iWidth = iface->configValue(QStringLiteral("indent-width"));
        if (iWidth.isValid())
            return QString(iWidth.toUInt(), ' ');
        /*
         * Provide a default implementation if current KatePart
         * does not handle "indent-width".
         */
        return QStringLiteral("    ");
    }
    return QStringLiteral("\t");
}

QString getIndentation( const QString &line ) {
    return line.left(line.indexOf(QRegularExpression(QStringLiteral("\\S")), 0));
}

}
