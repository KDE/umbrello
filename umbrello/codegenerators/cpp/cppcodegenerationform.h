/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef CPPCODEGENERATIONFORM_H
#define CPPCODEGENERATIONFORM_H

#include "ui_cppcodegenerationformbase.h"

class QListWidgetItem;

/**
 * @author Brian Thomas
 */

class CPPCodeGenerationForm : public QWidget, private Ui::CPPCodeGenerationFormBase
{
    Q_OBJECT
public:

    explicit CPPCodeGenerationForm (QWidget *parent = 0, const char *name = 0);
    virtual ~CPPCodeGenerationForm();

    void setPackageIsANamespace(bool bFlag = true);
    void setVirtualDestructors(bool bFlag = true);
    void setGenerateEmptyConstructors(bool bFlag = true);
    void setGenerateAccessorMethods(bool bFlag = true);
    void setOperationsAreInline(bool bFlag = true);
    void setAccessorsAreInline(bool bFlag = true);
    void setAccessorsArePublic(bool bFlag = true);
    void setGenerateMakefileDocument(bool bFlag = true);
    void setDocToolTag(const QString &value);
    void setGetterWithoutGetPrefix(bool bFlag = true);
    void setRemovePrefixFromAccessorMethodName(bool bFlag = true);
    void setAccessorMethodsStartWithUpperCase(bool bFlag);
    void setClassMemberPrefix(const QString &value);

    bool getPackageIsANamespace();
    bool getVirtualDestructors();
    bool getGenerateEmptyConstructors();
    bool getGenerateAccessorMethods();
    bool getOperationsAreInline();
    bool getAccessorsAreInline();
    bool getAccessorsArePublic();
    bool getGenerateMakefileDocument();
    QString getDocToolTag();
    bool getGettersWithGetPrefix();
    bool getRemovePrefixFromAccessorMethodName();
    bool getAccessorMethodsStartWithUpperCase();
    QString getClassMemberPrefix();

public Q_SLOTS:

    virtual void browseClicked();

private Q_SLOTS:

    virtual void generalOptionsListWidgetClicked(QListWidgetItem *);

private:

    friend class CPPCodeGenerationPolicyPage;

    // check boxes for the available options
    QListWidgetItem *m_optionPackageIsANamespace;
    QListWidgetItem *m_optionVirtualDestructors;
    QListWidgetItem *m_optionGenerateEmptyConstructors;
    QListWidgetItem *m_optionGenerateAccessorMethods;
    QListWidgetItem *m_optionOperationsAreInline;
    QListWidgetItem *m_optionAccessorsAreInline;
    QListWidgetItem *m_optionAccessorsArePublic;
    QListWidgetItem *m_optionDocToolTag;
    QListWidgetItem *m_optionGetterWithGetPrefix;
    QListWidgetItem *m_optionRemovePrefixFromAccessorMethodName;
    QListWidgetItem *m_optionAccessorMethodsStartWithUpperCase;

    static Qt::CheckState toCheckState(bool value);

};

#endif

