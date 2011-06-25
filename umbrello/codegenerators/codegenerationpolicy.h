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

#ifndef CODEGENERATIONPOLICY_H
#define CODEGENERATIONPOLICY_H

#include "basictypes.h"

#include <QtCore/QObject>
#include <QtCore/QDir>

class QWidget;
class CodeGenerationPolicyPage;

/**
 * class CodeGenerationPolicy
 * This class describes the code generation policy for this project.
 * Note that as the code gen policy may be the 'default' policy, it may
 * not be coupled with a code generator.
 */
class CodeGenerationPolicy : public QObject
{
    Q_OBJECT
public:

    /**
     * OverwritePolicy  can have the following values
     *  - Ok: if there is a file named the same as what you want to name your output file,
     *        you can overwrite the old file.
     *  - Ask:if there is a file named the same as what you want to name your output file,
     *        you should ask the User what to do, and give him the option to overwrite the file
     *        write the code to a different file, or to abort the generation of this class.
     *  - Never: you cannot overwrite any files. Generates a new file name like "fileName1.h", "fileName2.h"
     *        until you find an appropriate name.
     *  - Cancel: Do not output anything.  This is only set if the user chooses Apply to All Remaining Files
     *            and clicks on Do not Output  in the Ask dialog
     */
    enum OverwritePolicy {Ok=0, Ask, Never, Cancel};
    enum ModifyNamePolicy {No=0, Underscore, Capitalise};
    enum NewLineType {UNIX=0, DOS, MAC};
    enum IndentationType {NONE=0, TAB, SPACE};
    enum CommentStyle { SingleLine=0, MultiLine };

    CodeGenerationPolicy (CodeGenerationPolicy * clone);
    CodeGenerationPolicy ();

    virtual ~CodeGenerationPolicy ( );

    void setOverwritePolicy ( OverwritePolicy new_var );
    OverwritePolicy getOverwritePolicy ( ) const;

    void setCodeVerboseSectionComments ( bool new_var );
    bool getCodeVerboseSectionComments ( ) const;

    void setCodeVerboseDocumentComments ( bool new_var );
    bool getCodeVerboseDocumentComments ( ) const;

    void setHeadingFileDir ( const QString & path);
    QString getHeadingFileDir ( ) const;

    void setIncludeHeadings ( bool new_var );
    bool getIncludeHeadings ( ) const;

    void setOutputDirectory ( QDir new_var );
    QDir getOutputDirectory ( );

    void setLineEndingType ( NewLineType type );
    NewLineType getLineEndingType ( );

    QString getNewLineEndingChars ( ) const;

    void setIndentationType ( IndentationType type );
    IndentationType getIndentationType ( );

    void setIndentationAmount ( int amount );
    int getIndentationAmount ( );

    QString getIndentation ( ) const;

    void setModifyPolicy ( ModifyNamePolicy new_var );
    ModifyNamePolicy getModifyPolicy ( ) const;

    void setAutoGenerateConstructors ( bool var );
    bool getAutoGenerateConstructors ( );

    void setAttributeAccessorScope(Uml::Visibility::Value var);
    Uml::Visibility::Value getAttributeAccessorScope();

    void setAssociationFieldScope(Uml::Visibility::Value var);
    Uml::Visibility::Value getAssociationFieldScope();

    virtual CodeGenerationPolicyPage * createPage ( QWidget *parent = 0, const char * name = 0);

    QString getHeadingFile(const QString& str);

    virtual void setDefaults (CodeGenerationPolicy * defaults, bool emitUpdateSignal = true);
    virtual void setDefaults(bool emitUpdateSignal = true);

    virtual void writeConfig ();

    void emitModifiedCodeContentSig();

    void setCommentStyle ( CommentStyle new_var );
    CommentStyle getCommentStyle ( );

signals:

    void modifiedCodeContent();

protected:

/*
    // Policy of how to deal with overwriting existing files. Allowed values are "ask", "yes" and "no".
    OverwritePolicy m_overwritePolicy;

    // Whether or not verbose code commenting for sections is desired.
    // If true, comments for sections will be written even if the section is empty.
    bool m_codeVerboseSectionComments;

    // Whether or not verbose code commenting for documentation is desired.
    // If true, documentation for various code will be written even if no
    //code would normally be created at that point in the file.
    bool m_codeVerboseDocumentComments;

    QDir m_headingFiles; // location of the header file template.
    bool m_includeHeadings;
    QDir m_outputDirectory; // location of where output files will go.
    NewLineType m_lineEndingType; // What type of line ending characters to use.
    IndentationType m_indentationType; // The amount and type of whitespace to indent with.
    int m_indentationAmount; // The amount of units to indent with.
    ModifyNamePolicy m_modifyPolicy;
    bool m_autoGenerateConstructors;
    CommentStyle m_commentStyle;
    Uml::Visibility::Value m_attributeAccessorScope;
    Uml::Visibility::Value m_associationFieldScope;
*/

    // these 2 private fields 'cache' the string values of other fields we may frequently call for
    QString m_lineEndingChars;
    QString m_indentation;

    void calculateIndentation();

/*
protected:

    void initFields ( );
*/
};

#endif // CODEGENERATIONPOLICY_H
