#ifndef UMLINSTANCEATTRIBUTEDIALOG_H
#define UMLINSTANCEATTRIBUTEDIALOG_H

#include <QWidget>
#include "singlepagedialogbase.h"
#include "attribute.h"

namespace Ui {
class UMLInstanceAttributeDialog;
}

class UMLInstanceAttributeDialog : public SinglePageDialogBase
{
    Q_OBJECT

public:
    explicit UMLInstanceAttributeDialog(QWidget *parent, UMLAttribute *pAttribute);
    ~UMLInstanceAttributeDialog();

private:
    Ui::UMLInstanceAttributeDialog *ui;
    UMLAttribute *m_pAttribute;

protected:
    bool apply();

public slots:
    void slotNameChanged(const QString &text);
};

#endif // UMLINSTANCEATTRIBUTEDIALOG_H
