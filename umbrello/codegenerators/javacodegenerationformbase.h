/****************************************************************************
** Form interface generated from reading ui file './javacodegenerationformbase.ui'
**
** Created: Sun Aug 31 16:18:14 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef FORM1_H
#define FORM1_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;

class JavaCodeGenerationFormBase : public QWidget
{
    Q_OBJECT

public:
    JavaCodeGenerationFormBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~JavaCodeGenerationFormBase();

    QLabel* textLabel1_2;
    QGroupBox* groupBox1;
    QLabel* textLabel1;
    QComboBox* m_SelectCommentStyle;
    QGroupBox* groupBox2;
    QCheckBox* m_generateConstructors;
    QCheckBox* m_generateAccessors;
    QGroupBox* groupBox3;
    QCheckBox* m_makeANTDocumentCheckBox;

protected:
    QVBoxLayout* layout2;
    QHBoxLayout* groupBox1Layout;
    QVBoxLayout* groupBox2Layout;
    QVBoxLayout* groupBox3Layout;

protected slots:
    virtual void languageChange();
};

#endif // FORM1_H
