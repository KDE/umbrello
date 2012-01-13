/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "dialogbase.h"

// local includes
#include "icon_utils.h"
#include "umlwidgetstylepage.h"
#include "klocale.h"
#include <KFontChooser>
#include "umlwidget.h"

// qt includes
#include <QFrame>
#include <QHBoxLayout>

/**
 * Constructor
 */
DialogBase::DialogBase(QWidget *parent)
  : KPageDialog( parent)
{
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
    m_pageItem = new KPageWidgetItem( page, name );
    m_pageItem->setHeader( header );
    m_pageItem->setIcon( Icon_Utils::DesktopIcon(icon) );
    addPage( m_pageItem );
    //page->setMinimumSize( 310, 330 );
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
    m_fontChooser = new KFontChooser( (QWidget*)page, KFontChooser::NoDisplayFlags, QStringList(), 0);
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
    widget->setFont( m_fontChooser->font() );
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
    m_pStylePage->updateUMLWidget();
}
