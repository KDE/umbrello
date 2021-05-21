/*
    SPDX-FileCopyrightText: 2015-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IMAGETYPEWIDGET_H
#define IMAGETYPEWIDGET_H

#include <QWidget>

// KDE forwards
#if QT_VERSION >= 0x050000
class KComboBox;
#else
class KFileFilterCombo;
#endif

// Qt forwards
class QHBoxLayout;
class QLabel;

class ImageTypeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageTypeWidget(const QStringList &imageTypes, const QString &_default, QWidget *parent = 0);
    QString currentType();

private:
    QLabel *m_label;
#if QT_VERSION >= 0x050000
    KComboBox* m_comboBox;  ///< The image type selected.
#else
    KFileFilterCombo* m_comboBox;  ///< The image type selected.
#endif

signals:
    void currentIndexChanged(const QString &index);

private slots:
    void slotCurrentIndexChanged(const QString &index);
};

#endif // IMAGETYPEWIDGET_H
