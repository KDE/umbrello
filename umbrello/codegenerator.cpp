/***************************************************************************
                          codegenerator.cpp  -  description
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

#include <cstdlib> //to get the user name

#include <qptrlist.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qregexp.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kdialogbase.h>
#include <kapplication.h>

#include "codegenerator.h"
#include "umldoc.h"
#include "class.h"
#include "operation.h"
#include "attribute.h"
#include "umlview.h"  //for getting associations!! I'll fix this someday!!
#include "associationwidget.h" //and I'll fix this someday too
#include "dialogs/overwritedialogue.h"

#define MAXLINES 100

// constructor
CodeGenerator::CodeGenerator(QObject *parent, const char *name)
		:QObject(parent, name) {
	m_overwrite = Ask;
	m_modname = Capitalise;
	m_forceDoc = true;
	m_forceSections = false;
	m_outputDirectory = QDir::home();
	m_headingFiles = QDir::home();
	m_headingFiles.cd("headings");
	m_fileMap = new QMap<UMLClassifier*,QString>;
	m_applyToAllRemaining = true;
}

CodeGenerator::~CodeGenerator() {
	delete m_fileMap;
}

QString CodeGenerator::findFileName(UMLClassifier* concept, QString ext) {
	//if we already know to which file this class was written/should be written, just return it.
	if(m_fileMap->contains(concept))
		return ((*m_fileMap)[concept]);

	//else, determine the "natural" file name
	QString name;
	// Get the package name
	QString package = concept->getPackage();

	// Replace all white spaces with blanks
	package.simplifyWhiteSpace();

	// Replace all blanks with underscore
	package.replace(QRegExp(" "), "_");

	// if package is given add this as a directory to the file name
	if (!package.isEmpty())	{
		name = package + "." + concept->getName();
		package.replace(QRegExp("\\."), "/");
		package = "/" + package;
	} else {
		name = concept->getName();
	}

	// Convert all "." to "/" : Platform-specific path separator
	name.replace(QRegExp("\\."),"/"); // Simple hack!
 	if (ext != ".java" && ext != ".pm" && ext != ".py") {
		package = package.lower();
		name = name.lower();
	}

	// if a package name exists check the existence of the package directory
	if (!package.isEmpty())	{
		QDir packageDir(m_outputDirectory.absPath() + package);
		if (! (packageDir.exists() || packageDir.mkdir(packageDir.absPath()) ) ) {
			KMessageBox::error(0, i18n("Cannot create the package directory:\n") +
					   packageDir.absPath() + i18n("\nPlease check the access rigths"),
					   i18n("Cannot Create Directory"));
			return NULL;
		}
	}

	name.simplifyWhiteSpace();
	name.replace(QRegExp(" "),"_");

	ext.simplifyWhiteSpace();
	ext.replace(QRegExp(" "),"_");

	return overwritableName(concept, name, ext);
}

QString CodeGenerator::overwritableName(UMLClassifier* concept, QString name, QString ext) {
	//check if a file named "name" with extension "ext" already exists
	if(!m_outputDirectory.exists(name+ext)) {
		m_fileMap->insert(concept,name);
		return name; //if not, "name" is OK and we have not much to to
	}

	int suffix;
	OverwriteDialogue overwriteDialogue( name+ext, m_outputDirectory.absPath(),
					     m_applyToAllRemaining, kapp -> mainWidget() );
	switch(m_overwrite) {  //if it exists, check the OverwritePolicy we should use
		case Ok:		//ok to overwrite file
			break;
		case Ask:   		//ask if we can overwrite
			switch(overwriteDialogue.exec()) {
				case KDialogBase::Yes:  //overwrite file
					if ( overwriteDialogue.applyToAllRemaining() ) {
						m_overwrite = Ok;
					} else {
						m_applyToAllRemaining = false;
					}
					break;
			        case KDialogBase::No: //generate similar name
					suffix = 1;
					while( m_outputDirectory.exists(name + QString::number(suffix) + ext) ) {
						suffix++;
					}
					name += QString::number(suffix);
					if ( overwriteDialogue.applyToAllRemaining() ) {
						m_overwrite = Never;
					} else {
						m_applyToAllRemaining = false;
					}
					break;
				case KDialogBase::Cancel: //don't output anything
					if ( overwriteDialogue.applyToAllRemaining() ) {
						m_overwrite = Cancel;
					} else {
						m_applyToAllRemaining = false;
					}
					return NULL;
					break;
			}

			break;
	        case Never: //generate similar name
			suffix = 1;
			while( m_outputDirectory.exists(name + QString::number(suffix) + ext) ) {
				suffix++;
			}
			name += QString::number(suffix);
			break;
	        case Cancel: //don't output anything
			return NULL;
			break;
	}

	m_fileMap->insert(concept,name);
	return name;
}

bool CodeGenerator::openFile(QFile &file, QString fileName) {
	//open files for writing.
	if(fileName.isEmpty()) {
		kdWarning() << "cannot find a file name" << endl;
		return false;
	} else {
		file.setName(m_outputDirectory.absFilePath(fileName));
		if(!file.open(IO_WriteOnly)) {
			KMessageBox::sorry(0,i18n("Cannot open file %1 for writing. Please make sure the directory exists and you have permisions to write to it.").arg(file.name()),i18n("Cannot Open File"));
			return false;
		}
		return true;
	}

}


QString CodeGenerator::cleanName(QString name) {
	if(!name.contains(" ") || m_modname == No) {
		return name;
	}

	QString str = name;
	switch(m_modname) {
		case No: return name; // never reached, but gcc is creating a warning
		case Underscore:
			str.replace(QRegExp(" "),"_");
			break;
		case Capitalise:
	        default:
			str.simplifyWhiteSpace();
			int pos = -1;
			while((pos = str.find(" ",0)) != -1) {
				str.remove(pos,1);
				str.replace(pos,1,str.at(pos).upper());
			}
			break;
	}
	return str;
}

QString CodeGenerator::getHeadingFile(QString str) {
	if(!includeHeadings() || str.isEmpty())
		return QString();
	if(str.contains(" ") ||str.contains(";")) {
		kdWarning() << "File folder must not have spaces or semi colons!" << endl;
		return QString();
	}
	//if we only get the extension, then we look for the default
	// heading.[extension]. If there is no such file, we try to
	// get any file with the same extension
	QString filename;
	if(str.startsWith(".")) {
		if(QFile::exists(m_headingFiles.absFilePath("heading"+str)))
			filename = m_headingFiles.absFilePath("heading"+str);
		else {
			str.prepend('*');
			m_headingFiles.setNameFilter(str);
			//if there is more than one match we just take the first one
			filename = m_headingFiles.absFilePath(m_headingFiles.entryList().first());
			kdWarning() << "header file name set to " << filename << " because it was *" << endl;
		}
	} else {   //we got a file name (not only extension)
		filename = m_headingFiles.absFilePath(str);
	}

	QFile f(filename);
	if(!f.open(IO_ReadOnly)) {
		kdWarning() << "Error opening heading file: " << f.name() << endl;
		kdWarning() << "Headings directory was " << m_headingFiles.absPath() << endl;
		return QString();
	}
	QTextStream ts(&f);
	str = "";
	for(int l = 0; l < MAXLINES && !ts.atEnd(); l++)
		str += ts.readLine()+"\n";

	//do variable substitution
	str.replace( QRegExp("%author%"),QString(getenv("USER")));  //get the user name from some where else
	str.replace( QRegExp("%date%"), QDate::currentDate().toString());
	str.replace( QRegExp("%time%"), QTime::currentTime().toString());
	str.replace( QRegExp("%headingpath%"),filename );
	//you have to replace filepath in your code generator...

	return str;
}

QString CodeGenerator::formatDoc(const QString &text, const QString &linePrefix, int lineWidth) {
	QString output,
	comment(text);

	comment.replace(QRegExp("\n")," ");
	comment.simplifyWhiteSpace();

	int index;
	do {
		index = comment.findRev(" ",lineWidth + 1);
		output += linePrefix + comment.left(index) + "\n"; // add line
		comment.remove(0, index + 1);                      //and remove processed string, including
		// white space
	} while(index > 0 );

	return output;
}

void CodeGenerator::findObjectsRelated(UMLClassifier *c, QPtrList<UMLClassifier> &cList) {
	UMLClassifier *temp;
	UMLView *view;

	view = m_doc->getCurrentView();
	AssociationWidgetList associations;
	associations.setAutoDelete(false);
	view->getWidgetAssocs(c,associations);

	QPtrList<UMLAttribute> *atl;
	UMLAttribute *at;


	//associations
	for(AssociationWidget *a = associations.first(); a ; a = associations.next()) {
		temp = 0;
		switch(a->getAssocType()) {
			case Uml::at_Generalization:
				if(a->getWidgetAID()==c->getID())
					temp =(UMLClassifier*) m_doc->findUMLObject(a->getWidgetBID());
				if(temp  && !cList.containsRef(temp))
					cList.append(temp);
				break;
			case Uml::at_Aggregation:
			case Uml::at_Composition:
				if(a->getWidgetBID()==c->getID())
					temp = (UMLClassifier*)m_doc->findUMLObject(a->getWidgetAID());
				if(temp  && !cList.containsRef(temp))
					cList.append(temp);
				break;
			default:
				break;
		}
	}

	//operations
	QPtrList<UMLOperation> *opl = c->getOpList();
	for(UMLOperation *op = opl->first(); op ; op = opl->next()) {
		temp =0;
		//check return value
		// temp =(UMLClassifier*) m_doc->findUMLObject(Uml::ot_Concept,op->getReturnType());
		temp =(UMLClassifier*) m_doc->findUMLClassifier(op->getReturnType());
		if(temp && !cList.containsRef(temp))
			cList.append(temp);
		//check parameters
		atl = op->getParmList();
		for(at = atl->first(); at; at = atl->next()) {
			// temp = (UMLClassifier*)m_doc->findUMLObject(Uml::ot_Concept,at->getTypeName());
			temp = (UMLClassifier*)m_doc->findUMLClassifier(at->getTypeName());
			if(temp && !cList.containsRef(temp))
				cList.append(temp);
		}

	}

	//attributes
	UMLClass * myClass = dynamic_cast<UMLClass*>(c);
	if(myClass) {
		atl = myClass->getAttList();
		for(at = atl->first(); at; at = atl->next()) {
			temp=0;
			// temp =(UMLClassifier*) m_doc->findUMLObject(Uml::ot_Concept,at->getTypeName());
			temp =(UMLClassifier*) m_doc->findUMLClassifier(at->getTypeName());
			if(temp && !cList.containsRef(temp))
				cList.append(temp);
		}
	}


}


bool CodeGenerator::hasDefaultValueAttr(UMLClass *c) {
	QPtrList<UMLAttribute> *atl = c->getAttList();
	for(UMLAttribute *at = atl->first(); at; at = atl->next())
		if(!at->getInitialValue().isEmpty())
			return true;
	return false;
}

bool CodeGenerator::hasAbstractOps(UMLClassifier *c) {
	QPtrList<UMLOperation> *opl = c->getOpList();
	for(UMLOperation *op = opl->first(); op ; op = opl->next())
		if(op->getAbstract())
			return true;
	return false;
}

void CodeGenerator::generateAllClasses() {
	if(!m_doc) {
		kdWarning() << "generateAllClasses::Error: doc is NULL!" << endl;
		return;
	}
	m_fileMap->clear();
	QPtrList<UMLClassifier> cList = m_doc->getConcepts();
	generateCode(cList);
}


void CodeGenerator::generateCode( QPtrList<UMLClassifier> &list ) {
	if(!m_doc) {
		kdWarning() << "generateCode::Error: doc is NULL!" << endl;
		return;
	}

	UMLClassifier *c;
	m_fileMap->clear();
	for( c = list.first(); c ; c = list.next() )
		this->writeClass(c);    //call the right virtual function writeClass
}

void CodeGenerator::generateCode( UMLClassifier * c ) {
	if(!m_doc) {
		kdWarning() << "generateCode::Error: doc is NULL!" << endl;
		return;
	}
	//  m_fileMap->clear(); ???
	this->writeClass(c);   //call the right virtual function writeClass
}

void CodeGenerator::setDocument(UMLDoc *d) {
	m_doc = d;
}

void CodeGenerator::setOutputDirectory(QString d) {
	if(!d.isEmpty())
		m_outputDirectory.setPath(d);
}

QString CodeGenerator::outputDirectory() const {
	return m_outputDirectory.absPath();
}

void CodeGenerator::setOverwritePolicy(OverwritePolicy p) {
	m_overwrite = p;
}

CodeGenerator::OverwritePolicy CodeGenerator::overwritePolicy() const {
	return m_overwrite;
}

void CodeGenerator::setModifyNamePolicy(ModifyNamePolicy p) {
	m_modname = p;
}

CodeGenerator::ModifyNamePolicy CodeGenerator::modifyNamePolicy()const {
	return m_modname;
}

void CodeGenerator::setHeadingFileDir(const QString &path) {
	m_headingFiles.setPath(path);
}

QString CodeGenerator::headingFileDir() const {
	return m_headingFiles.absPath();
}

void CodeGenerator::setForceDoc(bool f) {
	m_forceDoc = f;
}

bool CodeGenerator::forceDoc() const {
	return m_forceDoc;
}

void CodeGenerator::setForceSections(bool f) {
	m_forceSections = f;
}

bool CodeGenerator::forceSections() const {
	return m_forceSections;
}

void CodeGenerator::setIncludeHeadings(bool i) {
	m_includeHeadings = i;
}

bool CodeGenerator::includeHeadings() const {
	return m_includeHeadings;
}
#include "codegenerator.moc"
