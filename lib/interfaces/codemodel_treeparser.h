/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2004 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef CODEMODEL_TREEPARSER_H
#define CODEMODEL_TREEPARSER_H

/**
@file codemodel_treeparser.h
Tree parser for a code model.
*/

class FileModel;
class NamespaceModel;
class ClassModel;
class FunctionModel;
class FunctionDefinitionModel;
class VariableModel;

/**
Tree parser for a code model.
This is a base class which can be subclassed to create a code model "tree parser".
Such tree parsers can be used to recursively walk through the whole code model
or its parts and do some actions.

Default implementation only walks through the model and does nothing. You need
to reimplement necessary virtual methods of this class to add desired functionality.
*/
class CodeModelTreeParser
{
public:
    /**Constructor.*/
    CodeModelTreeParser();
    /**Destructor.*/
    virtual ~CodeModelTreeParser();

    /**Parses recursively all code in the @p model.
    @param model CodeModel to parse.*/
    virtual void parseCode(const CodeModel* model);

    /**Parses recursively all code in the @p file.
    @param file FileModel to parse.*/
    virtual void parseFile(const FileModel* file);

    /**Parses recursively all code in the namespace @p ns.
    @param ns NamespaceModel to parse.*/
    virtual void parseNamespace(const NamespaceModel* ns);

    /**Parses recursively all code in the class @p klass.
    @param klass ClassModel to parse.*/
    virtual void parseClass(const ClassModel* klass);

    /**Parses function (function declaration) @p fun.
    @param fun FunctionModel to parse.*/
    virtual void parseFunction(const FunctionModel* fun);

    /**Parses function definition @p fun.
    @param fun FunctionDefinitionModel to parse.*/
    virtual void parseFunctionDefinition(const FunctionDefinitionModel* fun);

    /**Parses variable @p var.
    @param var VariableModel to parse.*/
    virtual void parseVariable(const VariableModel* var);

private:
    CodeModelTreeParser(const CodeModelTreeParser& source);
    void operator = (const CodeModelTreeParser& source);
};

#endif
