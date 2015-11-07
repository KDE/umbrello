/*
    Copyright 2015 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "imagetypewidget.h"

// kde includes
#include <KComboBox>
#if QT_VERSION < 0x050000
#include <kfilefiltercombo.h>
#endif
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

#if QT_VERSION >= 0x050000
    m_comboBox = new KComboBox(this);
    m_comboBox->addItems(imageTypes);
    m_comboBox->setCurrentText(_default);
#else
    m_comboBox = new KFileFilterCombo(this);
    m_comboBox->setMimeFilter(imageTypes, _default);
#endif
    layout->addWidget(m_comboBox, 2);
    m_comboBox->setEditable(false);
    m_label->setBuddy(m_comboBox);
    connect(m_comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotCurrentIndexChanged(QString)));
    setLayout(layout);
}

/**
 * Return current type as string.
 * @return String with currently selected type.
 */
QString ImageTypeWidget::currentType()
{
#if QT_VERSION >= 0x050000
    return m_comboBox->currentText();
#else
    return m_comboBox->currentFilter();
#endif
}
/**
 * Slot to export index changed signal from the combo box.
 * @param index
 */
void ImageTypeWidget::slotCurrentIndexChanged(const QString &index)
{
    emit currentIndexChanged(index);
}
