/*
    SPDX-FileCopyrightText: 2015-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    m_editField->addItem(QStringLiteral("default"), QVariant(0.0));
    Q_FOREACH(const QString &key, resolutions()) {
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
    result << QStringLiteral("72");
    result << QStringLiteral("96");
    result << QStringLiteral("150");
    result << QStringLiteral("300");
    result << QStringLiteral("600");
    result << QStringLiteral("1200");

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
    if (m_editField->currentText() == QStringLiteral("default"))
        return;
    bool ok;
    text.toFloat(&ok);
    if (!ok)
        m_editField->setEditText(QStringLiteral(""));
}
