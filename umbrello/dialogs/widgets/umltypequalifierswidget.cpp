#include "umltypequalifierswidget.h"
#include "umlobject.h"
#include "uml.h"

#include <KLocalizedString>

UmlTypeQualifiersWidget::UmlTypeQualifiersWidget(UMLObject *o,QWidget *parent) : QWidget(parent)
  , m_object(o)
  , m_role(Uml::RoleType::A)
{
    Q_ASSERT(o);
    m_texts[Uml::TypeQualifiers::Const] = i18nc("const type", "&Const");
    m_texts[Uml::TypeQualifiers::Volatile] = i18nc("volatile type", "&Volatile");
    m_texts[Uml::TypeQualifiers::Mutable] = i18nc("mutable type", "&Mutable");
    init(i18n("Type Qualifiers:"));
}

void UmlTypeQualifiersWidget::addToLayout(QGridLayout *layout, int row)
{
    layout->addWidget(m_label, row, 0);
    layout->addWidget(this, row, 1);
}

void UmlTypeQualifiersWidget::apply()
{
    for(ButtonMap::const_iterator i = m_buttons.constBegin(); i != m_buttons.constEnd(); ++i) {
        if (i.value()->isChecked()) {
            if (m_object)
                m_object->setQualifiers(i.key());
        }
    }
}

void UmlTypeQualifiersWidget::init(const QString &title)
{
    m_box = new QHBoxLayout;
    m_box->setContentsMargins(0,0,0,0);
    m_label = new QLabel(title);

    QList<Uml::TypeQualifiers::Enum> list;
    list << Uml::TypeQualifiers::Const << Uml::TypeQualifiers::Volatile << Uml::TypeQualifiers::Mutable;

    for(QList<Uml::TypeQualifiers::Enum>::const_iterator i = list.constBegin(); i != list.constEnd(); ++i){
        Uml::TypeQualifiers::Enum key = *i;
        auto *button = new QRadioButton(m_texts[key]);
        m_buttons[key] = button;
        m_box->addWidget(button);
    }
    setLayout(m_box);
}
