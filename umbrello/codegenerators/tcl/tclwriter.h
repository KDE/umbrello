/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Rene Meyer <rene.meyer@sturmit.de>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef TCLWRITER_H
#define TCLWRITER_H

#include "simplecodegenerator.h"
#include "umloperationlist.h"
#include "umlattributelist.h"
#include "umlassociationlist.h"

#include <QStringList>

class QFile;
class QTextStream;

/**
 * Class TclWriter is a code generator for UMLClassifier objects.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate both a header (.h) and
 * source (.tcl) file for that classifier.
 */
class TclWriter : public SimpleCodeGenerator
{
public:

    TclWriter();
    virtual ~TclWriter();

    virtual void writeClass(UMLClassifier * c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

private:

    void writeHeaderFile(UMLClassifier * c, QFile & file);
    void writeSourceFile(UMLClassifier * c, QFile & file);

    void writeCode(const QString &text);
    void writeComm(const QString &text);
    void writeDocu(const QString &text);

    void writeConstructorDecl();
    void writeDestructorDecl();

    void writeAttributeDecl(UMLClassifier * c, Uml::Visibility::Enum visibility, bool writeStatic);

    void writeAssociationIncl(UMLAssociationList list,
                              Uml::ID::Type myId, const QString &type);
    void writeAssociationDecl(UMLAssociationList associations,
                              Uml::Visibility::Enum permit, Uml::ID::Type id,
                              const QString &type);

    void writeAssociationRoleDecl(const QString &fieldClassName,
                                  const QString &roleName, const QString &multi,
                                  const QString &doc, const QString &docname);

    void writeInitAttributeHeader(UMLClassifier * c);
    void writeInitAttributeSource(UMLClassifier* c);

    void writeConstructorHeader();
    void writeConstructorSource(UMLClassifier * c);

    void writeDestructorHeader();
    void writeDestructorSource();

    void writeOperationHeader(UMLClassifier * c,
                              Uml::Visibility::Enum permitScope);
    void writeOperationSource(UMLClassifier * c,
                              Uml::Visibility::Enum permitScope);

    void writeAttributeSource(UMLClassifier * c);

    void writeAssociationSource(UMLAssociationList associations,
                                Uml::ID::Type id);
    void writeAssociationRoleSource(const QString &fieldClassName,
                                    const QString &roleName,
                                    const QString &multi);

    void writeUse(UMLClassifier * c);

    QString getUMLObjectName(UMLObject * obj);

    QString fixTypeName(const QString &string);

    QTextStream*  mStream;        ///< current output stream
    QString       mNamespace;     ///< namespace information
    QString       mClassGlobal;   ///< information about current classifier
    QStringList   ObjectFieldVariables;
    QStringList   VectorFieldVariables;

};

#endif // TCLWRITER_H
