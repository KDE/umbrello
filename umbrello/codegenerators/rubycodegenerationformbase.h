/****************************************************************************
** Form interface generated from reading ui file './rubycodegenerationformbase.ui'
**
** Created: Fri Jul 22 06:36:33 2005
**      by: The User Interface Compiler ()
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
class QSpacerItem;
class QGroupBox;
class QCheckBox;
class QComboBox;
class QLabel;

class RubyCodeGenerationFormBase : public QWidget
{
    Q_OBJECT

public:
    RubyCodeGenerationFormBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~RubyCodeGenerationFormBase();

    QGroupBox* groupBox2;
    QCheckBox* m_generateConstructors;
    QCheckBox* m_generateAssocAccessors;
    QCheckBox* m_generateAttribAccessors;
    QComboBox* m_accessorScopeCB;
    QLabel* textLabel1_3;
    QLabel* textLabel1_3_2;
    QComboBox* m_assocFieldScopeCB;
    QGroupBox* groupBox1;
    QLabel* textLabel1;
    QComboBox* m_SelectCommentStyle;
    QLabel* textLabel1_2;

protected:
    QGridLayout* Form1Layout;
    QGridLayout* layout6;
    QGridLayout* groupBox2Layout;
    QGridLayout* layout2;
    QGridLayout* layout4;
    QGridLayout* layout4_2;
    QHBoxLayout* groupBox1Layout;

protected slots:
    virtual void languageChange();

};

#endif // FORM1_H
