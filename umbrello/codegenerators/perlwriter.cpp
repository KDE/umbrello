/***************************************************************************
                          perlwriter.cpp  -  description
                             -------------------
    begin                : Wed Jan 22 2003
    copyright            : (C) 2003 by David Hugh-Jones
    email                : hughjonesd@yahoo.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "perlwriter.h"
#include "../class.h"
#include "../operation.h"
#include "../umldoc.h"
#include "../association.h"
#include "../attribute.h"
#include <kdebug.h>
#include <qregexp.h>
#include <qstring.h>
#include <qdir.h>

PerlWriter::PerlWriter( QObject *parent, const char *name )
	:CodeGenerator( parent, name) {}

PerlWriter::~PerlWriter() {}


void PerlWriter::writeClass(UMLClassifier *c) {
	if(!c) {
		kdDebug()<<"Cannot write class of NULL concept!\n";
		return;
	}

	QString classname = cleanName(c->getName());// this is fine: cleanName is "::-clean"
	QString fileName;

    	fileName = findFileName(c, ".pm"); 	//lower-cases my nice class names. That is bad.
						// correct solution: refactor,
						// split massive findFileName up, reimplement
						// parts here
						// actual solution: shameful ".pm" hack in codegenerator

	QString curDir = outputDirectory();
	if (fileName.contains("::")) {
		// create new directories for each level
		QString newDir;
		newDir = curDir;
		QString fragment = fileName;
		QDir* existing = new QDir (curDir);
		QRegExp regEx("(.*)(::)");
		regEx.setMinimal(true);
		while (regEx.search(fragment) > -1) {
			newDir = regEx.cap(1);
			fragment.remove(0, (regEx.pos(2) + 2)); // get round strange minimal matching bug
			existing->setPath(curDir + "/" + newDir);
			if (! existing->exists()) {
				existing->setPath(curDir);
				if (! existing->mkdir(newDir)) {
					emit codeGenerated(c, false);
					return;
				}
			}
			curDir += "/" + newDir;
		}
		fileName = fragment;
	}
	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}
	QString oldDir = outputDirectory();
	setOutputDirectory(curDir);
	QFile fileperl;
	if(!openFile(fileperl,fileName+".pm")) {
		emit codeGenerated(c, false);
		return;
	}
	QTextStream perl(&fileperl);
	setOutputDirectory(oldDir);
	//////////////////////////////
	//Start generating the code!!
	/////////////////////////////

	perl << "\n\npackage " << classname << ";\n\n";
	//try to find a heading file (license, coments, etc)
	QString str;
	str = getHeadingFile(".pm");   // what this mean?
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),fileName+".pm");
		str.replace(QRegExp("%filepath%"),fileperl.name());
		perl<<str<<endl;
	}

	//write includes
	QPtrList<UMLClassifier> includes;//ca existe en perl??
	findObjectsRelated(c,includes);
	UMLClassifier *conc;
	for(conc = includes.first(); conc ;conc = includes.next()) {
			perl << "use " << cleanName(conc->getName()) << ";" << endl; // seems OK
	}
	perl << endl;

	QPtrList<UMLAssociation> generalizations = c->getGeneralizations();
	QPtrList<UMLAssociation> aggregations = c->getAggregations();
	QPtrList<UMLAssociation> compositions = c->getCompositions();
	UMLAssociation *a;

	if (generalizations.count()) {
		int i;
		perl << "use base qw( ";
		for (a = generalizations.first(), i = generalizations.count();
		     a && i;
		     a = generalizations.next(), i--) {
			UMLObject* obj = m_doc->findUMLObject(a->getRoleBId());
			perl << cleanName(obj->getName()) << " ";
		}
		perl << ");\n";
	}

	//Write class Documentation
	if(forceDoc() || !c->getDoc().isEmpty()) {
		perl << endl << "=head1";
		perl << " " << classname.upper() << endl;
		perl << formatDoc(c->getDoc(),"");
		perl << endl << "=cut" << endl << endl;
	}

	//check if class is abstract and / or has abstract methods
	if(c->getAbstract())
        perl << "=head1 ABSTRACT CLASS\n\n=cut" << endl;

	//attributes
	UMLClass *myClass = dynamic_cast<UMLClass*>(c);
	if (myClass)
		writeAttributes(myClass, perl);      // keep for documentation's sake

	//operations
	writeOperations(c,perl);

	perl << endl;

	//finish file
	perl << "\n\n1;" << endl;

	//close files and notify we are done
	fileperl.close();
	emit codeGenerated(c, true);
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void PerlWriter::writeOperations(UMLClassifier *c, QTextStream &perl) {

	//Lists to store operations  sorted by scope
	QPtrList<UMLOperation> *opl;
	QPtrList<UMLOperation> oppub,opprot,oppriv;

	oppub.setAutoDelete(false);
	opprot.setAutoDelete(false);
	oppriv.setAutoDelete(false);

	//sort operations by scope first and see if there are abstract methods
	//keep this for documentation only!
	opl = c->getFilteredOperationsList();
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
		perl << endl << "=head1 PUBLIC METHODS"
			<< endl << endl << "=over 4" << endl;
		writeOperations(classname,oppub,perl);
		perl << endl << "=back" << endl;
	}

	if(forceSections() || !opprot.isEmpty()) {
		perl << endl << "=head1 METHODS FOR SUBCLASSING"
			<< endl << endl << "=over 4" << endl;
		writeOperations(classname,opprot,perl);
		perl << endl << "=back" << endl;
	}

	if(forceSections() || !oppriv.isEmpty()) {
		perl << endl << "=head1 PRIVATE METHODS"
			<< endl << endl << "=over 4"  << endl ;
		writeOperations(classname,oppriv,perl);

	}

    // moved here for perl
	UMLClass *myClass = dynamic_cast<UMLClass*>(c);

	if(myClass && hasDefaultValueAttr(myClass)) {
		QPtrList<UMLAttribute> *atl = myClass->getFilteredAttributeList();

		perl << endl;
		perl << endl << "=item _init\n\n" << endl;
		perl << "_init sets all " + classname + " attributes to their default \
		               values unless already set\n=cut\n\n";
		perl << "sub _init {" << endl << "\tmy $self = shift;" << endl<<endl;

		for(UMLAttribute *at = atl->first(); at ; at = atl->next()) {
			if(!at->getInitialValue().isEmpty())
				perl << "\tdefined $self->{" << cleanName(at->getName())<<"}"
                		<< " or $self->{" << cleanName(at->getName()) << "} = "
                		<< at->getInitialValue() << ";" << endl;
            	}

	    perl << " }" << endl;
	}

	perl << "\n=back\n\n";
}

void PerlWriter::writeOperations(QString /* classname */, QPtrList<UMLOperation> &opList, QTextStream &perl) {
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
			perl << "=item " << cleanName(op->getName()) << endl << endl;

            perl << "Parameters:\n\n=over 8" << endl << endl;
			for(at = atl->first(); at ; at = atl -> next())  //write parameter documentation
			{
				if(forceDoc() || !at->getDoc().isEmpty()) {
                    perl << endl << "=item ";

					perl << cleanName(at->getName()) + " : ";
					perl << at->getDoc();
                    perl << endl;
				}
			}//end for : write parameter documentation
            perl << "=back" << endl << endl;
            perl << "Description:" << endl << endl;
	        perl << formatDoc(op->getDoc(),"");
			perl <<"\n=cut" << endl;
		}//end if : write method documentation

		perl <<  "sub " << cleanName(op->getName()) << " {" << endl << endl << "}\n";
		perl << "\n" << endl;
	}//end for
}


void PerlWriter::writeAttributes(UMLClass *c, QTextStream &perl) {
	QPtrList<UMLAttribute> *atl;

	QPtrList <UMLAttribute>  atpub, atprot, atpriv, atdefval;
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


	if(forceSections() || atpub.count()) {
		writeAttributes(atpub,perl);
	}
   /* not needed as writeAttributes only writes documentation
	if(forceSections() || atprot.count()) {
		writeAttributes(atprot,perl);
	}

	if(forceSections() || atpriv.count()) {
		writeAttributes(atpriv,perl);
	}
*/
}


void PerlWriter::writeAttributes(QPtrList<UMLAttribute> &atList, QTextStream &perl) {
    perl << "\n=head1 PUBLIC ATTRIBUTES\n\n=over 4\n";

	for (UMLAttribute *at = atList.first(); at ; at = atList.next()) {
		if (forceDoc() || !at->getDoc().isEmpty()) {
            perl  << endl << "=item " << cleanName(at->getName()) << endl << endl
			     << at->getDoc() << endl;
		}
	} // end for
    perl << "\n=cut\n";
	return;
}


