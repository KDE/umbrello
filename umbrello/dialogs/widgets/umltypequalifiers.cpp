#include "umltypequalifiers.h"

#include <KLocalizedString>

UmlTypeQualifiers::UmlTypeQualifiers(QWidget *parent) : QWidget(parent)
{
    m_texts[Uml::TypeQualifiers::Const] = i18nc("const type", "&Const");
    m_texts[Uml::TypeQualifiers::Volatile] = i18nc("volatile type", "&Volatile");
    m_texts[Uml::TypeQualifiers::Mutable] = i18nc("mutable type", "&Mutable");
    init(i18n("Type Qualifiers"));
}

void UmlTypeQualifiers::addToLayout(QVBoxLayout *layout)
{
    layout->addWidget(m_box);
}

void UmlTypeQualifiers::init(const QString &title)
{
    auto *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    m_box = new QGroupBox(title, this);

    QHBoxLayout* boxlayout = new QHBoxLayout(m_box);
    int margin = fontMetrics().height();
    boxlayout->setMargin(margin);

    QList<Uml::TypeQualifiers::Enum> list;
    list << Uml::TypeQualifiers::Const << Uml::TypeQualifiers::Volatile << Uml::TypeQualifiers::Mutable;

    for(QList<Uml::TypeQualifiers::Enum>::const_iterator i = list.constBegin(); i != list.constEnd(); ++i){
        Uml::TypeQualifiers::Enum key = *i;
        auto *button = new QRadioButton(m_texts[key], m_box);
        m_buttons[key] = button;
        boxlayout->addWidget(button);
    }
    layout->addWidget(m_box);
    setLayout(layout);
}
