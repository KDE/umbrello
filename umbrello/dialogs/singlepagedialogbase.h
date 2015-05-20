/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2015                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef SINGLEPAGEDIALOGBASE_H
#define SINGLEPAGEDIALOGBASE_H

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QDialog>
class QDialogButtonBox;
#else
#include <KDialog>
#endif

/**
 * Base class for single page property dialogs
 *
 * @author   Ralf Habacker
 *
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
#if QT_VERSION >= 0x050000
class SinglePageDialogBase : public QDialog
#else
class SinglePageDialogBase : public KDialog
#endif
{
    Q_OBJECT

public:
    explicit SinglePageDialogBase(QWidget *parent, bool withApplyButton = false);
    virtual ~SinglePageDialogBase();
    virtual bool apply();

#if QT_VERSION >= 0x050000
    // keep in sync with MultiPageDialogBase
    void setCaption(const QString &caption);

    QWidget *mainWidget();
    void setMainWidget(QWidget *widget);
#endif

protected slots:
    void slotApply();
    void slotOk();

protected:
#if QT_VERSION >= 0x050000
    QDialogButtonBox *m_buttonBox;
    QWidget *m_mainWidget;
    void enableButtonOk(bool enable);
#endif
};

#endif
