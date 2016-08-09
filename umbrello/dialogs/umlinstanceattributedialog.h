#ifndef UMLINSTANCEATTRIBUTEDIALOG_H
#define UMLINSTANCEATTRIBUTEDIALOG_H

#include <QWidget>

namespace Ui {
class UMLInstanceAttributeDialog;
}

class UMLInstanceAttributeDialog : public QWidget
{
    Q_OBJECT

public:
    explicit UMLInstanceAttributeDialog(QWidget *parent = 0);
    ~UMLInstanceAttributeDialog();

private:
    Ui::UMLInstanceAttributeDialog *ui;
};

#endif // UMLINSTANCEATTRIBUTEDIALOG_H
