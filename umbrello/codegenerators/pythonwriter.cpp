/***************************************************************************
                          pythonwriter.h  -  description
                             -------------------
    begin                : Sat Dec 21 2002
    author               : Vincent Decorges
    email                : vincent.decorges@eivd.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pythonwriter.h"


#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "../umldoc.h"
#include "../association.h"
#include "../attribute.h"
#include "../classifier.h"
#include "../operation.h"
#include "../umlnamespace.h"

PythonWriter::PythonWriter( QObject *parent, const char *name ) :
  CodeGenerator( parent, name) {}

PythonWriter::~PythonWriter() {}

void PythonWriter::setSpaceIndent(int number) {
	spaceIndent = "";
	for (int i = 0; i < number; i++) {
		spaceIndent.append(' ');
	}
}

int PythonWriter::getSpaceIndent(void) {
	return spaceIndent.length();
}

void PythonWriter::writeClass(UMLClassifier *c) {
	if(!c) {
		kdDebug()<<"Cannot write class of NULL concept!\n";
		return;
	}

	spaceIndent = "    ";

	QString classname = cleanName(c->getName());
	QString fileName = c->getName();

	QPtrList<UMLAssociation> generalizations = c->getGeneralizations();
	QPtrList<UMLAssociation> aggregations = c->getAggregations();
	QPtrList<UMLAssociation> compositions = c->getCompositions();
	UMLAssociation *a;

	//find an appropriate name for our file
	fileName = findFileName(c,".py");
	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}

	QChar first = fileName.at(0);
	//Replace the first letter of the filename because
	//python class begin with an upper caracter (convention)
	first = first.upper();
	fileName = fileName.replace(0, 1, first);

	QFile fileh;
	if( !openFile(fileh,fileName+".py") ) {
		emit codeGenerated(c, false);
		return;
	}
	QTextStream h(&fileh);

	//////////////////////////////
	//Start generating the code!!
	/////////////////////////////


	//try to find a heading file (license, coments, etc)
	QString str;

	str = getHeadingFile(".py");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"), fileName+".py");
		str.replace(QRegExp("%filepath%"), fileh.name());
		h<<str<<endl;
	}


	//write includes and take namespaces into account
	QPtrList<UMLClassifier> includes;
	findObjectsRelated(c,includes);
	UMLClassifier* conc;
	for(conc = includes.first(); conc ;conc = includes.next()) {
		QString headerName = findFileName(conc, ".py");
		if ( !headerName.isEmpty() ) {
			first = headerName.at(0);
			first = first.upper();
			headerName = headerName.replace(0, 1, first);
			h<<"from "<<headerName<<" import *"<<endl;
		}
	}
	h<<endl;

	h<<"class "<<classname<<(generalizations.count() > 0 ? " (":"");
	int i;

	for (a = generalizations.first(), i = generalizations.count();
	     a && i; a = generalizations.next(), i--) {

		UMLObject* obj = m_doc->findUMLObject(a->getRoleBId());
		h<<cleanName(obj->getName())<<(i>1?", ":"");
	}


	h<<(generalizations.count() > 0 ? ")":"")<<":"<<endl<<endl;

	if(forceDoc() || !c->getDoc().isEmpty()) {
		h<<spaceIndent<<"\"\"\""<<endl;
		h<<spaceIndent<<c->getDoc()<<endl;
		h<<spaceIndent<<":version:"<<endl;
		h<<spaceIndent<<":author:"<<endl;
		h<<spaceIndent<<"\"\"\""<<endl<<endl;
	}

	//operations
	writeOperations(c,h);

	//finish files
	h<<endl<<endl;

	//close files and notfiy we are done
	fileh.close();
	emit codeGenerated(c, true);
}


////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void PythonWriter::writeOperations(UMLClassifier *c,QTextStream &h) {

	//Lists to store operations  sorted by scope
	QPtrList<UMLOperation> *opl;
	QPtrList<UMLOperation> oppub,opprot,oppriv;

	oppub.setAutoDelete(false);
	opprot.setAutoDelete(false);
	oppriv.setAutoDelete(false);

	//sort operations by scope first and see if there are abstract methods
	opl = c->getOpList();
	for(UMLOperation *op = opl->first(); op ; op = opl->next()) {
		switch(op->getScope()) {
			case Uml::Public:
				oppub.append(op);
				break;
			case Uml::Protected:
				opprot.append(op);
				break;
			case Uml::Private:
				oppriv.append(op);
				break;
		}
	}

	QString classname(cleanName(c->getName()));

	//write operations to file
	if(forceSections() || !oppub.isEmpty()) {
		writeOperations(classname,oppub,h,PUBLIC);
	}

	if(forceSections() || !opprot.isEmpty()) {
		writeOperations(classname,opprot,h,PROTECTED);
	}

	if(forceSections() || !oppriv.isEmpty()) {
		writeOperations(classname,oppriv,h,PRIVATE);
	}

}

void PythonWriter::writeOperations(QString /*classname*/, QPtrList<UMLOperation> &opList,
				   QTextStream &h, Access access) {
	UMLOperation *op;
	QPtrList<UMLAttribute> *atl;
	UMLAttribute *at;

	QString sAccess;

	switch (access) {

	case PUBLIC:
		sAccess = QString("");
		break;
	case PRIVATE:
		sAccess = QString("__");
		break;
	case PROTECTED:
		sAccess = QString("_");
		break;
	}


	for(op=opList.first(); op ; op=opList.next()) {
		atl = op -> getParmList();
		//write method doc if we have doc || if at least one of the params has doc
		bool writeDoc = forceDoc() || !op->getDoc().isEmpty();
		for(at = atl->first(); at ; at = atl -> next())
			writeDoc |= !at->getDoc().isEmpty();

		h<< spaceIndent << "def "<< sAccess + cleanName(op->getName()) << "(self";

		int j=0;
		for( at = atl->first(); at ;at = atl->next(),j++) {
			h << ", " << cleanName(at->getName())
			<< (!(at->getInitialValue().isEmpty()) ?
			    (QString(" = ")+at->getInitialValue()) :
			    QString(""));
		}

		h<<"):"<<endl;

		if( writeDoc )  //write method documentation
		{
			h<<spaceIndent<<spaceIndent<<"\"\"\""<<endl;
			h<<spaceIndent<<spaceIndent<<op->getDoc()<<endl<<endl;

			for(at = atl->first(); at ; at = atl -> next())  //write parameter documentation
			{
				if(forceDoc() || !at->getDoc().isEmpty()) {
					h<<spaceIndent<<spaceIndent<<"@param "<<at->getTypeName()<<
						" " << cleanName(at->getName());
					h<<" : "<<at->getDoc()<<endl;
				}
			}//end for : write parameter documentation
			h<<spaceIndent<<spaceIndent<<"@return " + op->getReturnType()<<" :"<<endl;
			h<<spaceIndent<<spaceIndent<<"@since"<<endl;
			h<<spaceIndent<<spaceIndent<<"@author"<<endl;
			h<<spaceIndent<<spaceIndent<<"\"\"\""<<endl;
		}
		h<<spaceIndent<<spaceIndent<<"pass"<<endl<<endl;

	}//end for
}




