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
