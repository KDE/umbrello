/***************************************************************************
                          tclwriter.h  -  description
                             -------------------
    begin               : Thu Jul 26 2005
    copyright           : (C) 2005 by Rene Meyer
    email               : rene.meyer@sturmit.de

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TCLWRITER_H
#define TCLWRITER_H

#include <qstringlist.h>
#include "simplecodegenerator.h"
#include "../umloperationlist.h"
#include "../umlattributelist.h"
#include "../umlassociationlist.h"

class           QFile;
class           QTextStream;
class           ClassifierInfo;

/**
  * class TclWriter is a code generator for UMLClassifier objects.
  * Create an instance of this class, and feed it a UMLClassifier when
  * calling writeClass and it will generate both a header (.h) and 
  * source (.tcl) file for that classifier.
  */
class TclWriter : public SimpleCodeGenerator
{
public:

    /**
     * Constructor, initialises a couple of variables
     */
    TclWriter(UMLDoc * parent = 0, const char *name = 0);

    /**
     * Destructor, empty
     */
    virtual ~ TclWriter();

    /**
     * call this method to generate tcl code for a UMLClassifier
     * @param c the class to generate code for
     */
    virtual void    writeClass(UMLClassifier * c);

    /**
     * checks whether type is "TclWriter"
     *
     * @param type
     */
    virtual bool    isType(QString & type);

    /**
     * returns "C++"
     */
    virtual QString getLanguage();

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

private:
    /**
     * Current output stream.
     */
    QTextStream * mStream;
    /**
     * write the header file for this classifier.
     */
    void            writeHeaderFile(UMLClassifier * c, QFile & file);

    /**
     * write the source code body file for this classifier.
     */
    void            writeSourceFile(UMLClassifier * c, QFile & file);

    /**
     * write the source codei text.
     */
    void            TclWriter::writeCode(QString text);

    /**
     * write comment text.
     */
    void            TclWriter::writeComm(QString text);

    /**
     * write documentation text.
     */
    void            TclWriter::writeDocu(QString text);

    void            TclWriter::writeConstructorDecl();

    void            TclWriter::writeDestructorDecl();

    /**
     * Summary information about current classifier.
     */
    ClassifierInfo *classifierInfo;
    QString         mNamespace;
    QString         mClass;
    QString         mClassGlobal;

    /**
     * writes the Attribute declarations
     * @param visibility the visibility of the attribs to print out
     * @param writeStatic whether to write static or non-static attributes out
     * @param stream text stream
     */
    void            writeAttributeDecl(Uml::Scope visibility, bool writeStatic);

    void            TclWriter::writeAssociationIncl(UMLAssociationList list,
            Uml::IDType myId, QString type);
    /**
     * Searches a list of associations for appropriate ones to write out as attributes
     */
    void            writeAssociationDecl(UMLAssociationList associations,
                                         Uml::Scope permit, Uml::IDType id, QString type);

    /**
     * Writes out an association as an attribute using Vector
     */
    void            writeAssociationRoleDecl(QString fieldClassName,
            QString roleName, QString multi, QString doc, QString docname);

    /**
     * If needed, write out the declaration for the method to initialize attributes of our class.
     */
    void            writeInitAttributeHeader();
    void            writeInitAttributeSource();

    void            writeConstructorHeader();
    void            writeConstructorSource();
    void            writeDestructorHeader();
    void            writeDestructorSource();
    void            writeOperationHeader(UMLClassifier * c,
                                         Uml::Scope permitScope);
    void            writeOperationSource(UMLClassifier * c,
                                         Uml::Scope permitScope);
    void            writeAttributeSource();
    void            writeAssociationSource(UMLAssociationList associations,
                                           Uml::IDType id);
    void            writeAssociationRoleSource(QString fieldClassName, QString roleName,
            QString multi);
    void            writeUse(UMLClassifier * c);



    /**
     * Returns the name of the given object (if it exists)
     */
    QString         getUMLObjectName(UMLObject * obj);

    /**
     * Replaces `string' with STRING_TYPENAME.
     */
    QString         fixTypeName(QString string);

    QStringList     ObjectFieldVariables;
    QStringList     VectorFieldVariables;

};



#endif // TCLWRITER_H
