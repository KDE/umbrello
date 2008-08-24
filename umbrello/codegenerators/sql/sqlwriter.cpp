/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Nikolaus Gradwohl  <guru@local-guru.net>      *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "sqlwriter.h"

#include "classifier.h"
#include "operation.h"
#include "umlnamespace.h"
#include "association.h"
#include "attribute.h"
#include "entity.h"
#include "model_utils.h"
#include "uniqueconstraint.h"
#include "foreignkeyconstraint.h"
#include "checkconstraint.h"
#include "umlentityattributelist.h"
#include "umlclassifierlistitemlist.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <QtCore/QList>

SQLWriter::SQLWriter()
{
}

SQLWriter::~SQLWriter()
{
}

void SQLWriter::writeClass(UMLClassifier *c)
{
    UMLEntity* e = static_cast<UMLEntity*>(c);

    if(!e) {
        uDebug()<<"Cannot write entity of NULL concept!";
        return;
    }

    m_pEntity = e;

    QString entityname = cleanName(m_pEntity->getName());

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
    if(forceDoc() || !m_pEntity->getDoc().isEmpty()) {
        sql << m_endl << "--" << m_endl;
        sql<<"-- TABLE: "<<entityname<<m_endl;
        sql<<formatDoc(m_pEntity->getDoc(),"-- ");
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
    UMLClassifierListItemList constrList = m_pEntity->getFilteredList(Uml::ot_UniqueConstraint);
    printUniqueConstraints(sql, constrList);


    // write all foreign key constraints
    constrList = m_pEntity->getFilteredList(Uml::ot_ForeignKeyConstraint);
    printForeignKeyConstraints(sql, constrList);

    // write all check constraints
    constrList = m_pEntity->getFilteredList(Uml::ot_CheckConstraint);
    printCheckConstraints(sql,constrList);

    // write all other indexes
    foreach( UMLEntityAttribute* ea, entAttList ) {
        if ( ea->getIndexType() != Uml::Index )
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
            if (objA->getID() == m_pEntity->getID() && objB->getID() != m_pEntity->getID())
                continue;
            constraintMap[a] = a;
        }
    }

    QMap<UMLAssociation*,UMLAssociation*>::Iterator itor = constraintMap.begin();
    for (;itor != constraintMap.end();itor++) {
        UMLAssociation* a = itor.value();
        sql << "ALTER TABLE "<< entityname
            << " ADD CONSTRAINT " << a->getName() << " FOREIGN KEY ("
            << a->getRoleName(Uml::B) << ") REFERENCES "
            << a->getObject(Uml::A)->getName()
            << " (" << a->getRoleName(Uml::A) << ");" << m_endl;

    }


    file.close();
    emit codeGenerated(m_pEntity, true);
}

Uml::Programming_Language SQLWriter::getLanguage()
{
    return Uml::pl_SQL;
}

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

const QStringList SQLWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords << "access"
        << "add"
        << "all"
        << "alter"
        << "analyze"
        << "and"
        << "any"
        << "as"
        << "asc"
        << "audit"
        << "begin"
        << "between"
        << "boolean"
        << "by"
        << "char"
        << "character"
        << "check"
        << "cluster"
        << "column"
        << "comment"
        << "commit"
        << "compress"
        << "connect"
        << "create"
        << "current"
        << "cursor"
        << "date"
        << "decimal"
        << "default"
        << "delete"
        << "desc"
        << "distinct"
        << "drop"
        << "else"
        << "elsif"
        << "end"
        << "escape"
        << "exception"
        << "exclusive"
        << "execute"
        << "exists"
        << "explain"
        << "false"
        << "file"
        << "float"
        << "for"
        << "from"
        << "function"
        << "grant"
        << "group"
        << "having"
        << "identified"
        << "if"
        << "immediate"
        << "in"
        << "increment"
        << "index"
        << "initial"
        << "insert"
        << "integer"
        << "intersect"
        << "into"
        << "is"
        << "level"
        << "like"
        << "lock"
        << "long"
        << "loop"
        << "maxextents"
        << "minus"
        << "mlslabel"
        << "mode"
        << "modify"
        << "noaudit"
        << "nocompress"
        << "not"
        << "nowait"
        << "null"
        << "number"
        << "of"
        << "offline"
        << "on"
        << "online"
        << "option"
        << "or"
        << "order"
        << "out"
        << "pctfree"
        << "prior"
        << "privileges"
        << "procedure"
        << "public"
        << "raw"
        << "rename"
        << "resource"
        << "return"
        << "revoke"
        << "rollback"
        << "row"
        << "rowid"
        << "rowlabel"
        << "rownum"
        << "rows"
        << "savepoint"
        << "select"
        << "session"
        << "set"
        << "share"
        << "size"
        << "smallint"
        << "some"
        << "start"
        << "successful"
        << "synonym"
        << "sysdate"
        << "table"
        << "then"
        << "to"
        << "trigger"
        << "true"
        << "truncate"
        << "type"
        << "uid"
        << "union"
        << "unique"
        << "update"
        << "user"
        << "using"
        << "validate"
        << "values"
        << "varchar"
        << "varchar2"
        << "varray"
        << "view"
        << "whenever"
        << "where"
        << "with";
    }

    return keywords;
}

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
        sql << m_indentation << cleanName(at->getName()) ;

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
        attrDoc = at->getDoc();
    }

    return;
}

void SQLWriter::printUniqueConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
   foreach( UMLClassifierListItem* cli, constrList ) {
       sql<<m_endl;

       UMLUniqueConstraint* uuc = static_cast<UMLUniqueConstraint*>(cli);
       UMLEntityAttributeList attList = uuc->getEntityAttributeList();

       // print documentation
       sql<<"-- "<<uuc->getDoc();
       sql<<m_endl;

       sql<<"ALTER TABLE "<<cleanName(m_pEntity->getName())
          <<" ADD CONSTRAINT "<<cleanName(uuc->getName());

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

           sql<<cleanName(entAtt->getName());
       }

       sql<<");";

       sql<<m_endl;
   }

}

void SQLWriter::printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
   foreach( UMLClassifierListItem* cli, constrList ) {

       sql<<m_endl;
       UMLForeignKeyConstraint* fkc = static_cast<UMLForeignKeyConstraint*>(cli);

       QMap<UMLEntityAttribute*,UMLEntityAttribute*> attributeMap;
       attributeMap = fkc->getEntityAttributePairs();

       // print documentation
       sql<<"-- "<<fkc->getDoc();
       sql<<m_endl;

       sql<<"ALTER TABLE "<<cleanName(m_pEntity->getName())
          <<" ADD CONSTRAINT "<<cleanName(fkc->getName());

       sql<<" FOREIGN KEY (";

       QList<UMLEntityAttribute*> entAttList = attributeMap.keys();

       bool first = true;
       // the local attributes which refer the attributes of the referenced entity
       foreach( UMLEntityAttribute* entAtt, entAttList ) {
           if ( first )
               first = false;
           else
               sql<<',';

           sql<<cleanName( entAtt->getName() );
       }

       sql<<')';

       sql<<" REFERENCES ";

       UMLEntity* referencedEntity = fkc->getReferencedEntity();
       sql<<cleanName( referencedEntity->getName() );

       sql<<'(';

       QList<UMLEntityAttribute*> refEntAttList = attributeMap.values();

       first = true;
       // the attributes of the referenced entity which are being referenced
       foreach( UMLEntityAttribute* refEntAtt, refEntAttList ) {
          if ( first )
              first = false;
          else
              sql<<',';

          sql<<cleanName( refEntAtt->getName() );
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

void SQLWriter::printIndex(QTextStream& sql, UMLEntity* ent , UMLEntityAttributeList entAttList)
{
    sql<<m_endl;
    sql<<"CREATE INDEX ";

    // we don;t have any name, so we just merge the names of all attributes along with their entity name

    sql<<cleanName( ent->getName() )<<'_';
    foreach( UMLEntityAttribute* entAtt,  entAttList ) {
        sql<<cleanName( entAtt->getName() )<<'_';
    }
    sql<<"index ";

    sql<<" ON "<<cleanName( m_pEntity->getName() )<<'(';

    bool first = true;

    foreach( UMLEntityAttribute* entAtt, entAttList ) {
        if ( first )
            first = false;
        else
            sql<<',';

        sql<<cleanName( entAtt->getName() );
    }

    sql<<");";

    sql<<m_endl;
}

void SQLWriter::printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList )
{
    Q_UNUSED(sql); Q_UNUSED(entAttList);
    // defer to derived classes
}

void SQLWriter::printCheckConstraints(QTextStream& sql,UMLClassifierListItemList constrList)
{
    foreach( UMLClassifierListItem* cli, constrList ) {
        UMLCheckConstraint* chConstr = static_cast<UMLCheckConstraint*>(cli);

        sql<<m_endl;
        // print documentation
        sql<<"-- "<<chConstr->getDoc();

        sql<<m_endl;

        sql<<"ALTER TABLE "<<cleanName( m_pEntity->getName() )
           <<" ADD CONSTRAINT "<<cleanName( chConstr->getName() )
           <<" CHECK ("<<chConstr->getCheckCondition()
           <<" )";

        sql<<m_endl;
   }
}

#include "sqlwriter.moc"
