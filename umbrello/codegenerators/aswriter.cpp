/***************************************************************************
                          aswriter.cpp  -  description
                             -------------------
    begin                : Sat Feb 08 2003
    copyright            : (C) 2003 by Alexander Blum
    email                : blum@kewbee.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "aswriter.h"
#include "../association.h"
#include "../class.h"
#include "../operation.h"
#include "../umldoc.h"
#include "../attribute.h"

#include <kdebug.h>
#include <qregexp.h>
#include <qstring.h>

ASWriter::ASWriter( UMLDoc *parent, const char *name )
	:SimpleCodeGenerator( parent, name) {}

ASWriter::~ASWriter() {}


void ASWriter::writeClass(UMLClassifier *c)
{
	if(!c)
	{
		kdDebug()<<"Cannot write class of NULL concept!\n";
		return;
	}

	QString classname = cleanName(c->getName());
	QString fileName = c->getName().lower();

	//find an appropriate name for our file
	fileName = findFileName(c,".as");
	if (!fileName)
	{
		emit codeGenerated(c, false);
		return;
	}

	QFile fileas;
	if(!openFile(fileas,fileName+".as"))
	{
		emit codeGenerated(c, false);
		return;
	}
	QTextStream as(&fileas);

	//////////////////////////////
	//Start generating the code!!
	/////////////////////////////


	//try to find a heading file (license, coments, etc)
	QString str;
	str = getHeadingFile(".as");
	if(!str.isEmpty())
	{
		str.replace(QRegExp("%filename%"),fileName+".as");
		str.replace(QRegExp("%filepath%"),fileas.name());
		as << str << endl;
	}


	//write includes
	UMLClassifierList includes;
	findObjectsRelated(c,includes);
	UMLClassifier *conc;
	for(conc = includes.first(); conc ;conc = includes.next())
	{
		QString headerName = findFileName(conc, ".as");
		if ( !headerName.isEmpty() )
		{
			as << "#include \"" << findFileName(conc,".as") << ".as\"" << endl;
		}
	}
	as << endl;

	//Write class Documentation if there is somthing or if force option
	if(forceDoc() || !c->getDoc().isEmpty())
	{
		as << endl << "/**" << endl;
		as << "  * class " << classname << endl;
		as << formatDoc(c->getDoc(),"  * ");
		as << "  */" << endl << endl;
	}

	UMLClassifierList superclasses = c->getSuperClasses();
	UMLAssociationList aggregations = c->getAggregations();
	UMLAssociationList compositions = c->getCompositions();

	//check if class is abstract and / or has abstract methods
	if(c->getAbstract() && !hasAbstractOps(c))
		as << "/******************************* Abstract Class ****************************" << endl << "  "
		<< classname << " does not have any pure virtual methods, but its author" << endl
		<< "  defined it as an abstract class, so you should not use it directly." << endl
		<< "  Inherit from it instead and create only objects from the derived classes" << endl
		<< "*****************************************************************************/" << endl << endl;

	as << classname << " = function ()" << endl;
	as << "{" << endl;
	as << "\tthis._init ();" << endl;
	as << "}" << endl;
	as << endl;

	for(UMLClassifier *obj = superclasses.first();
	    obj; obj = superclasses.next()) {
		as << classname << ".prototype = new " << cleanName(obj->getName()) << " ();" << endl;
	}

	as << endl;

	UMLClass *myClass = dynamic_cast<UMLClass*>(c);
	if(myClass) {

		UMLAttributeList *atl = myClass->getFilteredAttributeList();

	 	as << "/**" << endl;
		QString temp = "_init sets all " + classname + " attributes to its default\
	 value make sure to call this method within your class constructor";
		as << formatDoc(temp, " * ");
		as << " */" << endl;
		as << classname << ".prototype._init = function ()" << endl;
		as << "{" << endl;
		for(UMLAttribute *at = atl->first(); at ; at = atl->next())
		{
			if (forceDoc() || !at->getDoc().isEmpty())
			{
				as << "\t/**" << endl
				 << formatDoc(at->getDoc(), "\t * ")
				 << "\t */" << endl;
			}
			if(!at->getInitialValue().isEmpty())
			{
				as << "\tthis.m_" << cleanName(at->getName()) << " = " << at->getInitialValue() << ";" << endl;
			}
			else
			{
	 			as << "\tthis.m_" << cleanName(at->getName()) << " = \"\";" << endl;
			}
		}
	}

	//associations
	if (forceSections() || !aggregations.isEmpty ())
	{
		as << "\n\t/**Aggregations: */\n";
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next())
		{
			QString nm(cleanName(a->getObject(A)->getName()));
			if (a->getMulti(A).isEmpty())
				as << "\tthis.m_" << nm << " = new " << nm << " ();\n";
			else
				as << "\tthis.m_" << nm.lower() << " = new Array ();\n";
		}
	}
	if( forceSections() || !compositions.isEmpty())
	{
		as << "\n\t/**Compositions: */\n";
		for(UMLAssociation *a = compositions.first(); a; a = compositions.next())
		{
			QString nm(cleanName(a->getObject(A)->getName()));
			if(a->getMulti(A).isEmpty())
				as << "\tthis.m_" << nm << " = new " << nm << " ();\n";
			else
				as << "\tthis.m_" << nm.lower() << " = new Array ();\n";
		}
	}
	as << endl;

	as << "\t/**Protected: */\n";
	if(myClass) {
		UMLAttributeList *atl = myClass->getFilteredAttributeList();
		for (UMLAttribute *at = atl->first(); at ; at = atl->next())
		{
			if (at->getScope() == Uml::Protected)
			{
				as << "\tASSetPropFlags (this, \"" << cleanName(at->getName()) << "\", 1);" << endl;
			}
		}
	}

 	UMLOperationList opList(c->getFilteredOperationsList());
	for (UMLOperation *op = opList.first(); op; op = opList.next())
	{
		if (op->getScope() == Uml::Protected)
		{
			as << "\tASSetPropFlags (this, \"" << cleanName(op->getName()) << "\", 1);" << endl;
		}
	}
	as << endl;
	as << "\t/**Private: */\n";
	if(myClass) {
		UMLAttributeList *atl = myClass->getFilteredAttributeList();
		for (UMLAttribute *at = atl->first(); at; at = atl->next())
		{
			if (at->getScope() == Uml::Private)
			{
				as << "\tASSetPropFlags (this, \"" << cleanName(at->getName()) << "\", 7);" << endl;
			}
		}
	}

	for (UMLOperation *op = opList.first(); op; op = opList.next())
	{
		if (op->getScope() == Uml::Protected)
		{
			as << "\tASSetPropFlags (this, \"" << cleanName(op->getName()) << "\", 7);" << endl;
		}
	}
	as << "}" << endl;

	as << endl;

	//operations
	UMLOperationList ops(c->getFilteredOperationsList());
	writeOperations(classname, &ops, as);

	as << endl;

	//finish file

	//close files and notfiy we are done
	fileas.close();
	emit codeGenerated(c, true);
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void ASWriter::writeOperations(QString classname, UMLOperationList *opList, QTextStream &as)
{
	UMLOperation *op;
	UMLAttributeList *atl;
	UMLAttribute *at;

	for(op = opList->first(); op; op = opList->next())
	{
		atl = op -> getParmList();
		//write method doc if we have doc || if at least one of the params has doc
		bool writeDoc = forceDoc() || !op->getDoc().isEmpty();
		for(at = atl->first(); at ; at = atl -> next())
			writeDoc |= !at->getDoc().isEmpty();

		if( writeDoc )  //write method documentation
		{
			as << "/**\n" << formatDoc(op->getDoc()," * ");

			for(at = atl->first(); at ; at = atl -> next())  //write parameter documentation
			{
				if(forceDoc() || !at->getDoc().isEmpty())
				{
					as << " * @param " + cleanName(at->getName())<<endl;
					as << formatDoc(at->getDoc(),"    *      ");
				}
			}//end for : write parameter documentation
			as << " */" << endl;
		}//end if : write method documentation

		as << classname << ".prototype." << cleanName(op->getName()) << " function " << "(";

		int i= atl->count();
		int j=0;
		for (at = atl->first(); at ;at = atl->next(),j++)
		{
			as << cleanName(at->getName())
				 << (!(at->getInitialValue().isEmpty()) ? (QString(" = ")+at->getInitialValue()) : QString(""))
				 << ((j < i-1)?", ":"");
		}
		as << ")\n{\n\t\n}\n";
		as << "\n" << endl;
	}//end for
}

QString ASWriter::getLanguage() {
        return "ActionScript";
}

bool ASWriter::isType (QString & type)
{
   if(type == "ASWriter")
        return true;
   return false;
}


#include "aswriter.moc"
