/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlartifacttypewidget.h"

#include "umlapp.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>

typedef QMap<UMLArtifact::Draw_Type,QString> Map;

UMLArtifactTypeWidget::UMLArtifactTypeWidget(UMLArtifact *a, QWidget *parent) :
    QWidget(parent),
    m_object(a)
{
    Map texts;

    texts[UMLArtifact::file] = i18n("&File");
    texts[UMLArtifact::library] = i18n("&Library");
    texts[UMLArtifact::table] = i18n("&Table");
    texts[UMLArtifact::defaultDraw] = i18nc("draw as default", "&Default");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    m_box = new QGroupBox(i18n("Draw As"), this);
    QHBoxLayout* drawAsLayout = new QHBoxLayout(m_box );
    int margin = fontMetrics().height();
    drawAsLayout->setContentsMargins(margin, margin, margin, margin);

    for(Map::const_iterator i = texts.constBegin(); i != texts.constEnd(); ++i) {
        QRadioButton *button = new QRadioButton(i.value(), m_box );
        m_buttons[i.key()] = button;
        drawAsLayout->addWidget(button);
    }
    m_buttons[a->getDrawAsType()]->setChecked(true);
    layout->addWidget(m_box);
    setLayout(layout);
}

UMLArtifactTypeWidget::~UMLArtifactTypeWidget()
{
    delete m_box;
}

/**
 * Add this widget to a given layout.
 * @param layout The layout to which the widget should be added
 */
void UMLArtifactTypeWidget::addToLayout(QVBoxLayout *layout)
{
    layout->addWidget(m_box);
}

/**
 * Apply changes to the related UMLObject.
 */
void UMLArtifactTypeWidget::apply()
{
    for(ButtonMap::const_iterator i = m_buttons.constBegin(); i != m_buttons.constEnd(); ++i) {
        if (i.value()->isChecked())
            m_object->setDrawAsType(i.key());
    }
}
