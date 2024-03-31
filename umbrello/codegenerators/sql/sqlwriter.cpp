/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Nikolaus Gradwohl <guru@local-guru.net>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "sqlwriter.h"

#include "association.h"
#include "attribute.h"
#include "checkconstraint.h"
#include "classifier.h"
#include "debug_utils.h"
#include "enum.h"
#include "entity.h"
#include "foreignkeyconstraint.h"
#include "model_utils.h"
#include "operation.h"
#include "uniqueconstraint.h"
#include "umlentityattributelist.h"
#include "umlclassifierlistitemlist.h"
#include "uml.h"  // only needed for log{Warn,Error}

#include <KLocalizedString>
#include <KMessageBox>

#include <QFile>
#include <QList>
#include <QRegularExpression>
#include <QTextStream>

static const char *reserved_words[] = {
    "access",
    "add",
    "all",
    "alter",
    "analyze",
    "and",
    "any",
    "as",
    "asc",
    "audit",
    "begin",
    "between",
    "boolean",
    "by",
    "char",
    "character",
    "check",
    "cluster",
    "column",
    "comment",
    "commit",
    "compress",
    "connect",
    "create",
    "current",
    "cursor",
    "date",
    "decimal",
    "default",
    "delete",
    "desc",
    "distinct",
    "drop",
    "else",
    "elsif",
    "end",
    "escape",
    "exception",
    "exclusive",
    "execute",
    "exists",
    "explain",
    "false",
    "file",
    "float",
    "for",
    "from",
    "function",
    "grant",
    "group",
    "having",
    "identified",
    "if",
    "immediate",
    "in",
    "increment",
    "index",
    "initial",
    "insert",
    "integer",
    "intersect",
    "into",
    "is",
    "level",
    "like",
    "lock",
    "long",
    "loop",
    "maxextents",
    "minus",
    "mlslabel",
    "mode",
    "modify",
    "noaudit",
    "nocompress",
    "not",
    "nowait",
    "null",
    "number",
    "of",
    "offline",
    "on",
    "online",
    "option",
    "or",
    "order",
    "out",
    "pctfree",
    "prior",
    "privileges",
    "procedure",
    "public",
    "raw",
    "rename",
    "resource",
    "return",
    "revoke",
    "rollback",
    "row",
    "rowid",
    "rowlabel",
    "rownum",
    "rows",
    "savepoint",
    "select",
    "session",
    "set",
    "share",
    "size",
    "smallint",
    "some",
    "start",
    "successful",
    "synonym",
    "sysdate",
    "table",
    "then",
    "to",
    "trigger",
    "true",
    "truncate",
    "type",
    "uid",
    "union",
    "unique",
    "update",
    "user",
    "using",
    "validate",
    "values",
    "varchar",
    "varchar2",
    "varray",
    "view",
    "whenever",
    "where",
    "with",
    0
};

SQLWriter::SQLWriter()
    : m_pEntity(0)
{
}

SQLWriter::~SQLWriter()
{
}

/**
 * Call this method to generate sql code for a UMLClassifier.
 * @param c the class to generate code for
 */
void SQLWriter::writeClass(UMLClassifier *c)
{
    UMLEntity* e = c->asUMLEntity();

    if (!e) {
        logError2("SQLWriter::writeClass: Invalid cast from '%1' to UMLEntity* %2",
                  c->name(), c->baseTypeStr());
        return;
    }

    m_pEntity = e;

    QString entityname = cleanName(m_pEntity->name());

    //find an appropriate name for our file
    QString fileName = findFileName(m_pEntity, QStringLiteral(".sql"));
    if (fileName.isEmpty()) {
        emit codeGenerated(m_pEntity, false);
        return;
    }

    QFile file;
    if (!openFile(file, fileName)) {
        emit codeGenerated(m_pEntity, false);
        return;
    }

    //Start generating the code!!

    QTextStream sql(&file);
    //try to find a heading file (license, comments, etc)
    QString str;
    str = getHeadingFile(QStringLiteral(".sql"));
    if (!str.isEmpty()) {
        str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
        str.replace(QRegularExpression(QStringLiteral("%filepath%")), file.fileName());
        sql << str << m_endl;
    }

    //Write class Documentation if there is something or if force option
    if (forceDoc() || !m_pEntity->doc().isEmpty()) {
        sql << m_endl << "--" << m_endl;
        sql << "-- TABLE: " << entityname << m_endl;
        sql << formatDoc(m_pEntity->doc(),QStringLiteral("-- "));
        sql << "--  " << m_endl << m_endl;
    }

    // write all entity attributes
    UMLEntityAttributeList entAttList = m_pEntity->getEntityAttributes();
    if (language() == Uml::ProgrammingLanguage::PostgreSQL) {
        foreach(UMLEntityAttribute *at, entAttList) {
            if (at->getType()->baseType() == UMLObject::ot_Enum) {
                const UMLEnum *_enum = at->getType()->asUMLEnum();
                if (m_enumsGenerated.contains(at->getTypeName()))
                    continue;
                m_enumsGenerated.append(at->getTypeName());
                sql << "CREATE TYPE " << at->getTypeName() << " AS ENUM (";
                QString delimiter(QStringLiteral(""));
                UMLClassifierListItemList enumLiterals = _enum->getFilteredList(UMLObject::ot_EnumLiteral);
                foreach (UMLClassifierListItem* enumLiteral, enumLiterals) {
                    sql << delimiter << "'" << enumLiteral->name() << "'";
                    delimiter = QStringLiteral(", ");
                }
                sql << ");\n";
            }
        }
    }

    sql << "CREATE TABLE " <<  entityname << " (";

    printEntityAttributes(sql, entAttList);

    sql << m_endl << ");" << m_endl;

    // auto increments
    UMLEntityAttributeList autoIncrementList;
    foreach(UMLEntityAttribute* entAtt, entAttList) {
        autoIncrementList.append(entAtt);
    }

    printAutoIncrements(sql, autoIncrementList);


    // write all unique constraint (including primary key)
    UMLClassifierListItemList constrList = m_pEntity->getFilteredList(UMLObject::ot_UniqueConstraint);
    printUniqueConstraints(sql, constrList);


    // write all foreign key constraints
    constrList = m_pEntity->getFilteredList(UMLObject::ot_ForeignKeyConstraint);
    printForeignKeyConstraints(sql, constrList);

    // write all check constraints
    constrList = m_pEntity->getFilteredList(UMLObject::ot_CheckConstraint);
    printCheckConstraints(sql, constrList);

    // write all other indexes
    foreach(UMLEntityAttribute* ea, entAttList) {
        if (ea->indexType() != UMLEntityAttribute::Index)
            continue;
        UMLEntityAttributeList tempList;
        tempList.append(ea);
        printIndex(sql, m_pEntity, tempList);
        tempList.clear();
    }

        QMap<UMLAssociation*, UMLAssociation*> constraintMap; // so we don't repeat constraint
    UMLAssociationList relationships = m_pEntity->getRelationships();
    if (forceSections() || !relationships.isEmpty()) {
        foreach (UMLAssociation* a, relationships) {
            UMLObject *objA = a->getObject(Uml::RoleType::A);
            UMLObject *objB = a->getObject(Uml::RoleType::B);
            if (objA->id() == m_pEntity->id() && objB->id() != m_pEntity->id())
                continue;
            constraintMap[a] = a;
        }
    }

    QMap<UMLAssociation*, UMLAssociation*>::Iterator itor = constraintMap.begin();
    for (; itor != constraintMap.end(); ++itor) {
        UMLAssociation* a = itor.value();
        sql << "ALTER TABLE " <<  entityname
            << " ADD CONSTRAINT " << a->name() << " FOREIGN KEY ("
            << a->getRoleName(Uml::RoleType::B) << ") REFERENCES "
            << a->getObject(Uml::RoleType::A)->name()
            << " (" << a->getRoleName(Uml::RoleType::A) << ");" << m_endl;
    }

    file.close();
    emit codeGenerated(m_pEntity, true);
    emit showGeneratedFile(file.fileName());
}

/**
 * Returns "SQL".
 */
Uml::ProgrammingLanguage::Enum SQLWriter::language() const
{
    return Uml::ProgrammingLanguage::SQL;
}

/**
 * Reimplement method from CodeGenerator.
 */
QStringList SQLWriter::defaultDatatypes() const
{
    QStringList l;
    l.append(QStringLiteral("blob"));
    l.append(QStringLiteral("bigint"));
    l.append(QStringLiteral("char"));
    l.append(QStringLiteral("float"));
    l.append(QStringLiteral("date"));
    l.append(QStringLiteral("datetime"));
    l.append(QStringLiteral("decimal"));
    l.append(QStringLiteral("double"));
    l.append(QStringLiteral("enum"));
    l.append(QStringLiteral("longblob"));
    l.append(QStringLiteral("longtext"));
    l.append(QStringLiteral("mediumblob"));
    l.append(QStringLiteral("mediumint"));
    l.append(QStringLiteral("mediumtext"));
    l.append(QStringLiteral("set"));
    l.append(QStringLiteral("smallint"));
    l.append(QStringLiteral("text"));
    l.append(QStringLiteral("time"));
    l.append(QStringLiteral("timestamp"));
    l.append(QStringLiteral("tinyblob"));
    l.append(QStringLiteral("tinyint"));
    l.append(QStringLiteral("tinytext"));
    l.append(QStringLiteral("varchar"));
    l.append(QStringLiteral("year"));
    return l;
}

/**
 * Get list of reserved keywords.
 */
QStringList SQLWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        for (int i = 0; reserved_words[i]; ++i) {
            keywords.append(QLatin1String(reserved_words[i]));
        }
    }

    return keywords;
}

/**
 * Prints out attributes as columns in the table.
 * @param sql the stream we should print to
 * @param entityAttributeList the attributes to be printed
 */
void SQLWriter::printEntityAttributes(QTextStream& sql, UMLEntityAttributeList entityAttributeList)
{
    QString attrDoc;

    bool first = true;

    foreach (UMLEntityAttribute* at, entityAttributeList) {
       // print, after attribute
         if (first) {
             first = false;
         } else {
             sql <<",";
         }

        // print documentation/comment of last attribute at end of line
        if (attrDoc.isEmpty() == false) {
            sql << " -- " << attrDoc << m_endl;
        } else {
            sql << m_endl;
        }

        // write the attribute name
        sql << m_indentation << cleanName(at->name()) ;

        // the datatype
        if (language() == Uml::ProgrammingLanguage::MySQL &&
                at->getType() && at->getType()->baseType() == UMLObject::ot_Enum) {
            const UMLEnum *_enum = at->getType()->asUMLEnum();
            sql << " ENUM(";
            QString delimiter(QStringLiteral(""));
            UMLClassifierListItemList enumLiterals = _enum->getFilteredList(UMLObject::ot_EnumLiteral);
            foreach (UMLClassifierListItem* enumLiteral, enumLiterals) {
                sql << delimiter << "'" << enumLiteral->name() << "'";
                delimiter = QStringLiteral(", ");
            }
            sql << ')';
        } else
            sql <<  ' ' << at->getTypeName();

        // the length (if there's some value)
        QString lengthStr = at->getValues().trimmed();
        bool ok;
        uint length = lengthStr.toUInt(&ok);

        if (ok) {
            sql << '(' << length << ')';
        }

        // write the attributes (unsigned, zerofill etc)
        QString attributes = at->getAttributes().trimmed();
        if (!attributes.isEmpty()) {
            sql << ' ' << attributes;
        }

        if (!at->getNull()) {
            sql << " NOT NULL ";
        }

        // write any default values
        if (!at->getInitialValue().isEmpty()) {
            if (at->getType()->baseType() == UMLObject::ot_Enum) {
                sql << QStringLiteral(" DEFAULT '") << at->getInitialValue() << QStringLiteral("'");
            } else {
                sql << QStringLiteral(" DEFAULT ") + at->getInitialValue();
            }
        }
        // now get documentation/comment of current attribute
        attrDoc = at->doc();
    }
}

/**
 * Prints out unique constraints (including primary key) as "ALTER TABLE" statements.
 * @param sql the stream we should print to
 * @param constrList the unique constraints to be printed
 */
void SQLWriter::printUniqueConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
   foreach(UMLClassifierListItem* cli, constrList) {
       const UMLUniqueConstraint* uuc = cli->asUMLUniqueConstraint();
       if (!uuc) {
           logError2("SQLWriter::printUniqueConstraints: Invalid cast from '%1' to UMLUniqueConstraint* %2",
                     cli->name(), cli->baseTypeStr());
           return;
       }
       sql << m_endl;
       UMLEntityAttributeList attList = uuc->getEntityAttributeList();

       // print documentation
       sql << "-- " << uuc->doc();
       sql << m_endl;

       sql << "ALTER TABLE " << cleanName(m_pEntity->name())
           <<" ADD CONSTRAINT " << cleanName(uuc->name());

       if (m_pEntity->isPrimaryKey(uuc))
           sql << " PRIMARY KEY ";
       else
           sql << " UNIQUE ";

       sql << '(';

       bool first = true;
       foreach(UMLEntityAttribute* entAtt, attList) {
           if (first)
               first = false;
           else
               sql << ",";

           sql << cleanName(entAtt->name());
       }

       sql << ");";

       sql << m_endl;
   }

}

/**
 * Prints out foreign key constraints as "ALTER TABLE" statements.
 * @param sql the stream we should print to
 * @param constrList the foreignkey constraints to be printed
 */
void SQLWriter::printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
   foreach(UMLClassifierListItem* cli, constrList) {
       UMLForeignKeyConstraint* fkc = cli->asUMLForeignKeyConstraint();
       if (!fkc) {
           logError2("SQLWriter::printForeignKeyConstraints: Invalid cast from '%1' to UMLForeignKeyConstraint* %2",
                     cli->name(), cli->baseTypeStr());
           return;
       }
       sql << m_endl;

       QMap<UMLEntityAttribute*, UMLEntityAttribute*> attributeMap;
       attributeMap = fkc->getEntityAttributePairs();

       // print documentation
       sql << "-- " << fkc->doc();
       sql << m_endl;

       sql << "ALTER TABLE " << cleanName(m_pEntity->name())
           <<" ADD CONSTRAINT " << cleanName(fkc->name());

       sql << " FOREIGN KEY (";

       QList<UMLEntityAttribute*> entAttList = attributeMap.keys();

       bool first = true;
       // the local attributes which refer the attributes of the referenced entity
       foreach(UMLEntityAttribute* entAtt, entAttList) {
           if (first)
               first = false;
           else
               sql << ',';

           sql << cleanName(entAtt->name());
       }

       sql << ')';

       sql << " REFERENCES ";

       UMLEntity* referencedEntity = fkc->getReferencedEntity();
       sql << cleanName(referencedEntity->name());

       sql << '(';

       QList<UMLEntityAttribute*> refEntAttList = attributeMap.values();

       first = true;
       // the attributes of the referenced entity which are being referenced
       foreach(UMLEntityAttribute* refEntAtt, refEntAttList) {
          if (first)
              first = false;
          else
              sql << ',';

          sql << cleanName(refEntAtt->name());
       }

       sql << ')';

       UMLForeignKeyConstraint::UpdateDeleteAction updateAction, deleteAction;
       updateAction = fkc->getUpdateAction();
       deleteAction = fkc->getDeleteAction();

       sql << " ON UPDATE " << Model_Utils::updateDeleteActionToString(updateAction);
       sql << " ON DELETE " << Model_Utils::updateDeleteActionToString(deleteAction);

       sql << ';';

       sql << m_endl;
   }

}

/**
 * Prints out Indexes as "CREATE INDEX " statements.
 * @param sql The Stream we should print to
 * @param ent The Entity's attributes on which we want to create an Index
 * @param entAttList The list of entityattributes to create an index upon
 */
void SQLWriter::printIndex(QTextStream& sql, UMLEntity* ent, UMLEntityAttributeList entAttList)
{
    sql << m_endl;
    sql << "CREATE INDEX ";

    // we don;t have any name, so we just merge the names of all attributes along with their entity name

    sql << cleanName(ent->name()) << '_';
    foreach(UMLEntityAttribute* entAtt,  entAttList) {
        sql << cleanName(entAtt->name()) << '_';
    }
    sql << "index ";

    sql << " ON " << cleanName(ent->name()) << '(';

    bool first = true;

    foreach(UMLEntityAttribute* entAtt, entAttList) {
        if (first)
            first = false;
        else
            sql << ',';

        sql << cleanName(entAtt->name());
    }

    sql << ");";

    sql << m_endl;
}

/**
 * Handles AutoIncrements.
 * The derived classes provide the actual body.
 * @param sql The Stream we should print to
 * @param entAttList The List of Entity Attributes that we want to auto increment
 */
void SQLWriter::printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList)
{
    Q_UNUSED(sql); Q_UNUSED(entAttList);
    // defer to derived classes
}

/**
 * Prints out Check Constraints as "ALTER TABLE" statements.
 * @param sql The stream we should print to
 * @param constrList The checkConstraints to be printed
 */
void SQLWriter::printCheckConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
    foreach(UMLClassifierListItem* cli, constrList) {
        const UMLCheckConstraint* chConstr = cli->asUMLCheckConstraint();
        if (!chConstr) {
            logError2("SQLWriter::printCheckConstraints: Invalid cast from '%1' to UMLCheckConstraint* %2",
                      cli->name(), cli->baseTypeStr());
            return;
        }

        sql << m_endl;
        // print documentation
        sql << "-- " << chConstr->doc();

        sql << m_endl;

        sql << "ALTER TABLE " << cleanName(m_pEntity->name())
            <<" ADD CONSTRAINT " << cleanName(chConstr->name())
            <<" CHECK (" << chConstr->getCheckCondition()
            <<");";

        sql << m_endl;
   }
}

