/****************************************************************************
** Form interface generated from reading ui file 'classpropertiesbase.ui'
**
** Created: Mon Mar 10 23:43:50 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CLASSPROPERTIESBASE_H
#define CLASSPROPERTIESBASE_H

#include <qvariant.h>
#include "dialogpage.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QTextEdit;

class ClassPropertiesBase : public DialogPage
{
    Q_OBJECT

public:
    ClassPropertiesBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ClassPropertiesBase();

    QLineEdit* m_packageName;
    QLabel* textLabel3;
    QLineEdit* m_stereotype;
    QLabel* textLabel2;
    QLabel* textLabel1;
    QLineEdit* m_className;
    QCheckBox* m_abstract;
    QGroupBox* groupBox3;
    QRadioButton* m_public;
    QRadioButton* m_protected;
    QRadioButton* m_private;
    QGroupBox* groupBox2;
    QTextEdit* m_documentation;

protected:
    QVBoxLayout* ClassPropertiesBaseLayout;
    QGridLayout* layout4;
    QHBoxLayout* layout6;
    QHBoxLayout* groupBox3Layout;
    QVBoxLayout* groupBox2Layout;

protected slots:
    virtual void languageChange();
};

#endif // CLASSPROPERTIESBASE_H
