/***************************************************************************
                          cppwriter.cpp  -  description
                             -------------------
    begin                : Mon Jun 17 2002
    copyright            : (C) 2002 by Luis De la Parra Blum
    email                : luis@delaparra.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cppwriter.h"

#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "../umldoc.h"
#include "../class.h"
#include "../association.h"
#include "../attribute.h"
#include "../operation.h"
#include "../umlnamespace.h"


CppWriter::CppWriter( QObject *parent, const char *name )
		:CodeGenerator( parent, name) {}

CppWriter::~CppWriter() {}


void CppWriter::writeClass(UMLClassifier *c) {
	if(!c) {
		kdDebug()<<"Cannot write class of NULL concept!\n";
		return;
	}

	QString classname = cleanName(c->getName());
	QString fileName = c->getName().lower();

	//find an appropiate name for our file
	fileName = findFileName(c,".h");
	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}

	QFile fileh, filecpp;
	if( !openFile(fileh,fileName+".h") || !openFile(filecpp,fileName+".cpp")) {
		emit codeGenerated(c, false);
		return;
	}
	QTextStream h(&fileh),cpp(&filecpp);

	//////////////////////////////
	//Start generating the code!!
	/////////////////////////////


	//try to find a heading file (license, coments, etc)
	QString str;

	str = getHeadingFile(".h");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),fileName+".h");
		str.replace(QRegExp("%filepath%"),fileh.name());
		h<<str<<endl;
	}

	str = getHeadingFile(".cpp");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),fileName+".cpp");
		str.replace(QRegExp("%filepath%"),filecpp.name());
		cpp<<str<<endl;
	}


	QString hashDefine = classname.upper().simplifyWhiteSpace().replace(QRegExp(" "),  "_");

	h << "\n#ifndef "<< hashDefine + "_H" << endl;
	h << "#define "<< hashDefine + "_H\n\n";
	cpp << "\n#include \"" << fileName << ".h\"\n\n";

	//write includes and take namespaces into account
	QPtrList<UMLClassifier> includes;
	findObjectsRelated(c,includes);
	UMLClassifier* conc;
	for(conc = includes.first(); conc ;conc = includes.next()) {
		QString headerName = findFileName(conc, ".h");
		if (headerName.isEmpty()) {
			h<<"#include \""<<findFileName(conc,".h")<<".h\"\n";
		}
	}
	h<<"\n";
	for(conc = includes.first(); conc ; conc = includes.next()) {
		if(conc->getPackage()!=c->getPackage() && !conc->getPackage().isEmpty()) {
			h<<"using "<<cleanName(conc->getPackage())<<"::"<<cleanName(conc->getName())<<";\n";
		}
	}

	if(!c->getPackage().isEmpty()) {
		h<<"\nnamespace "<<cleanName(c->getPackage())<<"{\n\n";
		cpp<<"\nusing namespace "<<cleanName(c->getPackage())<<";\n\n";
	}

	//Write class Documentation if there is somthing or if force option
	if(forceDoc() || !c->getDoc().isEmpty()) {
		h<<"\n/**\n";
		h<<"  * class "<<classname<<endl;
		h<<formatDoc(c->getDoc(),"  * ");
		h<<"  */\n\n";
	}


	QPtrList<UMLAssociation> generalizations = c->getGeneralizations();
	QPtrList<UMLAssociation> aggregations = c->getAggregations();
	QPtrList<UMLAssociation> compositions = c->getCompositions();
	UMLAssociation *a;

	//check if class is abstract and / or has abstract methods
	if(c->getAbstract() && !hasAbstractOps(c))
		h<<"/******************************* Abstract Class ****************************\n  "
		<<classname<<" does not have any pure virtual methods, but its author\n"
		<<"  defined it as an abstract class, so you should not use it directly.\n"
		<<"  Inherit from it instead and create only objects from the derived classes\n"
		<<"*****************************************************************************/\n\n";

	h<<"class "<<classname<<(generalizations.count() > 0 ? " : ":"");
	int i;
	for (a = generalizations.first(), i = generalizations.count();
	        a && i;
	        a = generalizations.next(), i--) {
		UMLObject* obj = m_doc->findUMLObject(a->getRoleBId());
		h<<"public "<<cleanName(obj->getName())<<(i>1?", ":"");
	}
	h<<"\n{\n";


	//operations
	writeOperations(c,h,cpp);

	//associations
	if( forceSections() || !aggregations.isEmpty()) {
		h<<"\n/**Aggregations: */\n";
		for (a = aggregations.first(); a; a = aggregations.next()) {
			h<<"private:\n";
			//maybe we should parse the string here and take multiplicty into account to decide
			//which container to use.
			UMLObject *o = m_doc->findUMLObject(a->getRoleAId());
			QString typeName = cleanName(o->getName());
			if (a->getMultiA().isEmpty())
				h << typeName << " *m_" << typeName << ";\n";
			else
				h << "vector<" << typeName << "*> "
				<< typeName.lower() << "Vector;" << endl;
		}//end for
	}

	if( forceSections() || !compositions.isEmpty()) {
		h<<"\n/**Compositions: */\n";
		for (a = compositions.first(); a; a = compositions.next()) {
			// see comment on Aggregation about multiplicity...
			UMLObject *o = m_doc->findUMLObject(a->getRoleAId());
			QString typeName = cleanName(o->getName());
			if(a->getMultiA().isEmpty())
				h << typeName << " m_" << typeName << ";\n";
			else
				h << "vector<" << typeName << "> "
				<< typeName.lower() << "Vector;\n";
		}
	}


	//probably we need to check for concept being class much earlier
	UMLClass * myClass = dynamic_cast<UMLClass *>(c);

	//attributes
	if(myClass)
		writeAttributes(myClass,h);

	//finish files
	h<<"\n};\n\n";
	if(!c->getPackage().isEmpty())
		h<<"};  //end of class namespace\n";
	h<<"#endif // " << hashDefine + "_H\n";

	if(myClass && hasDefaultValueAttr(myClass)) {
		QPtrList<UMLAttribute> *atl = myClass->getAttList();
		cpp<<"void "<<classname<<"::initAttributes( )\n{\n";
		for(UMLAttribute *at = atl->first(); at ; at = atl->next())
			if(!at->getInitialValue().isEmpty())
				cpp<<"  "<<cleanName(at->getName())<<" = "<<at->getInitialValue()<<";\n";
		cpp<<"}\n";
	}

	cpp<<endl;
	//close files and notfiy we are done
	fileh.close();
	filecpp.close();
	emit codeGenerated(c, true);
}


////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void CppWriter::writeOperations(UMLClassifier *c,QTextStream &h, QTextStream &cpp) {

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
		h<<"\n/** Public methods: */\npublic:\n";
		writeOperations(classname,oppub,h,cpp);
	}

	if(forceSections() || !opprot.isEmpty()) {
		h<<"\n\n/** Protected methods: */\nprotected:\n";
		writeOperations(classname,opprot,h,cpp);
	}

	if(forceSections() || !oppriv.isEmpty()) {
		h<<"\n\n/** Private methods: */\nprivate:\n";
		writeOperations(classname,oppriv,h,cpp);
	}

	UMLClass * myClass = dynamic_cast<UMLClass *>(c);
	if(myClass && hasDefaultValueAttr(myClass)) {
		QString temp = "initAttributes sets all " + classname + " attributes to its default \
		               value make sure to call this method within your class constructor";
		h<<"  /**\n";
		h<<formatDoc(temp,"    * ");
		h<<"    */\n";
		h<<"private:  void initAttributes();\n";
	}
}

void CppWriter::writeOperations(QString classname, QPtrList<UMLOperation> &opList, QTextStream &h, QTextStream &cpp) {
	UMLOperation *op;
	QPtrList<UMLAttribute> *atl;
	UMLAttribute *at;

	for(op=opList.first(); op ; op=opList.next()) {
		atl = op -> getParmList();
		//write method doc if we have doc || if at least one of the params has doc
		bool writeDoc = forceDoc() || !op->getDoc().isEmpty();
		for(at = atl->first(); at ; at = atl -> next())
			writeDoc |= !at->getDoc().isEmpty();

		if( writeDoc )  //write method documentation
		{
			h<<"    /**\n"
			<<formatDoc(op->getDoc(),"      * ");

			for(at = atl->first(); at ; at = atl -> next())  //write parameter documentation
			{
				if(forceDoc() || !at->getDoc().isEmpty()) {
					h<<"      * @param " + cleanName(at->getName())<<endl;
					h<<formatDoc(at->getDoc(),"      *        ");
				}
			}//end for : write parameter documentation
			h<<"      */" << endl;
		}//end if : write method documentation

		h<< (op->getAbstract() ? "    virtual ":"    ")
		<< (op->getStatic() ? "static " : "")
		<<  op->getReturnType() << " " << cleanName(op->getName()) << "( ";

		cpp<< op->getReturnType() << " " << classname << "::"
		<< cleanName(op->getName()) + "( ";

		int i= atl->count();
		int j=0;
		for( at = atl->first(); at ;at = atl->next(),j++) {
			h << at->getTypeName() << " " << cleanName(at->getName())
			<< (!(at->getInitialValue().isEmpty()) ?
			    (QString(" = ")+at->getInitialValue()) :
			    QString(""))
			<< ((j < i-1)?", ":"");

			cpp << at->getTypeName() << " " << cleanName(at->getName()) << QString(((j < i-1)?", ":""));
		}
		h<< QString(op->getAbstract() ? " ) = 0;" : " );" );
		cpp <<" )\n{\n\n}\n";

		h<<"\n\n";
		cpp<<"\n\n";


	}//end for
}

void CppWriter::writeAttributes(UMLClass *c, QTextStream &h) {
	QPtrList<UMLAttribute> *atl;

	QPtrList <UMLAttribute>  atpub, atprot, atpriv, atdefval;
	atpub.setAutoDelete(false);
	atprot.setAutoDelete(false);
	atpriv.setAutoDelete(false);
	atdefval.setAutoDelete(false);

	//sort attributes by scope and see if they have a default value
	atl = c->getAttList();
	UMLAttribute *at;
	for(at = atl->first(); at ; at = atl->next()) {
		if(!at->getInitialValue().isEmpty())
			atdefval.append(at);
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

	if(forceSections() || atl->count())
		h<<"\n\n/**Attributes: */\n"<<endl;

	if(forceSections() || atpub.count()) {
		h<<"public:\n";
		writeAttributes(atpub,h);
	}

	if(forceSections() || atprot.count()) {
		h<<"protected:\n";
		writeAttributes(atprot,h);
	}

	if(forceSections() || atpriv.count()) {
		h<<"private:\n";
		writeAttributes(atpriv,h);
	}
}


void CppWriter::writeAttributes(QPtrList<UMLAttribute> &atList, QTextStream &h) {
	for (UMLAttribute *at = atList.first(); at ; at = atList.next()) {
		if (forceDoc() || !at->getDoc().isEmpty()) {
			h << "    /**" << endl
			<< formatDoc(at->getDoc(), "      * ")
			<< "      */" << endl;
		}
		h << "    " << at->getTypeName() << " " << cleanName(at->getName()) << ";" << endl;

	} // end for
}



