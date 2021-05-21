/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2003-2004 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2004 Jonas Jacobi <j.jacobi@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef CODEMODEL_UTILS_H
#define CODEMODEL_UTILS_H

#include "codemodel.h"

/**
@file codemodel_utils.h
Utility functions and classes for the CodeModel.
*/

/**
@class Pred
The predicate.
Pred is not a real class, it is only a template parameter used in @ref CodeModelUtils functions.

<b>How to create the predicate:</b>@n
Predicate is simply a class that have
@code bool operator() (predicateArgument) @endcode.
The return value of that operator is the result of a predicate.

For example we want to find all function definitions with a particular name.
We can use @ref CodeModelUtils::findFunctionDefinitions functions that require
you to write a predicate for function definition DOM's.
This can be done with following code:
@code
class MyPred{
public:
    MyPred(const QString &name): m_name(name) {}

    bool operator() (const FunctionDefinitionDom &def) const
    {
        return def->name() == m_name;
    }

private:
    QString m_name;
};
@endcode
*/


/**Namespace which contains utility functions and classes for the CodeModel.*/
namespace CodeModelUtils
{

/**Finds function definitions which match given predicate in files.

Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function definition before it is returned.
@param fileList The list of files to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions(Pred pred, const FileList& fileList, FunctionDefinitionList & lst);

/**Finds function definitions which match given predicate in the namespace.

Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function definition before it is returned.
@param ns The namespace to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions(Pred pred, const NamespaceDom& ns, FunctionDefinitionList & lst);

/**Finds function definitions which match given predicate in namespaces.

Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function definition before it is returned.
@param namespaceList The list of namespaces to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions(Pred pred, const NamespaceList& namespaceList, FunctionDefinitionList & lst);

/**Finds function definitions which match given predicate in classes.

Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function definition before it is returned.
@param classList The list of classes to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions(Pred pred, const ClassList& classList, FunctionDefinitionList & lst);

/**Finds function definitions which match given predicate in the list of function definitions.

Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function definition before it is returned.
@param functionDefinitionList The list of function definitions to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions(Pred pred, const FunctionDefinitionList& functionDefinitionList, FunctionDefinitionList & lst);

/**Finds function definitions which match given predicate in the class.

Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function definition before it is returned.
@param klass The class to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions(Pred pred, const ClassDom& klass, FunctionDefinitionList & lst);

/**Applies a predicate to a function definition.

Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function definition before it is returned.
@param fun The function definition.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions(Pred pred, const FunctionDefinitionDom& fun, FunctionDefinitionList & lst);

/**Finds function declarations which match given predicate in files.

Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function declaration before it is returned.
@param fileList The list of files to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations(Pred pred, const FileList& fileList, FunctionList & lst);

/**Finds function declarations which match given predicate in the namespace.

Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function declaration before it is returned.
@param ns The namespace to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations(Pred pred, const NamespaceDom& ns, FunctionList & lst);

/**Finds function declarations which match given predicate in namespaces.

Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function declaration before it is returned.
@param namespaceList The list of namespaces to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations(Pred pred, const NamespaceList& namespaceList, FunctionList & lst);

/**Finds function declarations which match given predicate in classes.

Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function declaration before it is returned.
@param classList The list of classes to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations(Pred pred, const ClassList& classList, FunctionList & lst);

/**Finds function declarations which match given predicate in the list of function declarations.

Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function declaration before it is returned.
@param functionList The list of function declarations to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations(Pred pred, const FunctionList& functionList, FunctionList & lst);

/**Finds function declarations which match given predicate in the class.

Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function declaration before it is returned.
@param klass The class to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations(Pred pred, const ClassDom& klass, FunctionList & lst);

/**Applies a predicate to a function declaration.

Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.

@param pred Predicate which is applied to a function declaration before it is returned.
@param fun The function declaration.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations(Pred pred, const FunctionDom& fun, FunctionList & lst);


//implementations of function templates defined above:

template <class Pred>
void findFunctionDefinitions(Pred pred, const FileList& fileList, FunctionDefinitionList & lst)
{
    for (FileList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it)
        findFunctionDefinitions(pred, model_cast<NamespaceDom>(*it), lst);
}

template <class Pred>
void findFunctionDefinitions(Pred pred, const NamespaceDom& ns, FunctionDefinitionList & lst)
{
    findFunctionDefinitions(pred, ns->namespaceList(), lst);
    findFunctionDefinitions(pred, ns->classList(), lst);
    findFunctionDefinitions(pred, ns->functionDefinitionList(), lst);
}

template <class Pred>
void findFunctionDefinitions(Pred pred, const NamespaceList& namespaceList, FunctionDefinitionList & lst)
{
    for (NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it)
        findFunctionDefinitions(pred, *it, lst);
}

template <class Pred>
void findFunctionDefinitions(Pred pred, const ClassList& classList, FunctionDefinitionList & lst)
{
    for (ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it)
        findFunctionDefinitions(pred, *it, lst);
}

template <class Pred>
void findFunctionDefinitions(Pred pred, const FunctionDefinitionList& functionDefinitionList, FunctionDefinitionList & lst)
{
    for (FunctionDefinitionList::ConstIterator it=functionDefinitionList.begin(); it!=functionDefinitionList.end(); ++it)
        findFunctionDefinitions(pred, *it, lst);
}

template <class Pred>
void findFunctionDefinitions(Pred pred, const ClassDom& klass, FunctionDefinitionList & lst)
{
    findFunctionDefinitions(pred, klass->classList(), lst);
    findFunctionDefinitions(pred, klass->functionDefinitionList(), lst);
}

template <class Pred>
void findFunctionDefinitions(Pred pred, const FunctionDefinitionDom& fun, FunctionDefinitionList & lst)
{
    if (pred(fun))
        lst << fun;
}



template <class Pred>
void findFunctionDeclarations(Pred pred, const FileList& fileList, FunctionList & lst)
{
    for (FileList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it)
        findFunctionDeclarations(pred, model_cast<NamespaceDom>(*it), lst);
}

template <class Pred>
void findFunctionDeclarations(Pred pred, const NamespaceDom& ns, FunctionList & lst)
{
    findFunctionDeclarations(pred, ns->namespaceList(), lst);
    findFunctionDeclarations(pred, ns->classList(), lst);
    findFunctionDeclarations(pred, ns->functionList(), lst);
}

template <class Pred>
void findFunctionDeclarations(Pred pred, const NamespaceList& namespaceList, FunctionList & lst)
{
    for (NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it)
        findFunctionDeclarations(pred, *it, lst);
}

template <class Pred>
void findFunctionDeclarations(Pred pred, const ClassList& classList, FunctionList & lst)
{
    for (ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it)
        findFunctionDeclarations(pred, *it, lst);
}

template <class Pred>
void findFunctionDeclarations(Pred pred, const FunctionList& functionList, FunctionList & lst)
{
    for (FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it)
        findFunctionDeclarations(pred, *it, lst);
}

template <class Pred>
void findFunctionDeclarations(Pred pred, const ClassDom& klass, FunctionList & lst)
{
    findFunctionDeclarations(pred, klass->classList(), lst);
    findFunctionDeclarations(pred, klass->functionList(), lst);
}

template <class Pred>
void findFunctionDeclarations(Pred pred, const FunctionDom& fun, FunctionList & lst)
{
    if (pred(fun))
        lst << fun;
}

/**A scope.*/
struct Scope {
    /**Class.*/
    ClassDom klass;
    /**Namespace.*/
    NamespaceDom ns;
};

/**Information about functions.*/
struct AllFunctions {
    /**Scope of functions.*/
    QMap<FunctionDom, Scope> relations;
    /**List of functions.*/
    FunctionList functionList;
};
/**Information about function definitions.*/
struct AllFunctionDefinitions {
    /**Scope of function definitions.*/
    QMap<FunctionDefinitionDom, Scope> relations;
    /**List of function definitions.*/
    FunctionDefinitionList functionList;
};

/**Namespace with utilities to find functions in the @ref CodeModel.*/
namespace Functions
{
/**Looks for functions in the class.
@param list The list of functions found by this routine.
@param dom The class to look for functions.*/
void processClasses(FunctionList &list, const ClassDom dom);

/**Looks for functions in the namespace.
@param list The list of functions found by this routine.
@param dom The namespace to look for functions.*/
void processNamespaces(FunctionList &list, const NamespaceDom dom);

/**Looks for functions in the class and also saves their scope.
@param list The list of functions found by this routine.
@param dom The class to look for functions.
@param relations The scope information.*/
void processClasses(FunctionList &list, const ClassDom dom, QMap<FunctionDom, Scope> &relations);

/**Looks for functions in the class and also saves their scope.
Used for classes within a namespace.
@param list The list of functions found by this routine.
@param dom The class to look for functions.
@param relations The scope information.
@param nsdom The namespace which contains a class.*/
void processClasses(FunctionList &list, const ClassDom dom, QMap<FunctionDom, Scope> &relations, const NamespaceDom &nsdom);

/**Looks for functions in the namespace and also saves their scope.
@param list The list of functions found by this routine.
@param dom The namespace to look for functions.
@param relations The scope information.*/
void processNamespaces(FunctionList &list, const NamespaceDom dom, QMap<FunctionDom, Scope> &relations);
}

/**Namespace with utilities to find function definitions in the @ref CodeModel.*/
namespace FunctionDefinitions
{
/**Looks for function definitions in the class.
@param list The list of function definitions found by this routine.
@param dom The class to look for function definitions.*/
void processClasses(FunctionDefinitionList &list, const ClassDom dom);

/**Looks for function definitions in the namespace.
@param list The list of function definitions found by this routine.
@param dom The namespace to look for function definitions.*/
void processNamespaces(FunctionDefinitionList &list, const NamespaceDom dom);

/**Looks for function definitions in the class and also saves their scope.
@param list The list of function definitions found by this routine.
@param dom The class to look for function definitions.
@param relations The scope information.*/
void processClasses(FunctionDefinitionList &list, const ClassDom dom, QMap<FunctionDefinitionDom, Scope> &relations);

/**Looks for function definitions in the class and also saves their scope.
Used for classes within a namespace.
@param list The list of function definitions found by this routine.
@param dom The class to look for function definitions .
@param relations The scope information.
@param nsdom The namespace which contains a class.*/
void processClasses(FunctionDefinitionList &list, const ClassDom dom, QMap<FunctionDefinitionDom, Scope> &relations, const NamespaceDom &nsdom);

/**Looks for function definitions in the namespace and also saves their scope.
@param list The list of function definitions found by this routine.
@param dom The namespace to look for function definitions.
@param relations The scope information.*/
void processNamespaces(FunctionDefinitionList &list, const NamespaceDom dom, QMap<FunctionDefinitionDom, Scope> &relations);
}

/**
 * Compares a declaration and a definition of a function.
 * @param dec declaration
 * @param def definition
 * @return true, if dec is the declaration of the function definition def, false otherwise
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
bool compareDeclarationToDefinition(const FunctionDom& dec, const FunctionDefinitionDom& def);

/**
 * Compares a declaration and a definition of a function.
 * @param dec declaration
 * @param def definition
 * @param nsImports namespace imports for the namespace the definition appears in
 * @return true, if dec is the declaration of the function definition def, false otherwise
 */
bool compareDeclarationToDefinition(const FunctionDom& dec, const FunctionDefinitionDom& def, const std::set<NamespaceImportModel>& nsImports);

/**
 * Predicate for use with findFunctionDefinitions. Searches for a definition matching a declaration.
 * @sa Pred documentation to learn more about predicates used with code model.
 * @author Jonas Jacobi
 */
class PredDefinitionMatchesDeclaration
{
public:
    explicit PredDefinitionMatchesDeclaration(const FunctionDom& func) : m_declaration(func) {}
    bool operator() (const FunctionDefinitionDom& def) const
    {
        return compareDeclarationToDefinition(m_declaration, def);
    }

private:
    const FunctionDom m_declaration;
};

template <class InputDomType>
class PredAmOwner
{
public:
    PredAmOwner(const FileDom& file) : m_file(file) {}
    bool operator() (const InputDomType& def) const
    {
        return def->file() == m_file;
    }

private:
    const FileDom m_file;
};

/**@return A list of all functions in the file.
@param dom File Dom to look for functions in.*/
FunctionList allFunctions(const FileDom &dom);
/**@return A detailed list of all functions in the file (detailed list contains
the information about a scope of each FunctionDom found).
@param dom File Dom to look for functions in.*/
AllFunctions allFunctionsDetailed(const FileDom &dom);
/**@return A detailed list of all function definitions in the file (detailed list contains
the information about a scope of each FunctionDefinitionDom found).
@param dom File Dom to look for functions in.*/
AllFunctionDefinitions allFunctionDefinitionsDetailed(const FileDom &dom);

/**@return A list of all functions in the file.
This version searches the file's whole group for
functions that may have been inserted into the other file's
structure.
Unlike the methods above, this guarantees that all returned
functions physically belong to that file.
@param dom File Dom to look for functions in. */
FunctionList allFunctionsExhaustive(FileDom &dom);

/**@return A list of all function-definitions in the file.
This version searches the file's whole group for
functions that may have been inserted into the other file's
structure.
Unlike the methods above, this guarantees that all returned
functions physically belong to that file.
@param dom File Dom to look for functions in.  */
FunctionDefinitionList allFunctionDefinitionsExhaustive(FileDom &dom);

/**
 * Finds a class by its position in a file(position inside the part of the file, where the class is declared).
 * In the case of nested classes the innermost class which is declared at/around the provided position.
 * @param nameSpace A namespace to search for the class.
 * @param line A linenumber inside the class declaration.
 * @param col The column of line.
 * @return The innermost class, which is declared at/around position defined with line / col, or 0 if no class is found.
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
ClassDom findClassByPosition(NamespaceModel* nameSpace, int line, int col);

/**
 * Same as above, just searches inside a class instead of a namespace.
 */
ClassDom findClassByPosition(ClassModel* aClass, int line, int col);

/**
 * Finds the last occurrence (line of file wise) of a method inside a class declaration with specific access specificer.
 * This can be used e.g. to find a position to new methods to the class.
 * @param aClass class to search for method.
 * @param access the access specifier with which methods are searched for.
 * @return The last line a Method with access specifier access is found,
 * or -1 if no method with that access specifier was found.
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
int findLastMethodLine(ClassDom aClass, CodeModelItem::Access access);

/**
 * Same as above, but finds a membervariable instead of a method.
 */
int findLastVariableLine(ClassDom aClass, CodeModelItem::Access access);

/**
 * Get the string representation of an access specifier
 * @param access An access specifier to get a string representation of.
 * @return string The representation of an access (e.g. "public").
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
QString accessSpecifierToString(CodeModelItem::Access access);


class CodeModelHelper
{
private:
    CodeModel* m_model;
    FileList m_files;
    QString m_fileName;

    FunctionDefinitionDom functionDefinitionAt(NamespaceDom ns, int line, int column);

    FunctionDefinitionDom functionDefinitionAt(ClassDom klass, int line, int column);

    FunctionDefinitionDom functionDefinitionAt(FunctionDefinitionDom fun, int line, int);

    FunctionDom functionDeclarationAt(NamespaceDom ns, int line, int column);

    FunctionDom functionDeclarationAt(ClassDom klass, int line, int column);

    FunctionDom functionDeclarationAt(FunctionDom fun, int line, int column);


    ClassDom classAt(NamespaceDom ns, int line, int column);

    ClassDom classAt(ClassDom klass, int line, int column);

public:
    CodeModelHelper(CodeModel* model, FileDom file);

    enum FunctionTypes {
        Declaration = 1,
        Definition = 2
    };

    FunctionDom functionAt(int line, int column, FunctionTypes types = (FunctionTypes)3);
    ClassDom classAt(int line, int column);
};

}

#endif
