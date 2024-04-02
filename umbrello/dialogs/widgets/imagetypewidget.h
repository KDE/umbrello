/*
    SPDX-FileCopyrightText: 2015-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IMAGETYPEWIDGET_H
#define IMAGETYPEWIDGET_H

#include <QWidget>

// KDE forwards
class KComboBox;

// Qt forwards
class QHBoxLayout;
class QLabel;

class ImageTypeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageTypeWidget(const QStringList &imageTypes, const QString &_default, QWidget *parent = nullptr);
    QString currentType();

private:
    QLabel *m_label;
    KComboBox* m_comboBox;  ///< The image type selected.

protected:
    Q_SIGNAL void currentIndexChanged(const QString &index);

private:
    Q_SLOT void slotCurrentIndexChanged(const QString &index);
};

#endif // IMAGETYPEWIDGET_H
