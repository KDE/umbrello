/****************************************************************************
** Form interface generated from reading ui file 'classoperationsbase.ui'
**
** Created: Mon Mar 10 23:43:51 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CLASSOPERATIONSBASE_H
#define CLASSOPERATIONSBASE_H

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

class ClassOperationsBase : public DialogPage
{
    Q_OBJECT

public:
    ClassOperationsBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ClassOperationsBase();

    QGroupBox* groupBox7;
    QListView* m_operationsList;
    QPushButton* m_upButton;
    QPushButton* m_downButton;
    QPushButton* m_newButton;
    QPushButton* m_propertiesButton;
    QPushButton* m_deleteButton;
    QGroupBox* groupBox6;
    QTextEdit* m_documentation;

public slots:
    virtual void itemSelected(QListViewItem*);
    virtual void createOperation();
    virtual void editSelected();
    virtual void deleteSelected();
    virtual void moveUp();
    virtual void moveDown();

protected:
    QVBoxLayout* ClassOperationsBaseLayout;
    QVBoxLayout* groupBox7Layout;
    QGridLayout* layout14;
    QVBoxLayout* layout10;
    QHBoxLayout* layout13;
    QVBoxLayout* groupBox6Layout;

protected slots:
    virtual void languageChange();
private:
    QPixmap image0;
    QPixmap image1;

};

#endif // CLASSOPERATIONSBASE_H
