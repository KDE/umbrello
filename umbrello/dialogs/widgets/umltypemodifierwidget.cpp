#include "umltypemodifierwidget.h"
#include "classifierlistitem.h"
#include "uml.h"

#include <KLocalizedString>

UMLTypeModifierWidget::UMLTypeModifierWidget(QWidget *parent) : QWidget(parent)
{
    m_texts[Uml::TypeModifiers::None] = i18nc("none", "None");
    m_texts[Uml::TypeModifiers::Pointer] = i18nc("pointer type", "*");
    m_texts[Uml::TypeModifiers::Reference] = i18nc("reference type", "&");
    init(i18n("Type Modifiers:"));

}

void UMLTypeModifierWidget::apply()
{
    QString currData = m_typeCB->currentText();
    Uml::TypeModifiers::Enum key = m_texts.key(currData);
    if(m_typeModifier)
        m_typeModifier->setModifier(key);
}

void UMLTypeModifierWidget::setUMLClassifierItem(UMLClassifierListItem *o)
{
    Q_ASSERT(o);
    m_typeModifier = o;
}

void UMLTypeModifierWidget::init(const QString &title)
{
    auto layout = new QHBoxLayout;
    auto label = new QLabel(title);
    layout->addWidget(label);
    m_typeCB = new QComboBox();

    foreach (const auto &text, m_texts) {
        m_typeCB->addItem(text);
    }

    layout->addWidget(m_typeCB);
    setLayout(layout);
}
