/***************************************************************************
                          sqlwriter.cpp  -  description
                             -------------------
    begin                : 10.02.2003
    copyright            : (C) 2003 Nikolaus Gradwohl
    email                : guru@local-guru.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sqlwriter.h"

#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "../class.h"
#include "../operation.h"
#include "../umlnamespace.h"
#include "../association.h"
#include "../attribute.h"

SQLWriter::SQLWriter( UMLDoc *parent, const char *name )
		:SimpleCodeGenerator( parent, name) {}

SQLWriter::~SQLWriter() {}

void SQLWriter::writeClass(UMLClassifier *c) {


	if(!c) {
		kdDebug()<<"Cannot write class of NULL concept!" << endl;
		return;
	}

	UMLClass * myClass = dynamic_cast<UMLClass*>(c);
	QString classname = cleanName(c->getName());
	QString fileName = c->getName().lower();

	//find an appropriate name for our file
	fileName = findFileName(c,".sql");
	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}

	QFile file;
	if( !openFile(file,fileName+".sql") ) {
		emit codeGenerated(c, false);
		return;
	}

	//Start generating the code!!

	QTextStream sql(&file);
	//try to find a heading file (license, coments, etc)
	QString str;
	str = getHeadingFile(".sql");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),fileName);
		str.replace(QRegExp("%filepath%"),file.name());
		sql<<str<<endl;
	}

	//Write class Documentation if there is somthing or if force option
	if(forceDoc() || !c->getDoc().isEmpty()) {
		sql << m_newLineEndingChars << "--" << m_newLineEndingChars;
		sql<<"-- TABLE: "<<classname<<endl;
		sql<<formatDoc(c->getDoc(),"-- ");
		sql << "--  " << m_newLineEndingChars << m_newLineEndingChars;
	}

	sql << "CREATE TABLE "<< classname << " ( " << endl;

	if(myClass)
		writeAttributes(myClass,sql);

	UMLAssociationList aggregations = c->getAggregations();
	if( forceSections() || !aggregations.isEmpty() ) {
		for(UMLAssociation* a = aggregations.first(); a; a = aggregations.next()) {
			if( a->getObject(A)->getID()==c->getID() ) {
				sql << m_newLineEndingChars << "-- CONSTRAINT " << a->getName() << " FOREIGN KEY (" << a->getMulti(B) <<
					") REFERENCES " <<   a->getObject(A)->getName() << "(" << a->getMulti(A) << ")";
			} else {
				sql << "," << m_newLineEndingChars << m_indentation << "CONSTRAINT " << a->getName() << " FOREIGN KEY (" << a->getMulti(B) <<
					") REFERENCES " <<   a->getObject(A)->getName() << "(" << a->getMulti(A) << ")";
			}
		}
	}

	sql << m_newLineEndingChars << ");" << m_newLineEndingChars;

	file.close();
	emit codeGenerated(c, true);
}


void SQLWriter::writeAttributes(UMLClass *c, QTextStream &sql) {
	UMLAttributeList* atl;
	UMLAttributeList atpub, atprot, atpriv;
	atpub.setAutoDelete(false);
	atprot.setAutoDelete(false);
	atpriv.setAutoDelete(false);

	//sort attributes by scope and see if they have a default value
	atl = c->getFilteredAttributeList();
	for(UMLAttribute* at=atl->first(); at ; at=atl->next()) {
		switch(at->getScope()) {
		case Uml::Public:
			atpub.append(at);
			break;
		case Uml::Protected:
			atprot.append(at);
			break;
		case Uml::Private:
			atpriv.append(at);
			break;
		}
	}

	printAttributes(sql, atpub);
	printAttributes(sql, atprot);
	printAttributes(sql, atpriv);
}

void SQLWriter::printAttributes(QTextStream& sql, UMLAttributeList attributeList) {
	bool first = true;
	QString attrDoc = "";
	UMLAttribute* at;

	for (at=attributeList.first();at;at=attributeList.next())
	{
		// print , after attribute
		if (first == false) {
			sql <<",";
		} else {
			first = false;
		}

		// print documentation/comment of last attribute at end of line
		if (attrDoc.isEmpty() == false)
		{
			sql << " -- " << attrDoc << endl;
		} else {
			sql << endl;
		}

		// write the attribute
		sql << m_indentation << cleanName(at->getName()) << " " << at->getTypeName() << " "
		    << (at->getInitialValue().isEmpty()?QString(""):QString(" DEFAULT ")+at->getInitialValue());

		// now get documentation/comment of current attribute
		attrDoc = at->getDoc();
	}

	// print documentation/comment at end of line
	if (attrDoc.isEmpty() == false)
	{
		sql << " -- " << attrDoc << endl;
	} else {
		sql << endl;
	}

	return;
}

QString SQLWriter::getLanguage() {
        return "SQL";
}

/**
 * checks whether type is "SQLWriter"
 *
 * @param type
 */
bool SQLWriter::isType (QString & type)
{
   if(type == "SQLWriter")
        return true;
   return false;
}

/**
 * List of reserved keywords for this code generator.
 *
 * Just add new keywords, then mark all lines and
 * pipe it through the external 'sort' program.
 */
static const char *ReservedWords[] = {
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
  NULL
};

/**
 * get list of reserved keywords
 */
const char **
SQLWriter::getReservedKeywords() {
  return ReservedWords;
}

#include "sqlwriter.moc"
