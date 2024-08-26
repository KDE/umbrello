/*
    SPDX-FileCopyrightText: 2015-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "imagetypewidget.h"

// kde includes
#include <KComboBox>
#include <KLocalizedString>

// qt includes
#include <QHBoxLayout>
#include <QLabel>

/**
 * Constructor
 * @param imageTypes A list of image types the user should be able to select from.
 * @param _default String which is selected by default.
 * @param parent Parent widget
 */
ImageTypeWidget::ImageTypeWidget(const QStringList &imageTypes, const QString &_default, QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    m_label = new QLabel(i18n("&Image type:"), this);
    m_label->setToolTip(i18n("The format that the images will be exported to"));
    layout->addWidget(m_label);

    m_comboBox = new KComboBox(this);
    m_comboBox->addItems(imageTypes);
    m_comboBox->setCurrentText(_default);
    layout->addWidget(m_comboBox, 2);
    m_comboBox->setEditable(false);
    m_label->setBuddy(m_comboBox);
    connect(m_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(slotCurrentIndexChanged(QString)));
    setLayout(layout);
}

/**
 * Return current type as string.
 * @return String with currently selected type.
 */
QString ImageTypeWidget::currentType()
{
    return m_comboBox->currentText();
}
/**
 * Slot to export index changed signal from the combo box.
 * @param index
 */
void ImageTypeWidget::slotCurrentIndexChanged(const QString &index)
{
    Q_EMIT currentIndexChanged(index);
}
