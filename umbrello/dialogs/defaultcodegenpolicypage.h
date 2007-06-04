
/****************************************************************************
** Form interface generated from reading ui file 'Defaultcodegenpolicypage.ui'
**
** Created: Fri Aug 1 12:47:23 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef DEFAULTCODEGENPOLICYPAGE_H
#define DEFAULTCODEGENPOLICYPAGE_H

#include <qvariant.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include "codegenerationpolicypage.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QLabel;

class DefaultCodeGenPolicyPage : public CodeGenerationPolicyPage
{
    Q_OBJECT

public:

    DefaultCodeGenPolicyPage( QWidget* parent = 0, const char* name = 0, CodeGenPolicyExt * policy =0);

    ~DefaultCodeGenPolicyPage();

    QLabel* textLabel;

protected:

protected slots:

};

#endif // DEFAULTCODEGENPOLICYPAGE_H
