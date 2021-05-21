/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    Basec on Cpp ImplementationHelperItem

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "implementationitem.h"

#include "helpers.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/codecompletion/codecompletionmodel.h>

#include "declarations/classmethoddeclaration.h"

#include <ktexteditor/document.h>
#include <kicon.h>
#include <klocalizedstring.h>
#include <KTextEditor/View>
#include <language/duchain/duchainutils.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/types/integraltype.h>

using namespace KDevelop;

namespace Php
{

#define RETURN_CACHED_ICON(name) {static QIcon icon(KIcon(name).pixmap(QSize(16, 16))); return icon;}

QVariant ImplementationItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const
{
    QVariant ret = NormalDeclarationCompletionItem::data(index, role, model);
    switch (role) {
    case Qt::DecorationRole:
        if (index.column() == KTextEditor::CodeCompletionModel::Icon) {
            switch (m_type) {
            case Override:
            case OverrideVar:
                RETURN_CACHED_ICON("CTparents");
            case Implement:
                RETURN_CACHED_ICON("CTsuppliers");
            }
        }
        break;
    case Qt::DisplayRole:
        if (index.column() == KTextEditor::CodeCompletionModel::Prefix) {
            QString prefix;
            switch (m_type) {
            case Override:
            case OverrideVar:
                prefix = i18n("Override");
                break;
            case Implement:
                prefix = i18n("Implement");
                break;
            }

            ret = prefix + ' ' + ret.toString();
        }
        //TODO column == Name - required?
        break;
    case KTextEditor::CodeCompletionModel::ItemSelected: {
        DUChainReadLocker lock(DUChain::lock());
        if (declaration().data()) {
            QualifiedIdentifier parentScope = declaration()->context()->scopeIdentifier(true);
            return i18n("From %1", parentScope.toString());
        }
    }
    break;
    case KTextEditor::CodeCompletionModel::InheritanceDepth:
        return QVariant(0);
    default:
        //pass
        break;
    }

    return ret;
}

void ImplementationItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    DUChainReadLocker lock(DUChain::lock());

    QString replText;

    if (m_declaration) {
        //TODO:respect custom code styles

        // get existing modifiers so we can respect the user's choice of public/protected and final
        QStringList modifiers = getMethodTokens(document->text(KTextEditor::Range(KTextEditor::Cursor::start(), word.start())));
        // get range to replace
        KTextEditor::Range replaceRange(word);
        if (!modifiers.isEmpty()) {
            // TODO: is there no easy API to map QString Index to a KTextEditor::Cursor ?!
            QString methodText = document->text(KTextEditor::Range(KTextEditor::Cursor::start(), word.start()));
            methodText = methodText.left(methodText.lastIndexOf(modifiers.last(), -1, Qt::CaseInsensitive));
            replaceRange.start() = KTextEditor::Cursor(methodText.count('\n'), methodText.length() - methodText.lastIndexOf('\n') - 1);
        }

        // get indentation
        QString indentation;
        {
            QString currentLine = document->line(replaceRange.start().line());
            indentation = getIndentation(currentLine);

            if ( !currentLine.isEmpty() && currentLine != indentation ) {
                // since theres some non-whitespace in this line, skip to the enxt one
                replText += '\n' + indentation;
            }

            if (indentation.isEmpty()) {
                // use a minimal indentation
                // TODO: respect code style
                indentation = "  ";
                replText += indentation;
            }
        }

        #if 0
        //Disabled, because not everyone writes phpdoc for every function
        //TODO: move to a phpdoc helper
        // build phpdoc comment
        {
            QualifiedIdentifier parentClassIdentifier;
            if (DUContext* pctx = m_declaration->context()) {
                parentClassIdentifier = pctx->localScopeIdentifier();
            } else {
                kDebug() << "completion item for implementation has no parent context!";
            }

            replText += "/**\n" + indentation + " * ";
            // insert old comment:
            const QString indentationWithExtra = "\n" + indentation + " *";
            replText += m_declaration->comment().replace('\n', indentationWithExtra.toAscii().constData());
            replText += "\n" + indentation + " * @overload " + m_declaration->internalContext()->scopeIdentifier(true).toString();
            replText += "\n" + indentation + " **/\n" + indentation;
        }
        #endif

        // write function signature

        // copy existing modifiers
        if (!modifiers.isEmpty()) {
            // the tokens are in a bad order and there's no reverse method or similar, so we can't simply join the tokens
            QStringList::const_iterator i = modifiers.constEnd() - 1;
            while (true) {
                replText += (*i) + ' ';
                if (i == modifiers.constBegin()) {
                    break;
                } else {
                    --i;
                }
            }
        }

        QString functionName;
        bool isConstructorOrDestructor = false;
        bool isInterface = false;

        if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(m_declaration.data())) {
            // NOTE: it should _never_ be private - but that's the completionmodel / context / worker's job
            if (!modifiers.contains("public") && !modifiers.contains("protected")) {
                if (member->accessPolicy() == Declaration::Protected) {
                    replText += "protected ";
                } else {
                    replText += "public ";
                }
            }
            if (!modifiers.contains("static") && member->isStatic()) {
                replText += "static ";
            }
            functionName = member->identifier().toString();

            ClassMethodDeclaration* method = dynamic_cast<ClassMethodDeclaration*>(m_declaration.data());
            if (method) {
                functionName = method->prettyName().str();
                isConstructorOrDestructor = method->isConstructor() || method->isDestructor();
            }

            if (member->context() && member->context()->owner()) {
                ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(member->context()->owner());
                if (classDec) {
                    isInterface = (classDec->classType() == ClassDeclarationData::Interface);
                }
            }
        } else {
            kDebug() << "completion item for implementation was not a classfunction declaration!";
            functionName = m_declaration->identifier().toString();
        }

        if (m_type == ImplementationItem::OverrideVar) {
            replText += "$" + functionName + " = ";
        } else {
            if (!modifiers.contains("function")) {
                replText += "function ";
            }

            replText += functionName;

            {
                // get argument list
                QString arguments;
                createArgumentList(*this, arguments, 0, true);
                replText += arguments;
            }

            QString arguments;
            QVector<Declaration*> parameters;
            if (DUChainUtils::getArgumentContext(m_declaration.data()))
                parameters = DUChainUtils::getArgumentContext(m_declaration.data())->localDeclarations();
            arguments = '(';
            bool first = true;
            foreach(Declaration* dec, parameters) {
                if (first)
                    first = false;
                else
                    arguments += ", ";

                arguments += '$' + dec->identifier().toString();
            }
            arguments += ')';

            bool voidReturnType = false;
            if (FunctionType::Ptr::dynamicCast(m_declaration->abstractType())) {
                AbstractType::Ptr retType = FunctionType::Ptr::staticCast(m_declaration->abstractType())->returnType();
                if (retType->equals(new IntegralType(IntegralType::TypeVoid))) {
                    voidReturnType = true;
                }
            }

            replText += QString("\n%1{\n%1    ").arg(indentation);
            if (isInterface || m_type == ImplementationItem::Implement) {
            } else if (!isConstructorOrDestructor && !voidReturnType) {
                replText += QString("$ret = parent::%2%3;\n%1    return $ret;").arg(indentation).arg(functionName).arg(arguments);
            } else {
                replText += QString("parent::%1%2;").arg(functionName).arg(arguments);
            }
            replText += QString("\n%1}\n%1")
                    .arg(indentation);

        }


        //TODO: properly place the cursor inside the {} part
        document->replaceText(replaceRange, replText);

    } else {
        kDebug() << "Declaration disappeared";
    }
}

}
