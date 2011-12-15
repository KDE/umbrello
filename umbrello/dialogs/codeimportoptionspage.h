/**
 * This class is used in the code generation wizard and
 * also in SettingsDlg.
 *
 * @author Ralf Habacker
 */
#ifndef CODEIMPORTOPTIONSPAGE_H
#define CODEIMPORTOPTIONSPAGE_H

#include <QtGui/QWidget>
#include "ui_codeimportoptionspage.h"

class CodeImportOptionsPage : public QWidget, private Ui::CodeImportOptionsPage
{
    Q_OBJECT
public:
    CodeImportOptionsPage(QWidget *parent = 0);
    ~CodeImportOptionsPage();
    void setDefaults();
    void apply();

signals:
     void applyClicked();

protected:
};

#endif
