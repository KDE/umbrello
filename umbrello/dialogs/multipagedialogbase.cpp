/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "multipagedialogbase.h"

// local includes
#include "icon_utils.h"
#include "uml.h"
#include "umlwidget.h"
#include "associationgeneralpage.h"
#include "associationrolepage.h"
#include "associationwidget.h"
#include "umlwidgetstylepage.h"

#include <KFontChooser>
#include <KLocale>
#include <KPageDialog>
#include <KPageWidget>

// qt includes
#include <QApplication>
#include <QDockWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>

/**
 * Constructor
 */
MultiPageDialogBase::MultiPageDialogBase(QWidget *parent, bool withDefaultButton)
  : QWidget(parent),
    m_pAssocGeneralPage(0),
    m_pRolePage(0),
    m_fontChooser(0),
    m_pStylePage(0),
    m_pageItem(0),
    m_pageDialog(0),
    m_pageWidget(0),
    m_useDialog(!parent || strcmp(parent->metaObject()->className(),"PropertiesWindow") != 0),
    m_isModified(false)
{
    if (m_useDialog) {
        m_pageDialog = new KPageDialog(parent);
        KDialog::ButtonCodes buttons = KDialog::Ok | KDialog::Apply | KDialog::Cancel | KDialog::Help;
        if (withDefaultButton)
            buttons |=  KDialog::Default;
        m_pageDialog->setButtons(buttons);
        m_pageDialog->setDefaultButton(KDialog::Ok);
        m_pageDialog->showButtonSeparator(true);
        m_pageDialog->setFaceType(KPageDialog::List);
        m_pageDialog->setModal(true);
        m_pageDialog->setHelp(QString::fromLatin1("umbrello/index.html"), QString());
        connect(m_pageDialog, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
        connect(m_pageDialog, SIGNAL(applyClicked()), this, SLOT(slotApplyClicked()));
        connect(m_pageDialog, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));
    } else {
        m_pageWidget = new KPageWidget(this);
        m_pageWidget->setFaceType(KPageView::Tree);
    }
}

MultiPageDialogBase::~MultiPageDialogBase()
{
    delete m_pageDialog;
    delete m_pageWidget;
}

/**
 * Apply all used pages
 */
void MultiPageDialogBase::apply()
{
    if (m_pAssocGeneralPage)
        m_pAssocGeneralPage->apply();

    if (m_pRolePage) {
        applyAssociationRolePage();
    }

    if (m_pStylePage) {
        applyStylePage();
    }

    //TODO include applying font settings data
}

void MultiPageDialogBase::setCaption(const QString &caption)
{
    if (m_pageDialog)
        m_pageDialog->setCaption(caption);
}

void MultiPageDialogBase::accept()
{
    if (m_pageDialog)
        m_pageDialog->accept();
}

void MultiPageDialogBase::reject()
{
    if (m_pageDialog)
        m_pageDialog->reject();
}

KPageWidgetItem *MultiPageDialogBase::currentPage()
{
    if (m_pageDialog)
        return m_pageDialog->currentPage();
    else
        return m_pageWidget->currentPage();
}

void MultiPageDialogBase::addPage(KPageWidgetItem *page)
{
    if (m_pageDialog)
        m_pageDialog->addPage(page);
    else
        m_pageWidget->addPage(page);
}

int MultiPageDialogBase::spacingHint()
{
    return KDialog::spacingHint();
}

int MultiPageDialogBase::exec()
{
    if (m_pageDialog)
        return m_pageDialog->exec();
    else {
        return 0;
    }
}

/**
 * Return state if any data has been changed in the dialog.
 *
 * @return true data has been changed
 */
bool MultiPageDialogBase::isModified()
{
    return m_isModified;
}

/**
 * Handle click on ok button.
 */
void MultiPageDialogBase::slotOkClicked()
{
    emit okClicked();
}

/**
 * Handle click on apply button.
 */
void MultiPageDialogBase::slotApplyClicked()
{
    emit applyClicked();
}

/**
 * Handle click on default button, if enabled in constructor.
 */
void MultiPageDialogBase::slotDefaultClicked()
{
    emit defaultClicked();
}

/**
 * Handle key press event.
 *
 * @param event key press event
 */
void MultiPageDialogBase::keyPressEvent(QKeyEvent *event)
{
    // Set modified state if any text has been typed in
    if (event->key() >= Qt::Key_Space
            && event->key() < Qt::Key_Multi_key)
        m_isModified = true;

    QWidget::keyPressEvent(event);
}

/**
 * Create a property page
 * @param name   The Text displayed in the page list
 * @param header The Text displayed above the page
 * @param icon  The icon to display in the page list
 * @return Pointer to created frame
 */
QFrame* MultiPageDialogBase::createPage(const QString& name, const QString& header, Icon_Utils::IconType icon)
{
    QFrame* page = new QFrame();
    m_pageItem = new KPageWidgetItem(page, name);
    if (!m_pageWidget) {
        m_pageItem->setHeader(header);
        m_pageItem->setIcon(KIcon(Icon_Utils::DesktopIcon(icon)));
    } else
        m_pageItem->setHeader(QString());
    addPage(m_pageItem);
    //page->setMinimumSize(310, 330);
    return page;
}

/**
 * create new page using a dedicated widget
 * @param name   The Text displayed in the page list
 * @param header The Text displayed above the page
 * @param icon  The icon to display in the page list
 * @param widget Widget to display in the page
 * @return page widget item instance
 */
KPageWidgetItem *MultiPageDialogBase::createPage(const QString& name, const QString& header, Icon_Utils::IconType icon, QWidget *widget)
{
    QFrame* page = createPage(name, header, icon);
    QHBoxLayout * topLayout = new QHBoxLayout(page);
    widget->setParent(page);
    topLayout->addWidget(widget);
    return m_pageItem;
}

/**
 * Sets up the general settings page.
 * @param widget The widget to load the initial data from
 */
void MultiPageDialogBase::setupGeneralPage(AssociationWidget *widget)
{
    QFrame *page = createPage(i18nc("general settings", "General"), i18n("General Settings"), Icon_Utils::it_Properties_General);
    QHBoxLayout *layout = new QHBoxLayout(page);
    m_pAssocGeneralPage = new AssociationGeneralPage (page, widget);
    layout->addWidget(m_pAssocGeneralPage);
}

/**
 * updates the general page data
 * @param widget Widget to save the general page data into
 */
void MultiPageDialogBase::applyGeneralPage(AssociationWidget *widget)
{
    Q_UNUSED(widget);
    Q_ASSERT(m_pAssocGeneralPage);
    m_pAssocGeneralPage->apply();
}

/**
 * Sets up the font selection page.
 * @param font The font to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupFontPage(const QFont &font)
{
    QFrame* page = createPage(i18n("Font"), i18n("Font Settings"), Icon_Utils::it_Properties_Font);
    QHBoxLayout * layout = new QHBoxLayout(page);
    m_fontChooser = new KFontChooser((QWidget*)page, KFontChooser::NoDisplayFlags, QStringList(), 0);
    m_fontChooser->setFont(font);
    layout->addWidget(m_fontChooser);
    return m_pageItem;
}

/**
 * Sets up the font selection page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupFontPage(UMLWidget *widget)
{
    return setupFontPage(widget->font());
}

/**
 * Sets up the font selection page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupFontPage(AssociationWidget *widget)
{
    return setupFontPage(widget->font());
}

/**
 * updates the font page data
 * @param widget Widget to save the font data into
 */
void MultiPageDialogBase::applyFontPage(UMLWidget *widget)
{
    Q_UNUSED(widget);
    Q_ASSERT(m_fontChooser);
    widget->setFont(m_fontChooser->font());
}

/**
 * updates the font page data
 * @param widget Widget to save the font data into
 */
void MultiPageDialogBase::applyFontPage(AssociationWidget *widget)
{
    Q_UNUSED(widget);
    Q_ASSERT(m_fontChooser);
    widget->lwSetFont(m_fontChooser->font());
}

/**
 * Sets up the style page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupStylePage(WidgetBase *widget)
{
    QFrame * page;
    if (widget->baseType() == WidgetBase::wt_Association)
        page = createPage(i18nc("style page name", "Style"), i18n("Role Style"), Icon_Utils::it_Properties_Color);
    else
        page = createPage(i18nc("widget style page", "Style"), i18n("Widget Style"), Icon_Utils::it_Properties_Color);
    QHBoxLayout * layout = new QHBoxLayout(page);
    m_pStylePage = new UMLWidgetStylePage(page, widget);
    layout->addWidget(m_pStylePage);
    return m_pageItem;
}

/**
 * Updates the style page.
 */
void MultiPageDialogBase::applyStylePage()
{
    Q_ASSERT(m_pStylePage);
    m_pStylePage->apply();
}

/**
 * Sets up the role settings page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *MultiPageDialogBase::setupAssociationRolePage(AssociationWidget *widget)
{
    QFrame *page = createPage(i18nc("role page name", "Roles"), i18n("Role Settings"), Icon_Utils::it_Properties_Roles);
    QHBoxLayout *layout = new QHBoxLayout(page);
    m_pRolePage = new AssociationRolePage(page, widget);
    layout->addWidget(m_pRolePage);
    return m_pageItem;
}

/**
 * Save all used pages
 */
void MultiPageDialogBase::applyAssociationRolePage()
{
    Q_ASSERT(m_pRolePage);
    m_pRolePage->apply();
}
