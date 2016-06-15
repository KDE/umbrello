#include "umltypequalifierswidget.h"
#include "classifierlistitem.h"
#include "uml.h"
#include "basictypes.h"

#include <KLocalizedString>

UmlTypeQualifiersWidget::UmlTypeQualifiersWidget(QWidget *parent)
  : QWidget(parent)
  ,ui(new Ui::UMLTypeQualifiersWidget)
{
    ui->setupUi(this);
    //Using some connects with lambda to control the widgets
    connect(ui->rb_none, &QRadioButton::clicked, [=]{
        if(ui->rb_none->isChecked()){
            ui->rb_mutable->setEnabled(true);
            ui->ck_const->setEnabled(true);
            ui->ck_volatile->setEnabled(true);
        }
    });

    connect(ui->rb_mutable, &QRadioButton::clicked, [=]{
        if(ui->rb_mutable->isChecked()){
            ui->ck_const->setDisabled(true);
            ui->ck_volatile->setDisabled(true);
        }
    });
    connect(ui->ck_const, &QCheckBox::clicked, [=]{
        if(ui->ck_const->isChecked() || ui->ck_volatile->isChecked()){
            ui->rb_mutable->setDisabled(true);
            ui->rb_none->setDisabled(true);
            ui->rb_none->setChecked(false);
        }
        else{
            ui->rb_mutable->setDisabled(false);
            ui->rb_none->setDisabled(false);
        }
    });
    connect(ui->ck_volatile, &QCheckBox::clicked, [=]{
       if(ui->ck_volatile->isChecked() || ui->ck_const->isChecked()){
           ui->rb_mutable->setDisabled(true);
           ui->rb_none->setDisabled(true);
           ui->rb_none->setChecked(false);
       }
       else{
           ui->rb_mutable->setDisabled(false);
           ui->rb_none->setDisabled(false);
       }
    });
}

void UmlTypeQualifiersWidget::apply()
{
    if(ui->ck_const->isChecked() && ui->ck_volatile->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::ConstVolatile);
    else if(ui->rb_none->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::None);
    else if(ui->ck_const->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::Const);
    else if(ui->rb_mutable->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::Mutable);
    else if(ui->ck_volatile->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::Volatile);
}

void UmlTypeQualifiersWidget::setUMLClassifierItem(UMLClassifierListItem *o)
{
    Q_ASSERT(o);
    m_qualifier = o;
}
