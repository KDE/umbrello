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

// kde includes
#include <QDialog>

class QDialogButtonBox;

/**
 * Base class for single page property dialogs
 *
 * @author   Ralf Habacker
 *
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class SinglePageDialogBase : public QDialog
{
    Q_OBJECT
public:
    explicit SinglePageDialogBase(QWidget *parent);
    virtual ~SinglePageDialogBase();
    virtual bool apply();
    // keep in sync with MultiPageDialogBase
    void setCaption(const QString &caption);

    QWidget *mainWidget();
    void setMainWidget(QWidget *widget);

protected slots:
    void slotApply();
    void slotOk();

protected:
    QDialogButtonBox *m_buttonBox;
    QWidget *m_mainWidget;
};

#endif
