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
