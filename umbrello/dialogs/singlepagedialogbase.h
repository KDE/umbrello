/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef SINGLEPAGEDIALOGBASE_H
#define SINGLEPAGEDIALOGBASE_H

#include <QtGlobal>

#include <QDialog>
class QAbstractButton;
class QDialogButtonBox;

/**
 * Base class for single page property dialogs
 *
 * @author   Ralf Habacker
 *
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class SinglePageDialogBase : public QDialog
{
    Q_OBJECT
public:
    explicit SinglePageDialogBase(QWidget *parent, bool withApplyButton = false, bool withSearchButton = false);
    virtual ~SinglePageDialogBase();
    virtual bool apply();

    enum ButtonCode { Cancel = 0, Ok = 1, Apply = 2 };
    // keep in sync with MultiPageDialogBase
    void setCaption(const QString &caption);

    QWidget *mainWidget();
    void setMainWidget(QWidget *widget);
    void setButtonText(ButtonCode code, const QString &text);

protected:
    Q_SLOT void slotApply();
    Q_SLOT void slotOk();
    Q_SLOT void slotCancel();
    Q_SLOT void slotClicked(QAbstractButton*);

protected:
    QDialogButtonBox *m_buttonBox;
    QWidget *m_mainWidget;
    void enableButtonOk(bool enable);
    virtual bool validate();
};

#endif
