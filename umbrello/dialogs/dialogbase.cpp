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
#include "dialogbase.h"

// local includes
#include "debug_utils.h"
#include "icon_utils.h"
#include "uml.h"
#include "umlwidget.h"
#include "umlwidgetstylepage.h"

#include <KFontChooser>
#include <KHelpClient>
#include <KLocalizedString>
#include <KPageDialog>
#include <KPageWidget>

// qt includes
#include <QAbstractButton>
#include <QApplication>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>

DEBUG_REGISTER(DialogBase)

/**
 * Constructor
 */
DialogBase::DialogBase(QWidget *parent, bool withDefaultButton)
  : QWidget(parent),
    m_fontChooser(0),
    m_pStylePage(0),
    m_pageItem(0),
    m_pageDialog(0),
    m_pageWidget(0),
    m_isModified(false),
    m_fontChooser(0),
    m_pStylePage(0),
    m_pageItem(0)
{
    bool useDialog = false;
    if (parent) {
        useDialog = (strcmp(parent->metaObject()->className(),"PropertiesWindow") != 0);
    }
    if (useDialog) {
        m_pageDialog = new KPageDialog(parent);
        m_pageDialog->setModal(true);
        m_pageDialog->setFaceType(KPageDialog::List);
        m_pageDialog->setStandardButtons(QDialogButtonBox::Ok |
                                         QDialogButtonBox::Apply |
                                         QDialogButtonBox::Cancel |
                                         QDialogButtonBox::Help);
        QDialogButtonBox * dlgButtonBox = m_pageDialog->findChild<QDialogButtonBox*>(QLatin1String("buttonbox"));
        if (withDefaultButton) {
            QPushButton *defaultButton = new QPushButton(i18n("Default"));
            m_pageDialog->addActionButton(defaultButton);
            connect(defaultButton, SIGNAL(clicked()), this, SLOT(slotDefaultClicked()));
        }
        connect(dlgButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotButtonClicked(QAbstractButton*)));
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
        m_pageItem->setHeader(QString());
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
    Q_UNUSED(widget);
    Q_ASSERT(m_fontChooser);
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
    Q_ASSERT(m_pStylePage);
    m_pStylePage->updateUMLWidget();
}

void DialogBase::setCaption(const QString &caption)
{
    if (m_pageDialog) {
        m_pageDialog->setWindowTitle(caption);
    }
}

void DialogBase::accept()
{
    if (m_pageDialog) {
        m_pageDialog->accept();
    }
}

void DialogBase::reject()
{
    if (m_pageDialog) {
        m_pageDialog->reject();
    }
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
    return 0;  // was QDialog::spacingHint();
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
 * Launch khelpcenter.
 */
void DialogBase::slotHelpClicked()
{
    DEBUG(DBG_SRC)  << "HELP clicked...directly handled";
//    QUrl url = QUrl(QLatin1String("help:/umbrello/index.html"));
//    QDesktopServices::openUrl(url);
    KHelpClient::invokeHelp(QLatin1String("help:/umbrello/index.html"), QLatin1String("umbrello"));
}

/**
 * Button clicked event handler for the dialog button box.
 * @param button  the button which was clicked
 */
void DialogBase::slotButtonClicked(QAbstractButton *button)
{
    if (button == (QAbstractButton*)m_pageDialog->button(QDialogButtonBox::Apply)) {
        DEBUG(DBG_SRC)  << "APPLY clicked...";
        slotApplyClicked();
    }
    else if (button == (QAbstractButton*)m_pageDialog->button(QDialogButtonBox::Ok)) {
        DEBUG(DBG_SRC)  << "OK clicked...";
        slotOkClicked();
    }
    else if (button == (QAbstractButton*)m_pageDialog->button(QDialogButtonBox::Cancel)) {
        DEBUG(DBG_SRC)  << "CANCEL clicked...";
    }
    else if (button == (QAbstractButton*)m_pageDialog->button(QDialogButtonBox::Help)) {
        DEBUG(DBG_SRC)  << "HELP clicked...";
        slotHelpClicked();
    }
    else {
        DEBUG(DBG_SRC)  << "Button clicked with unhandled role.";
    }
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
