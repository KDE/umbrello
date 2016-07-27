#include "umltypequalifierswidget.h"
#include "classifierlistitem.h"
#include "uml.h"
#include "basictypes.h"

#include <KLocalizedString>

UMLTypeQualifiersWidget::UMLTypeQualifiersWidget(QWidget *parent)
  : QWidget(parent)
  ,ui(new Ui::UMLTypeQualifiersWidget)
{
    ui->setupUi(this);
    //Using some connects with lambda to control the widgets
    connect(ui->noneRB, &QRadioButton::clicked, [=]{
        if(ui->noneRB->isChecked()){
            ui->mutableRB->setEnabled(true);
            ui->constCB->setEnabled(true);
            ui->volatileCB->setEnabled(true);
        }
    });

    connect(ui->mutableRB, &QRadioButton::clicked, [=]{
        if(ui->mutableRB->isChecked()){
            ui->constCB->setDisabled(true);
            ui->volatileCB->setDisabled(true);
        }
    });
    connect(ui->constCB, &QCheckBox::clicked, [=]{
        if(ui->constCB->isChecked() || ui->volatileCB->isChecked()){
            ui->mutableRB->setDisabled(true);
            ui->noneRB->setDisabled(true);
            ui->noneRB->setChecked(false);
        }
        else{
            ui->mutableRB->setDisabled(false);
            ui->noneRB->setDisabled(false);
        }
    });
    connect(ui->volatileCB, &QCheckBox::clicked, [=]{
       if(ui->volatileCB->isChecked() || ui->constCB->isChecked()){
           ui->mutableRB->setDisabled(true);
           ui->noneRB->setDisabled(true);
           ui->noneRB->setChecked(false);
       }
       else{
           ui->mutableRB->setDisabled(false);
           ui->noneRB->setDisabled(false);
       }
    });
}

void UMLTypeQualifiersWidget::apply()
{
    if(ui->constCB->isChecked() && ui->volatileCB->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::ConstVolatile);
    else if(ui->noneRB->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::None);
    else if(ui->constCB->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::Const);
    else if(ui->mutableRB->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::Mutable);
    else if(ui->volatileCB->isChecked())
        m_qualifier->setQualifier(Uml::TypeQualifiers::Volatile);
}

void UMLTypeQualifiersWidget::setUMLClassifierItem(UMLClassifierListItem *o)
{
    Q_ASSERT(o);
    m_qualifier = o;
    switch (m_qualifier->qualifier()) {
    case Uml::TypeQualifiers::ConstVolatile:
            ui->constCB->setChecked(true);
            ui->volatileCB->setChecked(true);
        break;
    case Uml::TypeQualifiers::Const:
        ui->constCB->setChecked(true);
        break;
    case Uml::TypeQualifiers::Mutable:
        ui->mutableRB->setChecked(true);
        break;
    case Uml::TypeQualifiers::Volatile:
        ui->volatileCB->setChecked(true);
        break;
    default:
        ui->noneRB->setChecked(true);
        break;
    }
}
