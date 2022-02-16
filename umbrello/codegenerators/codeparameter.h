/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEPARAMETER_H
#define CODEPARAMETER_H

#include "basictypes.h"
#include "codecomment.h"

class ClassifierCodeDocument;
class UMLObject;
class QXmlStreamWriter;

/**
 * class CodeParameter
 * A parameter on some type of code.
 */
class CodeParameter : public QObject
{
    Q_OBJECT
public:

    CodeParameter(ClassifierCodeDocument* doc, UMLObject* parentObj);
    virtual ~CodeParameter();

    ClassifierCodeDocument* getParentDocument();

    UMLObject* getParentObject();

    bool getAbstract();

    bool getStatic();

    QString getName() const;

    virtual QString getTypeName();

    Uml::Visibility::Enum getVisibility() const;

    virtual void setInitialValue(const QString &new_var);
    virtual QString getInitialValue();

    void setComment(CodeComment* comment);
    CodeComment* getComment();

    // the id of this parameter is the same as the parent UMLObject id.
    QString ID() const;

    virtual void updateContent() = 0;

protected:

    virtual void setAttributesOnNode(QXmlStreamWriter& writer);
    virtual void setAttributesFromNode(QDomElement& element);

private:

    ClassifierCodeDocument* m_parentDocument;
    UMLObject*              m_parentObject;
    CodeComment*            m_comment;
    QString                 m_initialValue;  ///< initial value of this code parameter

    void initFields(ClassifierCodeDocument* doc, UMLObject* obj);

public slots:

    void syncToParent();

};

#endif // CODEPARAMETER_H
