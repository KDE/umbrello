/****************************************************************************
** Form interface generated from reading ui file 'classattributesbase.ui'
**
** Created: Fri Mar 14 13:34:18 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CLASSATTRIBUTESBASE_H
#define CLASSATTRIBUTESBASE_H

#include <qvariant.h>
#include <qpixmap.h>
#include "dialogpage.h"
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QGroupBox;
class QListView;
class QListViewItem;
class QPushButton;
class QTextEdit;

class ClassAttributesBase : public DialogPage
{
    Q_OBJECT

public:
    ClassAttributesBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ClassAttributesBase();

    QGroupBox* groupBox7;
    QListView* m_attributesList;
    QPushButton* m_upButton;
    QPushButton* m_downButton;
    QPushButton* m_newButton;
    QPushButton* m_propertiesButton;
    QPushButton* m_deleteButton;
    QGroupBox* groupBox6;
    QTextEdit* m_documentation;

public slots:
    virtual void itemSelected(QListViewItem*);
    virtual void createAttribute();
    virtual void editSelected();
    virtual void deleteSelected();
    virtual void moveUp();
    virtual void moveDown();

protected:
    QVBoxLayout* ClassAttributesBaseLayout;
    QVBoxLayout* groupBox7Layout;
    QGridLayout* layout14;
    QVBoxLayout* layout10;
    QHBoxLayout* layout4;
    QVBoxLayout* groupBox6Layout;

protected slots:
    virtual void languageChange();
private:
    QPixmap image0;
    QPixmap image1;

};

#endif // CLASSATTRIBUTESBASE_H
