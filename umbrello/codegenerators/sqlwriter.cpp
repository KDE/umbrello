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

SQLWriter::SQLWriter( QObject *parent, const char *name )
		:CodeGenerator( parent, name) {}

SQLWriter::~SQLWriter() {}

void SQLWriter::writeClass(UMLClassifier *c) {


	if(!c) {
		kdDebug()<<"Cannot write class of NULL concept!\n";
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
		sql<<"\n--\n";
		sql<<"-- TABLE: "<<classname<<endl;
		sql<<formatDoc(c->getDoc(),"-- ");
		sql<<"--  \n\n";
	}

	sql << "CREATE TABLE "<< classname << " ( " << endl;

	if(myClass)
		writeAttributes(myClass,sql);

	UMLAssociationList aggregations = c->getAggregations();
	if( forceSections() || !aggregations.isEmpty() ) {
		for(UMLAssociation* a = aggregations.first(); a; a = aggregations.next()) {
			sql << ",\n\tCONSTRAINT " << a->getName() << " FOREIGN KEY (" << a->getMultiB() <<
			       ") REFERENCES " <<   a->getObjectA()->getName() << "(" << a->getMultiA() << ")";
		}
	}

	sql<<"\n);\n";

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

	bool first = true;
	printAttributes(sql, atpub, first);
	printAttributes(sql, atprot, first);
	printAttributes(sql, atpriv, first);
}

void SQLWriter::printAttributes(QTextStream& sql, UMLAttributeList attributeList, bool first) {
	UMLAttribute* at;
	for (at=attributeList.first();at;at=attributeList.next()) {
		if (!first) {
			sql <<","<<endl;
		} else {
			first = false;
		}
		sql << "\t" << cleanName(at->getName()) << " " << at->getTypeName() << " "
		    << (at->getDoc().isEmpty()?QString(""):at->getDoc())
		    << (at->getInitialValue().isEmpty()?QString(""):QString(" DEFAULT ")+at->getInitialValue());
	}
}
