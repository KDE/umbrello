#include "umlinstanceattributedialog.h"
#include "ui_umlinstanceattributedialog.h"

UMLInstanceAttributeDialog::UMLInstanceAttributeDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UMLInstanceAttributeDialog)
{
    ui->setupUi(this);
}

UMLInstanceAttributeDialog::~UMLInstanceAttributeDialog()
{
    delete ui;
}
