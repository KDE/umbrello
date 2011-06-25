/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Nikolaus Gradwohl  <guru@local-guru.net>      *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "sqlwriter.h"

#include "association.h"
#include "attribute.h"
#include "checkconstraint.h"
#include "classifier.h"
#include "debug_utils.h"
#include "entity.h"
#include "foreignkeyconstraint.h"
#include "model_utils.h"
#include "operation.h"
#include "uniqueconstraint.h"
#include "umlentityattributelist.h"
#include "umlclassifierlistitemlist.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <QtCore/QList>

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
    UMLEntity* e = static_cast<UMLEntity*>(c);

    if(!e) {
        uDebug()<<"Cannot write entity of NULL concept!";
        return;
    }

    m_pEntity = e;

    QString entityname = cleanName(m_pEntity->name());

    //find an appropriate name for our file
    QString fileName = findFileName(m_pEntity, ".sql");
    if (fileName.isEmpty()) {
        emit codeGenerated(m_pEntity, false);
        return;
    }

    QFile file;
    if( !openFile(file, fileName) ) {
        emit codeGenerated(m_pEntity, false);
        return;
    }

    //Start generating the code!!

    QTextStream sql(&file);
    //try to find a heading file (license, coments, etc)
    QString str;
    str = getHeadingFile(".sql");
    if(!str.isEmpty()) {
        str.replace(QRegExp("%filename%"),fileName);
        str.replace(QRegExp("%filepath%"),file.fileName());
        sql<<str<<m_endl;
    }

    //Write class Documentation if there is somthing or if force option
    if(forceDoc() || !m_pEntity->doc().isEmpty()) {
        sql << m_endl << "--" << m_endl;
        sql<<"-- TABLE: "<<entityname<<m_endl;
        sql<<formatDoc(m_pEntity->doc(),"-- ");
        sql << "--  " << m_endl << m_endl;
    }

    // write all entity attributes
    UMLEntityAttributeList entAttList = m_pEntity->getEntityAttributes();

    sql << "CREATE TABLE "<< entityname << " (";

    printEntityAttributes(sql, entAttList);

    sql << m_endl << ");" << m_endl;

    // auto increments
    UMLEntityAttributeList autoIncrementList;
    foreach( UMLEntityAttribute* entAtt, entAttList ) {
        autoIncrementList.append(entAtt);
    }

    printAutoIncrements( sql, autoIncrementList );


    // write all unique constraint ( including primary key )
    UMLClassifierListItemList constrList = m_pEntity->getFilteredList(UMLObject::ot_UniqueConstraint);
    printUniqueConstraints(sql, constrList);


    // write all foreign key constraints
    constrList = m_pEntity->getFilteredList(UMLObject::ot_ForeignKeyConstraint);
    printForeignKeyConstraints(sql, constrList);

    // write all check constraints
    constrList = m_pEntity->getFilteredList(UMLObject::ot_CheckConstraint);
    printCheckConstraints(sql,constrList);

    // write all other indexes
    foreach( UMLEntityAttribute* ea, entAttList ) {
        if ( ea->indexType() != UMLEntityAttribute::Index )
            continue;
        UMLEntityAttributeList tempList;
        tempList.append( ea );
        printIndex( sql, m_pEntity, tempList );
        tempList.clear();
    }

        QMap<UMLAssociation*,UMLAssociation*> constraintMap; // so we don't repeat constraint
    UMLAssociationList relationships = m_pEntity->getRelationships();
    if( forceSections() || !relationships.isEmpty() ) {
        foreach ( UMLAssociation* a , relationships ) {
            UMLObject *objA = a->getObject(Uml::A);
            UMLObject *objB = a->getObject(Uml::B);
            if (objA->id() == m_pEntity->id() && objB->id() != m_pEntity->id())
                continue;
            constraintMap[a] = a;
        }
    }

    QMap<UMLAssociation*,UMLAssociation*>::Iterator itor = constraintMap.begin();
    for (; itor != constraintMap.end(); ++itor) {
        UMLAssociation* a = itor.value();
        sql << "ALTER TABLE "<< entityname
            << " ADD CONSTRAINT " << a->name() << " FOREIGN KEY ("
            << a->getRoleName(Uml::B) << ") REFERENCES "
            << a->getObject(Uml::A)->name()
            << " (" << a->getRoleName(Uml::A) << ");" << m_endl;
    }

    file.close();
    emit codeGenerated(m_pEntity, true);
}

/**
 * Returns "SQL".
 */
Uml::ProgrammingLanguage SQLWriter::language() const
{
    return Uml::ProgrammingLanguage::SQL;
}

/**
 * Reimplement method from CodeGenerator.
 */
QStringList SQLWriter::defaultDatatypes()
{
    QStringList l;
    l.append("varchar");
    l.append("tinyint");
    l.append("smallint");
    l.append("mediumint");
    l.append("bigint");
    l.append("float");
    l.append("double");
    l.append("decimal");
    l.append("date");
    l.append("datetime");
    l.append("time");
    l.append("timestamp");
    l.append("year");
    l.append("char");
    l.append("tinyblob");
    l.append("blob");
    l.append("mediumblob");
    l.append("longblob");
    l.append("tinytext");
    l.append("text");
    l.append("mediumtext");
    l.append("longtext");
    l.append("enum");
    l.append("set");
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
            keywords.append(reserved_words[i]);
        }
    }

    return keywords;
}

/**
 * Prints out attributes as columns in the table.
 * @param sql the stream we should print to
 * @param entityAttributeList the attributes to be printed
 */
void SQLWriter::printEntityAttributes(QTextStream& sql, UMLEntityAttributeList entityAttributeList )
{
    QString attrDoc = "";

    bool first = true;

    foreach ( UMLEntityAttribute* at, entityAttributeList ) {
       // print , after attribute
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
        sql<< ' ' << at->getTypeName();

        // the length ( if there's some value)
        QString lengthStr = at->getValues().trimmed();
        bool ok;
        uint length = lengthStr.toUInt(&ok);

        if ( ok ) {
            sql<<'('<<length<<')';
        }

        // write the attributes ( unsigned, zerofill etc)
        QString attributes = at->getAttributes().trimmed();
        if ( !attributes.isEmpty() ) {
            sql<<' '<<attributes;
        }

        if ( !at->getNull() ) {
            sql<<" NOT NULL ";
        }

        // write any default values
        sql<< (at->getInitialValue().isEmpty()?QString(""):QString(" DEFAULT ")+at->getInitialValue());

        // now get documentation/comment of current attribute
        attrDoc = at->doc();
    }
}

/**
 * Prints out unique constraints (including primary key ) as "ALTER TABLE" statements.
 * @param sql the stream we should print to
 * @param constrList the unique constraints to be printed
 */
void SQLWriter::printUniqueConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
   foreach( UMLClassifierListItem* cli, constrList ) {
       sql<<m_endl;

       UMLUniqueConstraint* uuc = static_cast<UMLUniqueConstraint*>(cli);
       UMLEntityAttributeList attList = uuc->getEntityAttributeList();

       // print documentation
       sql<<"-- "<<uuc->doc();
       sql<<m_endl;

       sql<<"ALTER TABLE "<<cleanName(m_pEntity->name())
          <<" ADD CONSTRAINT "<<cleanName(uuc->name());

       if ( m_pEntity->isPrimaryKey( uuc ) )
           sql<<" PRIMARY KEY ";
       else
           sql<<" UNIQUE ";

       sql<<'(';

       bool first = true;
       foreach( UMLEntityAttribute* entAtt, attList ) {
           if ( first )
               first = false;
           else
               sql<<",";

           sql<<cleanName(entAtt->name());
       }

       sql<<");";

       sql<<m_endl;
   }

}

/**
 * Prints out foreign key constraints as "ALTER TABLE" statements.
 * @param sql the stream we should print to
 * @param constrList the foreignkey constraints to be printed
 */
void SQLWriter::printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
   foreach( UMLClassifierListItem* cli, constrList ) {

       sql<<m_endl;
       UMLForeignKeyConstraint* fkc = static_cast<UMLForeignKeyConstraint*>(cli);

       QMap<UMLEntityAttribute*,UMLEntityAttribute*> attributeMap;
       attributeMap = fkc->getEntityAttributePairs();

       // print documentation
       sql<<"-- "<<fkc->doc();
       sql<<m_endl;

       sql<<"ALTER TABLE "<<cleanName(m_pEntity->name())
          <<" ADD CONSTRAINT "<<cleanName(fkc->name());

       sql<<" FOREIGN KEY (";

       QList<UMLEntityAttribute*> entAttList = attributeMap.keys();

       bool first = true;
       // the local attributes which refer the attributes of the referenced entity
       foreach( UMLEntityAttribute* entAtt, entAttList ) {
           if ( first )
               first = false;
           else
               sql<<',';

           sql<<cleanName( entAtt->name() );
       }

       sql<<')';

       sql<<" REFERENCES ";

       UMLEntity* referencedEntity = fkc->getReferencedEntity();
       sql<<cleanName( referencedEntity->name() );

       sql<<'(';

       QList<UMLEntityAttribute*> refEntAttList = attributeMap.values();

       first = true;
       // the attributes of the referenced entity which are being referenced
       foreach( UMLEntityAttribute* refEntAtt, refEntAttList ) {
          if ( first )
              first = false;
          else
              sql<<',';

          sql<<cleanName( refEntAtt->name() );
       }

       sql<<')';

       UMLForeignKeyConstraint::UpdateDeleteAction updateAction, deleteAction;
       updateAction = fkc->getUpdateAction();
       deleteAction = fkc->getDeleteAction();

       sql<<" ON UPDATE "<<Model_Utils::updateDeleteActionToString(updateAction);
       sql<<" ON DELETE "<<Model_Utils::updateDeleteActionToString(deleteAction);

       sql<<';';

       sql<<m_endl;
   }

}

/**
 * Prints out Indexes as "CREATE INDEX " statements.
 * @param sql The Stream we should print to
 * @param ent The Entity's attributes on which we want to create an Index
 * @param entAttList The list of entityattributes to create an index upon
 */
void SQLWriter::printIndex(QTextStream& sql, UMLEntity* ent , UMLEntityAttributeList entAttList)
{
    sql<<m_endl;
    sql<<"CREATE INDEX ";

    // we don;t have any name, so we just merge the names of all attributes along with their entity name

    sql<<cleanName( ent->name() )<<'_';
    foreach( UMLEntityAttribute* entAtt,  entAttList ) {
        sql<<cleanName( entAtt->name() )<<'_';
    }
    sql<<"index ";

    sql<<" ON "<<cleanName( m_pEntity->name() )<<'(';

    bool first = true;

    foreach( UMLEntityAttribute* entAtt, entAttList ) {
        if ( first )
            first = false;
        else
            sql<<',';

        sql<<cleanName( entAtt->name() );
    }

    sql<<");";

    sql<<m_endl;
}

/**
 * Handles AutoIncrements.
 * The derived classes provide the actual body.
 * @param sql The Stream we should print to
 * @param entAttList The List of Entity Attributes that we want to auto increment
 */
void SQLWriter::printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList )
{
    Q_UNUSED(sql); Q_UNUSED(entAttList);
    // defer to derived classes
}

/**
 * Prints out Check Constraints as "ALTER TABLE" statements.
 * @param sql The stream we should print to
 * @param constrList The checkConstraints to be printed
 */
void SQLWriter::printCheckConstraints(QTextStream& sql,UMLClassifierListItemList constrList)
{
    foreach( UMLClassifierListItem* cli, constrList ) {
        UMLCheckConstraint* chConstr = static_cast<UMLCheckConstraint*>(cli);

        sql<<m_endl;
        // print documentation
        sql<<"-- "<<chConstr->doc();

        sql<<m_endl;

        sql<<"ALTER TABLE "<<cleanName( m_pEntity->name() )
           <<" ADD CONSTRAINT "<<cleanName( chConstr->name() )
           <<" CHECK ("<<chConstr->getCheckCondition()
           <<" )";

        sql<<m_endl;
   }
}

#include "sqlwriter.moc"
