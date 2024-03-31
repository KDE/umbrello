/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
class QAbstractButton;
class QFrame;
class NotePage;
class NoteWidget;
class MessageWidget;
class UMLWidget;
class SelectOperationPage;
class UMLWidgetStylePage;
class WidgetBase;
class UMLScene;
namespace Settings { class OptionState; }

/**
 * Base class for property dialogs
 *
 * MultiPageDialogBase contains common property dialog related methods and attributes
 * In finished state this class provides simple methods to setup common pages
 * like WidgetStyle, FontSetting and others
 *
 * @author   Ralf Habacker
 *
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class MultiPageDialogBase : public QWidget
{
    Q_OBJECT

public:
    /// Available page types
    enum PageType {
        AutoLayoutPage,
        ClassPage,
        CodeImportPage,
        CodeGenerationPage,
        CodeViewerPage,
        FontPage,
        GeneralPage,
        UserInterfacePage,
    };

    explicit MultiPageDialogBase(QWidget *parent, bool withDefaultButton=false);
    virtual ~MultiPageDialogBase();

    void apply();

    void setCaption(const QString &caption);
    void accept();
    void reject();
    KPageWidgetItem *currentPage() const;
    void addPage(KPageWidgetItem *page);
    void setCurrentPage(KPageWidgetItem *page);
    int exec();

    virtual bool isModified() const;

    Q_SIGNAL void okClicked();
    Q_SIGNAL void applyClicked();
    Q_SIGNAL void defaultClicked();

protected:
    Q_SLOT void slotEnableButtonOk(bool state);

private:
    Q_SLOT void slotOkClicked();
    Q_SLOT void slotApplyClicked();
    Q_SLOT void slotDefaultClicked();
    Q_SLOT void slotHelpClicked();
    Q_SLOT void slotButtonClicked(QAbstractButton *button);
protected:
    AssociationGeneralPage *m_pAssocGeneralPage;
    NotePage *m_notePage;
    SelectOperationPage *m_operationGeneralPage;
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

    void setupGeneralPage(NoteWidget *widget);
    KPageWidgetItem *setupGeneralPage(MessageWidget *widget);
    void applyGeneralPage(MessageWidget *widget);

    KPageWidgetItem *setupFontPage(const QFont &font);
    KPageWidgetItem *setupFontPage(UMLWidget *widget);
    KPageWidgetItem *setupFontPage(AssociationWidget *widget);
    void resetFontPage(QWidget *widget);
    void applyFontPage(AssociationWidget *widget);
    void applyFontPage(Settings::OptionState *state);
    void applyFontPage(UMLScene *scene);
    void applyFontPage(UMLWidget *widget);

    KPageWidgetItem *setupStylePage(WidgetBase *widget);
    KPageWidgetItem *setupStylePage(AssociationWidget *widget);
    void applyStylePage();

    KPageWidgetItem *setupAssociationRolePage(AssociationWidget *widget);
    void applyAssociationRolePage();
};

#endif
