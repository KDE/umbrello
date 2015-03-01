/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef MULTIPAGEDIALOGBASE_H
#define MULTIPAGEDIALOGBASE_H

#include "icon_utils.h"

#include <kpagewidget.h>

// qt class includes
#include <QDialog>
#include <QWidget>

//forward declarations
class AssociationWidget;
class AssociationGeneralPage;
class AssociationRolePage;
class KFontChooser;
class KPageDialog;
class QFrame;
class UMLWidget;
class UMLWidgetStylePage;
class WidgetBase;

/**
 * Base class for property dialogs
 *
 * MultiPageDialogBase contains common property dialog related methods and attributes
 * In finished state this class provides simple methods to setup common pages
 * like WidgetStyle, FontSetting and others
 *
 * @author   Ralf Habacker
 *
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class MultiPageDialogBase : public QWidget
{
    Q_OBJECT

public:
    explicit MultiPageDialogBase(QWidget *parent, bool withDefaultButton=false);
    virtual ~MultiPageDialogBase();

    void apply();

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
    AssociationGeneralPage *m_pAssocGeneralPage;
    AssociationRolePage *m_pRolePage;
    KFontChooser *m_fontChooser;
    UMLWidgetStylePage *m_pStylePage;
    KPageWidgetItem *m_pageItem;
    KPageDialog *m_pageDialog;
    KPageWidget *m_pageWidget;
    bool m_useDialog;
    bool m_isModified;
    virtual void keyPressEvent(QKeyEvent *event);

    QFrame* createPage(const QString& name, const QString& header, Icon_Utils::IconType icon);
    KPageWidgetItem *createPage(const QString &name, const QString &header, Icon_Utils::IconType icon, QWidget *widget);

    void setupGeneralPage(AssociationWidget *widget);
    void applyGeneralPage(AssociationWidget *widget);

    KPageWidgetItem *setupFontPage(const QFont &font);
    KPageWidgetItem *setupFontPage(UMLWidget *widget);
    KPageWidgetItem *setupFontPage(AssociationWidget *widget);
    void applyFontPage(UMLWidget *widget);
    void applyFontPage(AssociationWidget *widget);

    KPageWidgetItem *setupStylePage(WidgetBase *widget);
    void applyStylePage();

    KPageWidgetItem *setupAssociationRolePage(AssociationWidget *widget);
    void applyAssociationRolePage();

};

#endif
