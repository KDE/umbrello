#include "umltypequalifierswidget.h"
#include "classifierlistitem.h"
#include "uml.h"

#include <KLocalizedString>

UmlTypeQualifiersWidget::UmlTypeQualifiersWidget(QWidget *parent)
  : QWidget(parent)
{
    m_texts[Uml::TypeQualifiers::Const] = i18nc("const type", "&Const");
    m_texts[Uml::TypeQualifiers::Volatile] = i18nc("volatile type", "&Volatile");
    m_texts[Uml::TypeQualifiers::Mutable] = i18nc("mutable type", "&Mutable");
    m_texts[Uml::TypeQualifiers::ConstVolatile] = i18nc("const volatile type", "&Const Volatile");
    m_texts[Uml::TypeQualifiers::None] = i18nc("none", "&None");
    init(i18n("Type Qualifiers:"));

}

void UmlTypeQualifiersWidget::apply()
{
    for(ButtonMap::const_iterator i = m_buttons.constBegin(); i != m_buttons.constEnd(); ++i) {
        if (i.value()->isChecked()) {
            if (m_qualifier)
                m_qualifier->setQualifier(i.key());
        }
    }
}

void UmlTypeQualifiersWidget::setUMLClassifierItem(UMLClassifierListItem *o)
{
    Q_ASSERT(o);
    m_qualifier = o;
}

void UmlTypeQualifiersWidget::init(const QString &title)
{
    m_box = new QGridLayout;
    m_box->setContentsMargins(0,0,0,0);
    m_label = new QLabel(title);
    m_box->addWidget(m_label,0,0);

    QList<Uml::TypeQualifiers::Enum> list;
    list << Uml::TypeQualifiers::None << Uml::TypeQualifiers::Const << Uml::TypeQualifiers::Volatile << Uml::TypeQualifiers::Mutable << Uml::TypeQualifiers::ConstVolatile;
    int cont=0;
    for(QList<Uml::TypeQualifiers::Enum>::const_iterator i = list.constBegin(); i != list.constEnd(); ++i, ++cont){
        Uml::TypeQualifiers::Enum key = *i;
        auto *button = new QRadioButton(m_texts[key]);
        m_buttons[key] = button;
        m_box->addWidget(button,1,cont);
    }
    setLayout(m_box);
}
