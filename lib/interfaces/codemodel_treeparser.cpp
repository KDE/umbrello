/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codemodel_treeparser.h"
#include "codemodel.h"

CodeModelTreeParser::CodeModelTreeParser()
{
}

CodeModelTreeParser::~CodeModelTreeParser()
{
}

void CodeModelTreeParser::parseCode(const CodeModel * model)
{
    const FileList fileList = model->fileList();
    for (FileList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it)
        parseFile(*it);
}

void CodeModelTreeParser::parseFile(const FileModel * file)
{
    const NamespaceList namespaceList = file->namespaceList();
    const ClassList classList = file->classList();
    const FunctionList functionList = file->functionList();
    const FunctionDefinitionList functionDefinitionList = file->functionDefinitionList();
    const VariableList variableList = file->variableList();

    for (NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it)
        parseNamespace(*it);
    for (ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it)
        parseClass(*it);
    for (FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it)
        parseFunction(*it);
    for (FunctionDefinitionList::ConstIterator it=functionDefinitionList.begin(); it!=functionDefinitionList.end(); ++it)
        parseFunctionDefinition(*it);
    for (VariableList::ConstIterator it=variableList.begin(); it!=variableList.end(); ++it)
        parseVariable(*it);
}

void CodeModelTreeParser::parseNamespace(const NamespaceModel * ns)
{
    const NamespaceList namespaceList = ns->namespaceList();
    const ClassList classList = ns->classList();
    const FunctionList functionList = ns->functionList();
    const FunctionDefinitionList functionDefinitionList = ns->functionDefinitionList();
    const VariableList variableList = ns->variableList();

    for (NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it)
        parseNamespace(*it);
    for (ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it)
        parseClass(*it);
    for (FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it)
        parseFunction(*it);
    for (FunctionDefinitionList::ConstIterator it=functionDefinitionList.begin(); it!=functionDefinitionList.end(); ++it)
        parseFunctionDefinition(*it);
    for (VariableList::ConstIterator it=variableList.begin(); it!=variableList.end(); ++it)
        parseVariable(*it);
}

void CodeModelTreeParser::parseClass(const ClassModel * klass)
{
    const ClassList classList = klass->classList();
    const FunctionList functionList = klass->functionList();
    const FunctionDefinitionList functionDefinitionList = klass->functionDefinitionList();
    const VariableList variableList = klass->variableList();

    for (ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it)
        parseClass(*it);
    for (FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it)
        parseFunction(*it);
    for (FunctionDefinitionList::ConstIterator it=functionDefinitionList.begin(); it!=functionDefinitionList.end(); ++it)
        parseFunctionDefinition(*it);
    for (VariableList::ConstIterator it=variableList.begin(); it!=variableList.end(); ++it)
        parseVariable(*it);
}

void CodeModelTreeParser::parseFunction(const FunctionModel * /*fun*/)
{
}

void CodeModelTreeParser::parseFunctionDefinition(const FunctionDefinitionModel * /*fun*/)
{
}

void CodeModelTreeParser::parseVariable(const VariableModel * /*var*/)
{
}



