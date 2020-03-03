/*
    Copyright (C) 2015-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

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

#include "resolutionwidget.h"

// kde includes
#include <KComboBox>
#include <KLocalizedString>

// qt includes
#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QLabel>

/**
 * Constructor
 * @param parent QWidget parent
 */
ResolutionWidget::ResolutionWidget(QWidget *parent) :
    ComboBoxWidgetBase(i18n("&Resolution:"), i18n("DPI"), parent)
{
    m_editField->clear();
    m_editField->addItem(QLatin1String("default"), QVariant(0.0));
    foreach(const QString &key, resolutions()) {
        m_editField->addItem(key, QVariant(key.toFloat()));
    }
    connect(m_editField, SIGNAL(editTextChanged(QString)), this, SLOT(slotTextChanged(QString)));
}

/**
 * Return current selected resolution.
 * @return resolutions as QString
 */
float ResolutionWidget::currentResolution()
{
    QVariant v = m_editField->itemData(m_editField->currentIndex());
    if (v.canConvert<float>()) {
        return v.value<float>();
    } else {
        bool ok;
        float value = m_editField->currentText().toFloat(&ok);
        return ok ? value : 0.0;
    }
}

bool numberLessThan(const QString &s1, const QString &s2)
{
    return s1.toFloat() < s2.toFloat();
}

/**
 * Returns a QStringList containing all supported resolutions
 * @return QStringList with resolutions
 */
QStringList ResolutionWidget::resolutions()
{
    QStringList result;
    result << QLatin1String("72");
    result << QLatin1String("96");
    result << QLatin1String("150");
    result << QLatin1String("300");
    result << QLatin1String("600");
    result << QLatin1String("1200");

    QString currentResolution = QString::number(qApp->desktop()->logicalDpiX());
    if (!result.contains(currentResolution))
        result << currentResolution;
    qSort(result.begin(), result.end(), numberLessThan);
    return result;
}

/**
 * Limit user to only be able to enter numbers.
 * @param text Currently edited text
 */
void ResolutionWidget::slotTextChanged(const QString &text)
{
    if (m_editField->currentText() == QLatin1String("default"))
        return;
    bool ok;
    text.toFloat(&ok);
    if (!ok)
        m_editField->setEditText(QLatin1String(""));
}
