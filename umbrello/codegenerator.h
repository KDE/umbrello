/***************************************************************************
                          codegenerator.h  -  description
                             -------------------
    begin                : Mon Jun 17 2002
    copyright            : (C) 2002 by Luis De la Parra Blum
    email                : luis@delaparra.org
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <klibloader.h>
#include <qdir.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qobject.h>

class UMLDoc;
class UMLClassifier;
class UMLClass;

/**
 * CodeGenerator is the base class for all CodeGenerators. It
 * provides the interface through which all Generators are invoked and
 * the all the basic functionality. The only thing it doesn't do is to
 * generate code =)
 *
 * If you want to implement a CodeGenerator for some language follow
 * these steps:
 *
 * Create a class which inherits CodeGenerator. This class can have
 * any name, I use names like CppWriter for the Cpp Generator,
 * JavaWriter for the Java Generator and so on, but you can use what
 * you want.
 *
 * The only method which you should implement in your class is:
 *
 *    void writeClass(UMLClassifier *)
 *
 * in which you get a pointer to the UMLClassifier (class) for which you
 * have to write the code, and you write a file with the code.
 *
 * The code you generate should be output to "outputDirectory" and you
 * should respect the OverwritePolicy specified. You should call
 * findFileName(..) to get an appropriate file name, and then you can
 * call openFile if you want, but if you want to do it yourself you
 * must check the overwrite policy :
 *
 * OverwritePolicy  can have the following values
 *  - Ok: if there is a file named the same as what you want to name your output file,
 *        you can overwrite the old file.
 *  - Ask:if there is a file named the same as what you want to name your output file,
 *        you should ask the User what to do, and give him the option to overwrite the file
 *        write the code to a different file, or to abort the generation of this class.
 *  - Never: you cannot overwrite any files. Generates a new file name like "fileName1.h", "fileName2.h"
 *        until you find an appropriate name.
 *  - Cancel: Do not output anything.  This is only set if the user chooses Apply to All Remaining Files
 * 	      and clicks on Do not Output  in the Ask dialogue
 *
 * Finally put your generator in a library which can be dlopened
 * together with a factory class (see below) and you are ready to go.
 */
class CodeGenerator : public QObject {
	Q_OBJECT

public:
	enum OverwritePolicy {Ok=0, Ask, Never, Cancel};
	enum ModifyNamePolicy {No=0, Underscore, Capitalise};

	CodeGenerator( QObject *parent = 0, const char *name = 0 );
	virtual ~CodeGenerator();

	void generateAllClasses();

	/**
	 *   Generate code for the list of classes given.
	 */
	void generateCode( QPtrList<UMLClassifier> &list );

	/**
	 *   Generate code for class c
	 */
	void generateCode( UMLClassifier *c);

	void setDocument(UMLDoc *d);

	void setOutputDirectory(QString d);
	QString outputDirectory() const;

	void setOverwritePolicy(OverwritePolicy p);
	OverwritePolicy overwritePolicy() const;

	void setModifyNamePolicy(ModifyNamePolicy p);
	ModifyNamePolicy modifyNamePolicy()const;

	void setIncludeHeadings(bool i);
	bool includeHeadings() const;

	void setHeadingFileDir(const QString &);
	QString headingFileDir() const;

	void setForceDoc(bool f);
	bool forceDoc() const;

	void setForceSections(bool f);
	bool forceSections() const;

protected:

	/**
	 * To be implemented by the Language Writers
	 */
	virtual void writeClass(UMLClassifier *) {};

	/**
	 * Finds an appropriate file name for class c, taking into account the Overwrite
	 * Policy and asking the user what to do if need be. (if policy == Ask)
	 *
	 * @param concept the class for which an output file name is desired.
	 * @param ext the extension (or suffix) used for output files
	 * @return the file name that should be used. (without extension) or 
	 * 	NULL if none to be used
	 */
	QString findFileName(UMLClassifier* concept, QString ext);

	/**
	 * Returns a name that can be written to in the output directory,
	 * respecting the overwrite policy.
	 * If a file of the given name and extension does not exist,
	 * then just returns the name.
	 * If a file of the given name and extension does exist,
	 * then opens an overwrite dialog. In this case the name returned
	 * may be a modification of the input name.
	 * This method is invoked by findFileName().
	 *
	 * @param concept the class for which an output file name is desired.
	 * @param name the proposed output file name
	 * @param ext the extension (or suffix) used for output files
	 * @return the real file name that should be used. (without extension) or 
	 * 	NULL if none to be used
	 */
	QString overwritableName(UMLClassifier* concept, QString name, QString ext);

	/** Opens a file named "name" for writing in the outputDirectory.
	 * If something goes wrong, it informs the user
	 * if this function returns true, you know you can write to the file
	 */
	bool openFile(QFile &file, QString name);

	/**
	 *  Gets the heading file (as a string) to be inserted at the
	 *  begining of the generated file. you give the file type as
	 *  parameter and get the string. if fileName starts with a
	 *  period (.) then fileName is the extension (.cpp, .h,
	 *  .java) if fileName starts with another character you are
	 *  requesting a specific file (mylicensefile.txt).  The files
	 *  can have parameters which are denoted by %parameter%.
	 *
	 *  current parameters are
	 *  %author%
	 *  %date%
	 *  %time%
	 *  %filepath%
	 */
	QString getHeadingFile(QString file);
	
	/**
	 * Replaces spaces with underscores and capitalises as defined in m_modname
	 */
	QString cleanName(QString);

	/** Format documentation for output in source files
	 *
	 * @param text the documentation which has to be formatted
	 * @param linePrefix the prefix which has to be added in the beginnig of each line
	 * @param lineWidth the line width used for word-wrapping the documentation
	 *
	 * @return the formatted documentation text
	 */
	QString formatDoc(const QString &text, const QString &linePrefix="* ", int lineWidth=80);

	/**
	 * Finds all classes in the current document to which objects of class c
	 * are in some way related. Posible relations are Associations (gneratlization,
	 * composition, etc) as well as parameters to methos and return values
	 * this is useful in deciding which classes/files to import/include in code generation
	 * @param c the class for which relations are to be found
	 * @param cList a reference to the list into which return the result
	 */
	void findObjectsRelated(UMLClassifier *c, QPtrList<UMLClassifier> &cList);

	bool hasDefaultValueAttr(UMLClass *c);

	bool hasAbstractOps(UMLClassifier *c);

	/* Attributes*/
	QDir m_outputDirectory;
	OverwritePolicy m_overwrite;
	ModifyNamePolicy m_modname;
	QDir m_headingFiles;

	UMLDoc *m_doc;
	bool m_forceDoc;
	bool m_forceSections;
	bool m_includeHeadings;

	/**
	 * used by overwriteDialogue to know if the apply to all 
	 * remaining files checkbox should be checked (is by default)
	 */
	bool m_applyToAllRemaining;

	/**
	 * Maps UMLObjects to filenames. Used for finding out which file
	 * each class was written to.
	 */
	QMap<UMLClassifier*,QString> *m_fileMap;

signals:
	/**
	 * This signal is emitted when code for a UMLClassifier has been 
	 * generated.
	 * @param concept    The concept which was processed
	 * @param generated  Flag, set to true if generation was successfull
	 */
	void codeGenerated(UMLClassifier* concept, bool generated);
};


/**
 * WriterFactory is the Factory class for the library.
 * see the documentation of KLibFactory.
 * see the example implementation in "factory.cpp" by Luis De la Parra
 *
 * This class creates Objects and returns them to be used by the application.
 *
 * If you want to write a library containing Code Generators, you have
 * to implement the generators you want by subclassing CodeGenerator
 * (see the doc for CodeGenerator) and implement WriterFactory. Then
 * build the library as "shared library" with the option "module" so
 * that it can be dlopened, and distribute it.
 *
 * Your implementation of WriterFactory should behave like this:
 *
 * languagesAvailable() must return a QStringList of all the languages
 * offered by this library. For example, if in your library you
 * implement generators for C++, Python, and "PseudoCode", then you
 * should return a list containing this names.
 *
 * generatorName(const QString &l) must return the Class name of the
 * object implementing the language "l". Remember that you can name
 * your generators what you want, so if you subclased CodeGenerator
 * and created a class named "MyFirstCodeGenerator", which outputs
 * Java code, then you should return a QString "MyFirstCodeGenerator"
 * when l = "Java" If your library doesnt recognize /does not
 * implement the language "l", then return an empty string.
 *
 * createObject(...) receives a object name in "name". you should
 * check what this is and then return a object of that type. For
 * example, if name = "MyFirstCodeGenerator", then you should create
 * an object of that type and return a pointer to it. If you did not
 * write any class named "name" in your library, return NULL
 */
class WriterFactory : public KLibFactory {
	Q_OBJECT
public:

	WriterFactory( QObject *parent = 0, const char *name = 0 );
	virtual ~WriterFactory();

	/**
	 * Returns a QStringList containing the languages offered by this library
	 */
	QStringList languagesAvailable();

	/**
	 * Returns the name of the generator which implements language l
	 */
	QString generatorName(const QString &l);

	virtual QObject* createObject( QObject* parent = 0, const char* pname = 0,
	                               const char* name = "QObject",
	                               const QStringList &args = QStringList() );

private:
	static KInstance* s_instance;
};


class GeneratorInfo {
public:
	QString language;
	QString library;
	QString object;

};

#endif

