/*
 *    SPDX-FileCopyrightText: 2026 Luca Sforza <blum@kewbee.de>
 *    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tswriter.h"

#include "umlassociation.h"
#include "umlattribute.h"
#include "umlclassifier.h"
#include "umloperation.h"
#include "umldoc.h"
#include "umlapp.h"
#include "debug_utils.h"
#include "umlenumliteral.h"

#include <QRegularExpression>
#include <QTextStream>
#include <cstdio>

// Helper to map UML visibility to TS keywords
QString toTSVisibility(Uml::Visibility::Enum v) {
    switch(v) {
        case Uml::Visibility::Public: return QStringLiteral("public");
        case Uml::Visibility::Protected: return QStringLiteral("protected");
        case Uml::Visibility::Private: return QStringLiteral("private");
        default: return QStringLiteral("public"); // Default to public in TS
    }
}

TSWriter::TSWriter()
{
}

TSWriter::~TSWriter()
{
}

/**
 * Call this method to generate Actionscript code for a UMLClassifier.
 * @param c   the class you want to generate code for
 */
void TSWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        logWarn0("TSWriter::writeClass: Cannot write class of NULL classifier");
        return;
    }
    UMLEnum *asEnum = dynamic_cast<UMLEnum*>(c);
    if (asEnum) {
        writeEnum(asEnum); // Call your helper method
        return;            // STOP here so we don't write it as a class
    }

    QString classname = cleanName(c->name());
    QString fileName = findFileName(c, QStringLiteral(".ts"));
    if (!fileName.endsWith(QStringLiteral(".ts"), Qt::CaseInsensitive)) {
        fileName += QStringLiteral(".ts");
    }

    if (fileName.isEmpty()) {
        Q_EMIT codeGenerated(c, false);
        return;
    }

    QFile fileTs;
    if (!openFile(fileTs, fileName)) {
        Q_EMIT codeGenerated(c, false);
        return;
    }
    QTextStream ts(&fileTs);

    // 1. Header & License
    QString str = getHeadingFile(QStringLiteral(".ts"));
    if (!str.isEmpty()) {
        str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
        str.replace(QRegularExpression(QStringLiteral("%filepath%")), fileTs.fileName());
        ts << str << m_endl;
    }

    // 2. Imports
    // In TS, we usually import classes from other files.
    UMLPackageList includes;
    findObjectsRelated(c, includes);
    for (UMLPackage* conc : includes) {
        if (conc == c) continue; // Don't import self
        QString headerName = findFileName(conc, QStringLiteral(".ts")); // Get filename without extension for import
        if (!headerName.isEmpty()) {
            ts << "import { " << cleanName(conc->name()) << " } from \"./" << headerName << "\";" << m_endl;
        }
    }
    ts << m_endl;

    // 3. Class Documentation
    if (forceDoc() || !c->doc().isEmpty()) {
        ts << "/**" << m_endl;
        ts << formatDoc(c->doc(), QStringLiteral(" * "));
        ts << " */" << m_endl;
    }

    // 4. Class Declaration
    QString inheritanceStr; // Default constructor creates empty string
    UMLClassifierList superclasses = c->getSuperClasses();
    if (!superclasses.isEmpty()) {
        // Fix: Use QStringLiteral for concatenation
        inheritanceStr = QStringLiteral(" extends ") + cleanName(superclasses.first()->name());
    }

    // Check if Interface or Class
    QString typeKeyword = c->isInterface() ? QStringLiteral("interface") : QStringLiteral("class");
    if (c->isAbstract() && !c->isInterface()) typeKeyword = QStringLiteral("abstract class");

    ts << "export " << typeKeyword << " " << classname << inheritanceStr << " {" << m_endl;

    // 5. Attributes (Fields)
    UMLAttributeList atl = c->getAttributeList();
    for (UMLAttribute *at : atl) {
        if (forceDoc() || !at->doc().isEmpty()) {
            ts << m_indentation << "/** " << formatDoc(at->doc(), QStringLiteral("")) << " */" << m_endl;
        }

        QString vis = toTSVisibility(at->visibility());
        QString type = cleanName(at->getTypeName());
        if (type.isEmpty()) type = QStringLiteral("any"); // Fallback

        // Format: private myVar: string;
        ts << m_indentation << vis << " " << cleanName(at->name()) << ": " << type;

        // Initial Value
        if (!at->getInitialValue().isEmpty()) {
            ts << " = " << at->getInitialValue();
        }
        ts << ";" << m_endl;
    }

    // 6. Associations (as Fields)
    writeAssociation(classname, c, ts);

    // 7. Constructor
    // Interfaces don't have constructors in TS
    if (!c->isInterface()) {
        ts << m_endl << m_indentation << "constructor() {" << m_endl;
        if (!superclasses.isEmpty()) {
            ts << m_indentation << m_indentation << "super();" << m_endl;
        }
        // Here you could initialize arrays or other complex objects if needed
        ts << m_indentation << "}" << m_endl;
    }

    // 8. Operations (Methods)
    UMLOperationList ops(c->getOperationsList());
    writeOperations(classname, &ops, ts, c->isInterface());

    ts << "}" << m_endl; // End Class

    fileTs.close();
    Q_EMIT codeGenerated(c, true);
    Q_EMIT showGeneratedFile(fileTs.fileName());
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void TSWriter::writeEnum(UMLEnum *e)
{
    QString classname = cleanName(e->name());
    QString fileName = findFileName(e, QStringLiteral(".ts"));
    if (!fileName.endsWith(QStringLiteral(".ts"), Qt::CaseInsensitive)) {
        fileName += QStringLiteral(".ts");
    }

    if (fileName.isEmpty()) {
        Q_EMIT codeGenerated(e, false);
        return;
    }

    QFile fileTs;
    if (!openFile(fileTs, fileName)) {
        Q_EMIT codeGenerated(e, false);
        return;
    }
    QTextStream ts(&fileTs);

    // 1. Header
    QString str = getHeadingFile(QStringLiteral(".ts"));
    if (!str.isEmpty()) {
        str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
        str.replace(QRegularExpression(QStringLiteral("%filepath%")), fileTs.fileName());
        ts << str << m_endl;
    }
    ts << m_endl;

    // 2. Documentation
    if (forceDoc() || !e->doc().isEmpty()) {
        ts << "/**" << m_endl;
        ts << formatDoc(e->doc(), QStringLiteral(" * "));
        ts << " */" << m_endl;
    }

    // 3. Enum Definition
    ts << "export enum " << classname << " {" << m_endl;

    // --- FIX: Use UMLObject::ot_EnumLiteral and UMLClassifierListItemList ---
    UMLClassifierListItemList literals = e->getFilteredList(UMLObject::ot_EnumLiteral);

    for (UMLClassifierListItem *obj : literals) {
        // --- FIX: Safe cast now that header is included ---
        UMLEnumLiteral *literal = static_cast<UMLEnumLiteral*>(obj);

        QString name = cleanName(literal->name());
        QString value = literal->value();

        if (forceDoc() || !literal->doc().isEmpty()) {
            ts << m_indentation << "/** " << formatDoc(literal->doc(), QStringLiteral("")) << " */" << m_endl;
        }

        ts << m_indentation << name;
        if (!value.isEmpty()) {
            ts << " = " << value;
        }
        ts << "," << m_endl;
    }

    ts << "}" << m_endl;

    fileTs.close();
    Q_EMIT codeGenerated(e, true);
    Q_EMIT showGeneratedFile(fileTs.fileName());
}

/**
 * Write a list of associations.
 * @param classname   the name of the class
 * @param c           UML classifier
 * @param ts          output stream for the JS file
 */
void TSWriter::writeAssociation(QString& classname, UMLClassifier* c, QTextStream &ts)
{
    // Aggregations and Compositions are treated as class fields in TS
    UMLAssociationList assocList = c->getAggregations();
    assocList.append(c->getCompositions());
    assocList.append(c->getAssociations());
    assocList.append(c->getUniAssociationToBeImplemented());

    int i = 0;
    for (UMLAssociation *a : assocList) {
        i++;
        Uml::RoleType::Enum role = (a->getObject(Uml::RoleType::A)->name() == classname ? Uml::RoleType::B : Uml::RoleType::A);

        QString roleName = cleanName(a->getRoleName(role));
        if (roleName.isEmpty()) {
            roleName = QString::number(i) + QStringLiteral("_association");
        }

        // Doc
        if (forceDoc() || !a->doc().isEmpty()) {
            ts << m_indentation << "/** " << formatDoc(a->doc(), QStringLiteral("")) << " */" << m_endl;
        }

        // Multiplicity check
        bool okCvt;
        int nMulti = a->getMultiplicity(role).toInt(&okCvt, 10);
        bool isCollection = !a->getMultiplicity(role).isEmpty() && (!okCvt || nMulti > 1);

        QString typeName = cleanName(a->getObject(role)->name());

        // Visibility - Associations usually imply private/protected properties with accessors,
        // but for simple projects, public or private fields are fine.
        QString vis = QStringLiteral("public");

        if (isCollection) {
            // e.g. public students: Student[] = [];
            ts << m_indentation << vis << " " << roleName << ": " << typeName << "[] = [];" << m_endl;
        } else {
            // e.g. public teacher: Teacher;
            // Note: In strict TS, this might error if not initialized in constructor.
            // We can add ' | null = null' or '!' to suppress, but let's keep it simple.
            ts << m_indentation << vis << " " << roleName << ": " << typeName << ";" << m_endl;
        }
    }
}

/**
 * Write a list of class operations.
 * @param classname   the name of the class
 * @param opList      the list of operations
 * @param ts          output stream for the JS file
 * @param isInterface is interface
 */
void TSWriter::writeOperations(QString classname, UMLOperationList *opList, QTextStream &ts, bool isInterface)
{
    Q_UNUSED(classname);

    for (UMLOperation* op : *opList) {
        ts << m_endl;

        // Doc
        if (forceDoc() || !op->doc().isEmpty()) {
            ts << m_indentation << "/**" << m_endl;
            ts << formatDoc(op->doc(), m_indentation + QStringLiteral(" * "));
            // Param docs
            UMLAttributeList atl = op->getParameterList();
            for (UMLAttribute* at : atl) {
                if (!at->doc().isEmpty()) {
                    ts << m_indentation << " * @param " << cleanName(at->name()) << " " << at->doc() << m_endl;
                }
            }
            ts << m_indentation << " */" << m_endl;
        }

        // Signature
        QString vis = toTSVisibility(op->visibility());
        QString isAbstract = (op->isAbstract() && !isInterface) ? QStringLiteral("abstract ") : QStringLiteral("");
        QString name = cleanName(op->name());
        QString returnType = cleanName(op->getTypeName());

        if (returnType.isEmpty() || returnType == QStringLiteral("void")) returnType = QStringLiteral("void");

        // Parameters
        QStringList paramStrs;
        UMLAttributeList atl = op->getParameterList();
        for (UMLAttribute* at : atl) {
            QString pType = cleanName(at->getTypeName());
            if (pType.isEmpty()) pType = QStringLiteral("any");
            paramStrs << cleanName(at->name()) + QStringLiteral(": ") + pType;
        }

        // Output: public abstract doWork(a: string): void;
        ts << m_indentation << vis << " " << isAbstract << name << "(" << paramStrs.join(QStringLiteral(", ")) << "): " << returnType;

        // Body
        if (isInterface || op->isAbstract()) {
            ts << ";" << m_endl;
        } else {
            ts << " {" << m_endl;
            // User provided source code?
            QString sourceCode = op->getSourceCode();
            if (!sourceCode.isEmpty()) {
                ts << formatSourceCode(sourceCode, m_indentation + m_indentation);
            } else {
                // Return default if needed
                if (returnType != QStringLiteral("void")) {
                    ts << m_indentation << m_indentation << "return null; // TODO: Implement return" << m_endl;
                }
            }
            ts << m_indentation << "}" << m_endl;
        }
    }
}

/**
 * Returns "TypeScript".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage::Enum TSWriter::language() const
{
    return Uml::ProgrammingLanguage::TypeScript;
}

/**
 * Get list of reserved keywords.
 * @return   the list of reserved keywords
 */
QStringList TSWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords
        << QStringLiteral("break")
        << QStringLiteral("case")
        << QStringLiteral("const")
        << QStringLiteral("continue")
        << QStringLiteral("default")
        << QStringLiteral("else")
        << QStringLiteral("false")
        << QStringLiteral("for")
        << QStringLiteral("function")
        << QStringLiteral("if")
        << QStringLiteral("in")
        << QStringLiteral("new")
        << QStringLiteral("return")
        << QStringLiteral("switch")
        << QStringLiteral("this")
        << QStringLiteral("true")
        << QStringLiteral("var")
        << QStringLiteral("while")
        << QStringLiteral("with")
        << QStringLiteral("class")
        << QStringLiteral("interface");
    }

    return keywords;
}

QStringList TSWriter::defaultDatatypes() const
{
    QStringList l;

    // --- Primitives ---
    l.append(QStringLiteral("string"));
    l.append(QStringLiteral("number"));
    l.append(QStringLiteral("boolean"));
    l.append(QStringLiteral("bigint"));
    l.append(QStringLiteral("symbol"));
    l.append(QStringLiteral("undefined"));
    l.append(QStringLiteral("null"));

    // --- Special Types ---
    l.append(QStringLiteral("any"));
    l.append(QStringLiteral("unknown")); // Safer alternative to 'any'
    l.append(QStringLiteral("never"));   // Unreachable states
    l.append(QStringLiteral("void"));    // Return type for functions
    l.append(QStringLiteral("object"));  // Non-primitive types

    // --- Common Built-ins (Bases for Generics) ---
    // Note: These usually require type arguments (e.g., Array<string>)
    l.append(QStringLiteral("Array"));
    l.append(QStringLiteral("Set"));
    l.append(QStringLiteral("Map"));
    l.append(QStringLiteral("Promise"));
    l.append(QStringLiteral("Date"));
    l.append(QStringLiteral("RegExp"));
    l.append(QStringLiteral("Error"));

    return l;
}
