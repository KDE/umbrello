/***************************************************************************
                          jswriter.cpp  -  description
                             -------------------
    begin                : Sat Feb 08 2003
    copyright            : (C) 2003 by Alexander Blum
    email                : blum@kewbee.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License js published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jswriter.h"
#include "../association.h"
#include "../class.h"
#include "../operation.h"
#include "../umldoc.h"
#include "../attribute.h"

#include <kdebug.h>

#include <qregexp.h>
#include <qstring.h>

JSWriter::JSWriter( UMLDoc *parent, const char *name )
	:SimpleCodeGenerator( parent, name) {
	pListOfReservedKeywords = NULL;
}

JSWriter::~JSWriter() {}


void JSWriter::writeClass(UMLClassifier *c)
{
	if(!c)
	{
		kdDebug()<<"Cannot write class of NULL concept!" << endl;
		return;
	}

	QString classname = cleanName(c->getName());
	QString fileName = c->getName().lower();

	//find an appropriate name for our file
	fileName = findFileName(c,".js");
	if (!fileName)
	{
		emit codeGenerated(c, false);
		return;
	}

	QFile filejs;
	if(!openFile(filejs,fileName+".js"))
	{
		emit codeGenerated(c, false);
		return;
	}
	QTextStream js(&filejs);

	//////////////////////////////
	//Start generating the code!!
	/////////////////////////////


	//try to find a heading file (license, coments, etc)
	QString str;
	str = getHeadingFile(".js");
	if(!str.isEmpty())
	{
		str.replace(QRegExp("%filename%"),fileName+".js");
		str.replace(QRegExp("%filepath%"),filejs.name());
		js << str << endl;
	}


	//write includes
	UMLClassifierList includes;
	findObjectsRelated(c,includes);
	UMLClassifier *conc;
	for(conc = includes.first(); conc ;conc = includes.next())
	{
		QString headerName = findFileName(conc, ".js");
		if ( !headerName.isEmpty() )
		{
			js << "#include \"" << findFileName(conc,".js") << ".js\"" << endl;
		}
	}
	js << endl;

	//Write class Documentation if there is somthing or if force option
	if(forceDoc() || !c->getDoc().isEmpty())
	{
		js << endl << "/**" << endl;
		js << "  * class " << classname << endl;
		js << formatDoc(c->getDoc(),"  * ");
		js << "  */" << endl << endl;
	}


	//check if class is abstract and / or has abstract methods
	if(c->getAbstract() && !hasAbstractOps(c))
		js << "/******************************* Abstract Class ****************************" << endl << "  "
		<< classname << " does not have any pure virtual methods, but its author" << endl
		<< "  defined it as an abstract class, so you should not use it directly." << endl
		<< "  Inherit from it instead and create only objects from the derived classes" << endl
		<< "*****************************************************************************/" << endl << endl;

	js << classname << " = function ()" << endl;
	js << "{" << endl;
	js << m_indentation << "this._init ();" << endl;
	js << "}" << endl;
	js << endl;

	UMLClassifierList superclasses = c->getSuperClasses();
	for (UMLClassifier *obj = superclasses.first();
	     obj; obj = superclasses.next()) {
		js << classname << ".prototype = new " << cleanName(obj->getName()) << " ();" << endl;
	}

	js << endl;

	UMLClass *myClass = dynamic_cast<UMLClass*>(c);
	if(myClass) {
		UMLAttributeList atl = myClass->getFilteredAttributeList();

	 	js << "/**" << endl;
		QString temp = "_init sets all " + classname + " attributes to its default\
	 value make sure to call this method within your class constructor";
		js << formatDoc(temp, " * ");
		js << " */" << endl;
		js << classname << ".prototype._init = function ()" << endl;
		js << "{" << endl;
		for(UMLAttribute *at = atl.first(); at ; at = atl.next())
		{
			if (forceDoc() || !at->getDoc().isEmpty())
			{
				js << m_indentation << "/**" << endl
				 << formatDoc(at->getDoc(), m_indentation + " * ")
				 << m_indentation << " */" << endl;
			}
			if(!at->getInitialValue().isEmpty())
			{
				js << m_indentation << "this.m_" << cleanName(at->getName()) << " = " << at->getInitialValue() << ";" << endl;
			}
			else
			{
	 			js << m_indentation << "this.m_" << cleanName(at->getName()) << " = \"\";" << endl;
			}
		}
	}

	//associations
	UMLAssociationList aggregations = c->getAggregations();
	if (forceSections() || !aggregations.isEmpty ())
	{
		js << m_newLineEndingChars << m_indentation << "/**Aggregations: */" << m_newLineEndingChars;
		for (UMLAssociation* a = aggregations.first(); a; a = aggregations.next())
		{
			QString nm(cleanName(a->getObject(Uml::A)->getName()));
			if (a->getMulti(Uml::A).isEmpty())
				js << m_indentation << "this.m_" << nm << " = new " << nm << " ();" << m_newLineEndingChars;
			else
				js << m_indentation << "this.m_" << nm.lower() << " = new Array ();" << m_newLineEndingChars;
		}
	}
	UMLAssociationList compositions = c->getCompositions();
	if( forceSections() || !compositions.isEmpty())
	{
		js << m_newLineEndingChars << m_indentation << "/**Compositions: */" << m_newLineEndingChars;
		for (UMLAssociation *a = compositions.first(); a; a = compositions.next())
		{
			QString nm(cleanName(a->getObject(Uml::A)->getName()));
			if(a->getMulti(Uml::A).isEmpty())
				js << m_indentation << "this.m_" << nm << " = new "<< nm << " ();" << m_newLineEndingChars;
			else
				js << m_indentation << "this.m_" << nm.lower() << " = new Array ();" << m_newLineEndingChars;
		}
	}
	js << endl;
	js << "}" << endl;
	js << endl;

	//operations
	UMLOperationList ops(c->getOpList());
	writeOperations(classname, &ops, js);

	js << endl;

	//finish file

	//close files and notfiy we are done
	filejs.close();
	emit codeGenerated(c, true);
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void JSWriter::writeOperations(QString classname, UMLOperationList *opList, QTextStream &js)
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
			js << "/**" << m_newLineEndingChars << formatDoc(op->getDoc()," * ");

			for(at = atl->first(); at ; at = atl -> next())  //write parameter documentation
			{
				if(forceDoc() || !at->getDoc().isEmpty())
				{
					js << " * @param " + cleanName(at->getName())<<endl;
					js << formatDoc(at->getDoc(),"    *      ");
				}
			}//end for : write parameter documentation
			js << " */" << endl;
		}//end if : write method documentation

		js << classname << ".prototype." << cleanName(op->getName()) << " function " << "(";

		int i= atl->count();
		int j=0;
		for (at = atl->first(); at ;at = atl->next(),j++)
		{
			js << cleanName(at->getName())
				 << (!(at->getInitialValue().isEmpty()) ? (QString(" = ")+at->getInitialValue()) : QString(""))
				 << ((j < i-1)?", ":"");
		}
		js << ")" << m_newLineEndingChars << "{" << m_newLineEndingChars <<
		m_indentation << m_newLineEndingChars << "}" << m_newLineEndingChars;
		js << m_newLineEndingChars << endl;
	}//end for
}

/**
 * returns "JavaScript"
 */
QString JSWriter::getLanguage() {
        return "JavaScript";
}

/**
 * checks whether type is "JSWriter"
 *
 * @param type
 */
bool JSWriter::isType (QString & type)
{
   if(type == "JSWriter")
        return true;
   return false;
}

/**
 * List of reserved keywords for this code generator.
 *
 * Just add new keywords, then mark all lines and
 * pipe it through the external 'sort' program.
 */
static const char *ReservedKeywords[] = {
  "break",
  "case",
  "const",
  "continue",
  "default",
  "else",
  "false",
  "for",
  "function",
  "if",
  "in",
  "new",
  "return",
  "switch",
  "this",
  "true",
  "var",
  "while",
  "with",
  NULL
};

/**
 * get list of reserved keywords
 */
const QPtrList<const char *> * JSWriter::getReservedKeywords() {
  if (pListOfReservedKeywords == NULL)
  {
    pListOfReservedKeywords = convertListOfReservedKeywords(ReservedKeywords);
  }

  return pListOfReservedKeywords;
}

#include "jswriter.moc"
