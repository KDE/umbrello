/*
    Form interface generated from reading ui file 'Defaultcodegenpolicypage.ui'

    Created: Fri Aug 1 12:47:23 2003
    by: The User Interface Compiler ($Id$)

    WARNING! All changes made in this file will be lost!
*/

#ifndef DEFAULTCODEGENPOLICYPAGE_H
#define DEFAULTCODEGENPOLICYPAGE_H

#include "codegenerationpolicypage.h"

class QLabel;
class QWidget;

class DefaultCodeGenPolicyPage : public CodeGenerationPolicyPage
{
    Q_OBJECT
public:
    explicit DefaultCodeGenPolicyPage(QWidget   *parent = nullptr, const char* name = nullptr, CodeGenPolicyExt * policy = nullptr);
    ~DefaultCodeGenPolicyPage();

private:
    QLabel* m_textLabel;

};

#endif // DEFAULTCODEGENPOLICYPAGE_H
