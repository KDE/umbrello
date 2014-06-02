/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "dialogbase.h"

// local includes
#include "icon_utils.h"
#include "uml.h"
#include "umlwidget.h"
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
DialogBase::DialogBase(QWidget *parent, bool withDefaultButton)
  : QWidget(parent),
    m_pageDialog(0),
    m_pageWidget(0),
    m_useDialog(strcmp(parent->metaObject()->className(),"PropertiesWindow") != 0),
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
        m_pageDialog->setHelp("umbrello/index.html", QString());
        connect(m_pageDialog, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
        connect(m_pageDialog, SIGNAL(applyClicked()), this, SLOT(slotApplyClicked()));
        connect(m_pageDialog, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));
    } else {
        m_pageWidget = new KPageWidget(this);
        m_pageWidget->setFaceType(KPageView::Tree);
    }
}

DialogBase::~DialogBase()
{
    delete m_pageDialog;
    delete m_pageWidget;
}

/**
 * Create a property page
 * @param name   The Text displayed in the page list
 * @param header The Text displayed above the page
 * @param icon  The icon to display in the page list
 * @return Pointer to created frame
 */
QFrame* DialogBase::createPage(const QString& name, const QString& header, Icon_Utils::IconType icon)
{
    QFrame* page = new QFrame();
    m_pageItem = new KPageWidgetItem(page, name);
    if (!m_pageWidget) {
        m_pageItem->setHeader(header);
        m_pageItem->setIcon(Icon_Utils::DesktopIcon(icon));
    } else
        m_pageItem->setHeader("");
    addPage(m_pageItem);
    //page->setMinimumSize(310, 330);
    return page;
}

/**
 * Sets up the font selection page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *DialogBase::setupFontPage(UMLWidget *widget)
{
    QFrame* page = createPage(i18n("Font"), i18n("Font Settings"), Icon_Utils::it_Properties_Font);
    QHBoxLayout * m_pStyleLayout = new QHBoxLayout(page);
    m_fontChooser = new KFontChooser((QWidget*)page, KFontChooser::NoDisplayFlags, QStringList(), 0);
    m_fontChooser->setFont(widget->font());
    m_pStyleLayout->addWidget(m_fontChooser);
    return m_pageItem;
}

/**
 * updates the font page data
 * @param widget Widget to save the font data into
 */
void DialogBase::saveFontPageData(UMLWidget *widget)
{
    widget->setFont(m_fontChooser->font());
}

/**
 * Sets up the style page.
 * @param widget The widget to load the initial data from
 */
KPageWidgetItem *DialogBase::setupStylePage(UMLWidget *widget)
{
    QFrame * page = createPage(i18nc("widget style page", "Style"), i18n("Widget Style"), Icon_Utils::it_Properties_Color);
    QHBoxLayout * m_pStyleLayout = new QHBoxLayout(page);
    m_pStylePage = new UMLWidgetStylePage(page, widget);
    m_pStyleLayout->addWidget(m_pStylePage);
    return m_pageItem;
}

/**
 * updates the font page data
 * @param widget Widget to save the font data into
 */
void DialogBase::saveStylePageData(UMLWidget *widget)
{
    Q_UNUSED(widget);
    m_pStylePage->updateUMLWidget();
}

void DialogBase::setCaption(const QString &caption)
{
    if (m_pageDialog)
        m_pageDialog->setCaption(caption);
}

void DialogBase::accept()
{
    if (m_pageDialog)
        m_pageDialog->accept();
}

void DialogBase::reject()
{
    if (m_pageDialog)
        m_pageDialog->reject();
}

KPageWidgetItem *DialogBase::currentPage()
{
    if (m_pageDialog)
        return m_pageDialog->currentPage();
    else
        return m_pageWidget->currentPage();
}

void DialogBase::addPage(KPageWidgetItem *page)
{
    if (m_pageDialog)
        m_pageDialog->addPage(page);
    else
        m_pageWidget->addPage(page);
}

int DialogBase::spacingHint()
{
    return KDialog::spacingHint();
}

int DialogBase::exec()
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
bool DialogBase::isModified()
{
    return m_isModified;
}

/**
 * Handle click on ok button.
 */
void DialogBase::slotOkClicked()
{
    emit okClicked();
}

/**
 * Handle click on apply button.
 */
void DialogBase::slotApplyClicked()
{
    emit applyClicked();
}

/**
 * Handle click on default button, if enabled in constructor.
 */
void DialogBase::slotDefaultClicked()
{
    emit defaultClicked();
}

/**
 * Handle key press event.
 *
 * @param event key press event
 */
void DialogBase::keyPressEvent(QKeyEvent *event)
{
    // Set modified state if any text has been typed in
    if (event->key() >= Qt::Key_Space
            && event->key() < Qt::Key_Multi_key)
        m_isModified = true;

    QWidget::keyPressEvent(event);
}
