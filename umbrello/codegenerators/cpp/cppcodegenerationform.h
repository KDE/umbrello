/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
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

    bool getPackageIsANamespace();
    bool getVirtualDestructors();
    bool getGenerateEmptyConstructors();
    bool getGenerateAccessorMethods();
    bool getOperationsAreInline();
    bool getAccessorsAreInline();
    bool getAccessorsArePublic();
    bool getGenerateMakefileDocument();
    QString getDocToolTag();

public slots:

    virtual void browseClicked();

private slots:

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

    static Qt::CheckState toCheckState(bool value);

};

#endif

