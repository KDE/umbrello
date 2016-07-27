#include "umltypemodifierwidget.h"
#include "classifierlistitem.h"
#include "uml.h"

#include <KLocalizedString>

UMLTypeModifierWidget::UMLTypeModifierWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::UMLTypeModifierWidget)
{
    ui->setupUi(this);

    m_texts[Uml::TypeModifiers::None] = i18nc("none", "None");
    m_texts[Uml::TypeModifiers::Pointer] = i18nc("pointer type", "*");
    m_texts[Uml::TypeModifiers::Reference] = i18nc("reference type", "&");

    foreach (const auto &text, m_texts) {
        ui->typeCB->addItem(text);
    }
}

void UMLTypeModifierWidget::apply()
{
    QString currData = ui->typeCB->currentText();
    Uml::TypeModifiers::Enum key = m_texts.key(currData);
    if(m_typeModifier)
        m_typeModifier->setModifier(key);
}

void UMLTypeModifierWidget::setUMLClassifierItem(UMLClassifierListItem *o)
{
    Q_ASSERT(o);
    m_typeModifier = o;
    switch (m_typeModifier->modifier()) {
    case Uml::TypeModifiers::Pointer:
        ui->typeCB->setCurrentIndex(1);
        break;
    case Uml::TypeModifiers::Reference:
        ui->typeCB->setCurrentIndex(2);
    default:
        ui->typeCB->setCurrentIndex(0);
        break;
    }
}
