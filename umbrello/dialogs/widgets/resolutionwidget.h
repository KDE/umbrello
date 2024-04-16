/*
    SPDX-FileCopyrightText: 2015-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef RESOLUTIONWIDGET_H
#define RESOLUTIONWIDGET_H

#include "comboboxwidgetbase.h"

class KComboBox;
class QHBoxLayout;
class QLabel;

class ResolutionWidget : public ComboBoxWidgetBase
{
    Q_OBJECT
public:
    explicit ResolutionWidget(QWidget *parent = 0);
    float currentResolution();

private Q_SLOTS:
    void slotTextChanged(const QString &text);

private:
    QStringList resolutions();
};

#endif // RESOLUTIONWIDGET_H
