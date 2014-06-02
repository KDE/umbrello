/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DIALOGBASE_H
#define DIALOGBASE_H

#include "icon_utils.h"

#include <kpagewidget.h>

// qt class includes
#include <QDialog>
#include <QWidget>

//forward declarations
class KFontChooser;
class KPageDialog;
class QFrame;
class UMLWidget;
class UMLWidgetStylePage;

/**
 * Base class for property dialogs
 *
 * DialogBase contains common property dialog related methods and attributes
 * In finished state this class provides simple methods to setup common pages
 * like WidgetStyle, FontSetting and others
 *
 * @author   Ralf Habacker
 *
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class DialogBase : public QWidget
{
    Q_OBJECT

public:
    explicit DialogBase(QWidget *parent, bool withDefaultButton=false);
    virtual ~DialogBase();
    QFrame* createPage(const QString& name, const QString& header, Icon_Utils::IconType icon);
    KPageWidgetItem *setupFontPage(UMLWidget *widget);
    void saveFontPageData(UMLWidget *widget);

    KPageWidgetItem *setupStylePage(UMLWidget *widget);
    void saveStylePageData(UMLWidget *widget);

    void setCaption(const QString &caption);
    void accept();
    void reject();
    KPageWidgetItem *currentPage();
    void addPage(KPageWidgetItem *page);
    static int spacingHint();
    int exec();

    virtual bool isModified();

signals:
    void okClicked();
    void applyClicked();
    void defaultClicked();

private slots:
    void slotOkClicked();
    void slotApplyClicked();
    void slotDefaultClicked();

protected:
    KFontChooser *m_fontChooser;
    UMLWidgetStylePage *m_pStylePage;
    KPageWidgetItem *m_pageItem;
    KPageDialog *m_pageDialog;
    KPageWidget *m_pageWidget;
    bool m_useDialog;
    bool m_isModified;
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif
