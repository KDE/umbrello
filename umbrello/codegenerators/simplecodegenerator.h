/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef SIMPLECODEGENERATOR_H
#define SIMPLECODEGENERATOR_H

#include "basictypes.h"
#include "codegenerator.h"

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QMap>

class UMLDoc;

/**
 * A simple code generator interface designed to work with
 * the existing codewriters.
 */
class SimpleCodeGenerator : public CodeGenerator
{
    Q_OBJECT
public:

    SimpleCodeGenerator(bool createDirHierarchyForPackages = true);
    virtual ~SimpleCodeGenerator();

    void writeCodeToFile(UMLClassifierList & concepts);
    void writeCodeToFile();

    /**
     * Call this method to generate code for a UMLClassifier
     * @param c the class you want to generate code for.
     */
    virtual void writeClass(UMLClassifier *c) = 0;

    CodeDocument * newClassifierCodeDocument(UMLClassifier * classifier);

protected:

    QString className_;
    QString fileName_;

    QString findFileName(UMLPackage* concept, const QString &ext);
    QString overwritableName(UMLPackage* concept, const QString &name, const QString &ext);
    bool hasDefaultValueAttr(UMLClassifier *c);
    bool hasAbstractOps(UMLClassifier *c);

    QString indent();

    /**
     * Maps UMLObjects to filenames. Used for finding out which file
     * each class was written to.
     */
    QMap<UMLPackage*,QString> m_fileMap;

    UMLDoc *m_doc;  ///< the parent document

    /**
     * For some code generators, it does not make much sense to create a
     * directory for each package because that would lead to a rather
     * sparsely populated directory tree (maximum of just one source file
     * per directory.)
     */
    bool m_createDirHierarchyForPackages;

    // old attributes writers will look for
    QString m_indentation;
    int m_indentLevel;
    QString m_endl;

    // override parent method..we need special handling
    void initFromParentDocument();

private:

    void initFields(UMLDoc * doc) ;

public slots:

    void syncCodeToDocument();

};

#endif // SIMPLECODEGENERATOR_H
