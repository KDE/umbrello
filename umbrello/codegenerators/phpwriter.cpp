/***************************************************************************
			  phpwriter.cpp  -  description
			     -------------------
    begin		: Thu Oct 17 2002
    copyright	    : (C) 2002 by Heiko Nardmann
    email		: h.nardmann@secunet.de
 ***************************************************************************/

/***************************************************************************
 *									 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.				   *
 *									 *
 ***************************************************************************/

#include "phpwriter.h"

#include <kdebug.h>
#include <qregexp.h>
#include <qstring.h>

#include "../umldoc.h"
#include "../class.h"
#include "../association.h"
#include "../attribute.h"
#include "../operation.h"
#include "../umlnamespace.h"

PhpWriter::PhpWriter( UMLDoc *parent, const char *name )
	:SimpleCodeGenerator( parent, name) {}

PhpWriter::~PhpWriter() {}


void PhpWriter::writeClass(UMLClassifier *c) {
	if(!c) {
		kdDebug()<<"Cannot write class of NULL concept!\n";
		return;
	}

	QString classname = cleanName(c->getName());
	QString fileName = c->getName().lower();

	//find an appropriate name for our file
	fileName = findFileName(c,".php");
	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}

	QFile filephp;
	if(!openFile(filephp,fileName+".php")) {
		emit codeGenerated(c, false);
		return;
	}
	QTextStream php(&filephp);

	//////////////////////////////
	//Start generating the code!!
	/////////////////////////////


	//try to find a heading file (license, coments, etc)
	QString str;
	str = getHeadingFile(".php");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),fileName+".php");
		str.replace(QRegExp("%filepath%"),filephp.name());
		php<<str<<endl;
	}


	//write includes
	UMLClassifierList includes;
	findObjectsRelated(c,includes);
	UMLClassifier *conc;
	for(conc = includes.first(); conc ;conc = includes.next()) {
		QString headerName = findFileName(conc, ".php");
		if (headerName.isEmpty()) {
			php << "include '" << findFileName(conc,".php") << ".php\';" << endl;
		}
	}
	php << endl;

	//Write class Documentation if there is somthing or if force option
	if(forceDoc() || !c->getDoc().isEmpty()) {
		php << endl << "/**" << endl;
		php << " * class " << classname << endl;
		php << formatDoc(c->getDoc()," * ");
		php << " */" << endl ;
	}

	UMLClassifierList superclasses = c->getSuperClasses();
	UMLAssociationList aggregations = c->getAggregations();
	UMLAssociationList compositions = c->getCompositions();
	UMLAssociation *a;

	//check if class is abstract and / or has abstract methods
//FG	if(c->getAbstract() && !hasAbstractOps(c))
	if(c->getAbstract())
		php << "/******************************* Abstract Class ****************************" << endl << "  "
		<< classname << " does not have any pure virtual methods, but its author" << endl
		<< "  defined it as an abstract class, so you should not use it directly." << endl
		<< "  Inherit from it instead and create only objects from the derived classes" << endl
		<< "*****************************************************************************/" << endl << endl;

	php << "class " << classname << (superclasses.count() > 0 ? " extends ":"");
	for (UMLClassifier *obj = superclasses.first();
		obj; obj = superclasses.next()) {
 		php<<cleanName(obj->getName());
	}
	php<<"\n{\n";

	//associations
	if( forceSections() || !aggregations.isEmpty()) {
		php<<"\n\t/** Aggregations: */\n";
		for (a = aggregations.first(); a; a = aggregations.next()) {
			php<<"\n";
			//maybe we should parse the string here and take multiplicity into account to decide
			//which container to use.
			UMLObject *o = m_doc->findUMLObject(a->getRoleId(A));
			QString typeName = cleanName(o->getName());
			if (a->getMulti(A).isEmpty())  {
				php << "\tvar $m_" << ";" << endl;
			} else {
				php << "\tvar $m_" << "Vector = array();" << endl;
			}
		}//end for
	}

	if( forceSections() || !compositions.isEmpty()) {
		php<<"\n\t/** Compositions: */\n";
		for (a = compositions.first(); a ; a = compositions.next()) {
			// see comment on Aggregation about multiplicity...
			UMLObject *o = m_doc->findUMLObject(a->getRoleId(A));
			QString typeName = cleanName(o->getName());
			if (a->getMulti(A).isEmpty())  {
				php << "\tvar $m_" << ";" << endl;
			} else {
				php << "\tvar $m_" << "Vector = array();" << endl;
			}
		}
	}

	UMLClass * myClass = dynamic_cast<UMLClass *>(c);

	//attributes
	if(myClass)
		writeAttributes(myClass, php);

	//operations
	writeOperations(c,php);

	if(myClass && hasDefaultValueAttr(myClass)) {
		UMLAttributeList *atl = myClass->getFilteredAttributeList();
		php << endl;

		php << "\t/**" << endl;
		QString temp = "initAttributes sets all " + classname + " attributes to its default \
			       value make sure to call this method within your class constructor";
		php << formatDoc(temp,"\t * ");
		php << "\t */" << endl;
		php << "\tfunction "<<"initAttributes( )" << endl;
		php << "\t{" << endl;
		for(UMLAttribute* at = atl->first(); at; at = atl->next())  {
			if(!at->getInitialValue().isEmpty())  {
				php << "\t\t$this->" << cleanName(at->getName()) << " = " <<
					at->getInitialValue() << ";" << endl;
			}
		}
		php << "\t}" << endl;
	}

	php << endl;

	//finish file
	php << "\n} // end of " << classname << endl;
	php << "?>" << endl;

	//close files and notfiy we are done
	filephp.close();
	emit codeGenerated(c, true);
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void PhpWriter::writeOperations(UMLClassifier *c, QTextStream &php) {

	//Lists to store operations  sorted by scope
	UMLOperationList oppub,opprot,oppriv;

	oppub.setAutoDelete(false);
	opprot.setAutoDelete(false);
	oppriv.setAutoDelete(false);

	//sort operations by scope first and see if there are abstract methods
	UMLOperationList opl(c->getFilteredOperationsList());
	for(UMLOperation *op = opl.first(); op ; op = opl.next()) {
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
		php << endl;
		writeOperations(classname,oppub,php);
	}

	if(forceSections() || !opprot.isEmpty()) {
		php << endl;
		writeOperations(classname,opprot,php);
	}

	if(forceSections() || !oppriv.isEmpty()) {
		php << endl;
		writeOperations(classname,oppriv,php);
	}
}

void PhpWriter::writeOperations(QString /* classname */, UMLOperationList &opList, QTextStream &php) {
	UMLOperation *op;
	UMLAttributeList *atl;
	UMLAttribute *at;

	for(op=opList.first(); op ; op=opList.next()) {
		atl = op -> getParmList();
		//write method doc if we have doc || if at least one of the params has doc
		bool writeDoc = forceDoc() || !op->getDoc().isEmpty();
		for(at = atl->first(); at ; at = atl -> next())
			writeDoc |= !at->getDoc().isEmpty();

		if( writeDoc )  //write method documentation
		{
			php <<"\t/**\n"
			<<formatDoc(op->getDoc(),"\t * ");
			php << "\t *\n";

			for(at = atl->first(); at ; at = atl -> next())  //write parameter documentation
			{
				if(forceDoc() || !at->getDoc().isEmpty()) {
					php <<"\t * @param " + at->getTypeName() + " " + cleanName(at->getName());
					php << " " + formatDoc(at->getDoc(),"");
				}
			}//end for : write parameter documentation
			php << "\t * @return " << op->getReturnType() << endl;
			if (op->getAbstract()) php << "\t * @abstract\n";
			if (op->getStatic()) php << "\t * @static\n";
			switch(op->getScope()) {
				case Uml::Public:
					php << "\t * @access public\n";
					break;
				case Uml::Protected:
					php << "\t * @access protected\n";
					break;
				case Uml::Private:
					php << "\t * @access private\n";
					break;
			}
			php <<"\t */" << endl;
		}//end if : write method documentation

		php <<  "\tfunction " << cleanName(op->getName()) << "(";

		int i= atl->count();
		int j=0;
		for( at = atl->first(); at ;at = atl->next(),j++) {
			php << " $" << cleanName(at->getName())
			<< (!(at->getInitialValue().isEmpty()) ?
			    (QString(" = ")+at->getInitialValue()) :
			    QString(""))
			<< ((j < i-1)?", ":"");
		}
		php <<" )\n\t{\n\t\t\n\t} // end of member function " + cleanName(op->getName()) + "\n";
		php << endl;
	}//end for
}

void PhpWriter::writeAttributes(UMLClass *c, QTextStream &php) {
	UMLAttributeList *atl;

	UMLAttributeList  atpub, atprot, atpriv, atdefval;
	atpub.setAutoDelete(false);
	atprot.setAutoDelete(false);
	atpriv.setAutoDelete(false);
	atdefval.setAutoDelete(false);

	//sort attributes by scope and see if they have a default value
	atl = c->getFilteredAttributeList();
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
		php<<"\n\t/*** Attributes: ***/\n"<<endl;

	if(forceSections() || atpub.count()) {
		writeAttributes(atpub,php);
	}

	if(forceSections() || atprot.count()) {
		writeAttributes(atprot,php);
	}

	if(forceSections() || atpriv.count()) {
		writeAttributes(atpriv,php);
	}
}


void PhpWriter::writeAttributes(UMLAttributeList &atList, QTextStream &php) {
	for (UMLAttribute *at = atList.first(); at ; at = atList.next()) {
		if (forceDoc() || !at->getDoc().isEmpty()) {
			php << "\t/**" << endl
			<< formatDoc(at->getDoc(), "\t * ");
			switch(at->getScope()) {
				case Uml::Public:
					php << "\t * @access public" << endl;
					break;
				case Uml::Protected:
					php << "\t * @access protected" << endl;
					break;
				case Uml::Private:
					php << "\t * @access private" << endl;
					break;
			}

			php << "\t */" << endl;
		}
		php << "\tvar " << "$" << cleanName(at->getName()) << ";" << endl;

	} // end for
	return;
}

QString PhpWriter::getLanguage() {
	return "PHP";
}

bool PhpWriter::isType (QString & type)
{
   if(type == "PhpWriter")
	return true;
   return false;
}



#include "phpwriter.moc"
