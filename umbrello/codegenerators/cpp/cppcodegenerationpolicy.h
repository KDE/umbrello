/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CPPCODEGENERATIONPOLICY_H
#define CPPCODEGENERATIONPOLICY_H

#include "codegenpolicyext.h"
#include "codegenerationpolicy.h"

#include <QtCore/QString>

class KConfig;
class CodeGenerationPolicyPage;

class CPPCodeGenerationPolicy : public CodeGenPolicyExt
{
    Q_OBJECT
public:

    static const char * DEFAULT_VECTOR_METHOD_APPEND;
    static const char * DEFAULT_VECTOR_METHOD_REMOVE;
    static const char * DEFAULT_VECTOR_METHOD_INIT;
    static const char * DEFAULT_OBJECT_METHOD_INIT;

    CPPCodeGenerationPolicy();
    virtual ~CPPCodeGenerationPolicy();

    void setAccessorsAreInline(bool var);
    bool getAccessorsAreInline();

    void setOperationsAreInline(bool var);
    bool getOperationsAreInline();

    void setDestructorsAreVirtual(bool var);
    bool getDestructorsAreVirtual();

    void setPackageIsNamespace(bool var);
    bool getPackageIsNamespace();

    void setAutoGenerateAccessors(bool var);
    bool getAutoGenerateAccessors();

    void setAccessorsArePublic(bool var);
    bool getAccessorsArePublic();

    /**
     * We want to be flexible about which classes are allowed for generation
     * of the CPP code. In the next 4 method pairs, we give accessors that allow setting and getting
     * the names of the classes, and their include files for string and vectors.
     */
    void setStringClassName(const QString &value);
    QString getStringClassName();
    void setStringClassNameInclude(const QString &value);
    QString getStringClassNameInclude();
    void setVectorClassName(const QString &value);
    QString getVectorClassName();
    void setVectorClassNameInclude(const QString &value);
    QString getVectorClassNameInclude();

    void setDocToolTag(const QString &value);
    QString getDocToolTag();

    void setStringIncludeIsGlobal (bool value);
    bool stringIncludeIsGlobal ();

    void setVectorIncludeIsGlobal (bool value);
    bool vectorIncludeIsGlobal ();

    /** More flexible generation. We want to allow the user to specify how the
     *  bodies of the vector methods should be auto-generated.
     */
    QString getVectorMethodAppend(const QString & variableName ="", const QString & itemClassName = "");
    QString getVectorMethodRemove(const QString & variableName ="", const QString & itemClassName = "");
    QString getVectorMethodInit(const QString & variableName ="", const QString & itemClassName = "");

    QString getObjectMethodInit(const QString & variableName ="", const QString & itemClassName = "");

    virtual void setDefaults (CPPCodeGenerationPolicy * cppclone, bool emitUpdateSignal = true);
    virtual void setDefaults(bool emitUpdateSignal = true);

    CodeGenerationPolicyPage * createPage ( QWidget *parent = 0, const char * name = 0);

protected:

    void init();

private:

    QString m_vectorMethodAppendBase;
    QString m_vectorMethodRemoveBase;
    QString m_vectorMethodInitBase;
    QString m_objectMethodInitBase;

};

#endif // CPPCODEGENERATIONPOLICY_H
