#include "umltypemodifiers.h"

#include <KLocalizedString>

UmlTypeModifiers::UmlTypeModifiers(QWidget *parent) : QWidget(parent)
{
    m_texts[Uml::TypeModifiers::Const] = i18nc("const type", "&Const");
    m_texts[Uml::TypeModifiers::Volatile] = i18nc("volatile type", "&Volatile");
    m_texts[Uml::TypeModifiers::Mutable] = i18nc("mutable type", "&Mutable");
    init(i18n("Type Modifiers"));
}

void UmlTypeModifiers::addToLayout(QVBoxLayout *layout)
{
    layout->addWidget(m_box);
}

void UmlTypeModifiers::init(const QString &title)
{
    auto *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    m_box = new QGroupBox(title, this);

    QHBoxLayout* boxlayout = new QHBoxLayout(m_box);
    int margin = fontMetrics().height();
    boxlayout->setMargin(margin);

    QList<Uml::TypeModifiers::Enum> list;
    list << Uml::TypeModifiers::Const << Uml::TypeModifiers::Volatile << Uml::TypeModifiers::Mutable;

    for(QList<Uml::TypeModifiers::Enum>::const_iterator i = list.constBegin(); i != list.constEnd(); ++i){
        Uml::TypeModifiers::Enum key = *i;
        auto *button = new QRadioButton(m_texts[key], m_box);
        m_buttons[key] = button;
        boxlayout->addWidget(button);
    }
    layout->addWidget(m_box);
    setLayout(layout);
}
