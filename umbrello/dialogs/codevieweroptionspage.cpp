/***************************************************************************
                          codegenerationoptionspage.cpp  -  description
                             -------------------
    begin                : Thu Jul 25 2002
    copyright            : (C) 2002 by Luis De la Parra
    email                : luis@delaparra.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codevieweroptionspage.h"

// qt/kde includes
#include <kdebug.h>
#include <kfontdialog.h>
#include <kcolorbutton.h>


CodeViewerOptionsPage::CodeViewerOptionsPage( Settings::CodeViewerState options, QWidget *parent, const char *name )
        :CodeViewerOptionsBase(parent,name)
{
    init (options);
}

CodeViewerOptionsPage::~CodeViewerOptionsPage() { }

void CodeViewerOptionsPage::init( Settings::CodeViewerState options )
{
    m_options = options;

    // set widget stuff
    /*
        fontChooser->setFont( options.font );
        selectColorButton -> setColor (options.selectedColor);
        fontColorButton -> setColor (options.fontColor);
        paperColorButton -> setColor (options.paperColor);
        editBlockColorButton -> setColor (options.editBlockColor);
        nonEditBlockColorButton -> setColor (options.nonEditBlockColor);
        umlObjectColorButton -> setColor (options.umlObjectColor);
     */
}

void CodeViewerOptionsPage::apply() {
    /*
        m_options.umlObjectColor = umlObjectColorButton->color();
        m_options.editBlockColor = editBlockColorButton->color();
        m_options.nonEditBlockColor = nonEditBlockColorButton->color();
        m_options.selectedColor = selectColorButton->color();
        m_options.paperColor = paperColorButton->color();
        m_options.fontColor = fontColorButton->color();
        m_options.font = fontChooser->font();
     */
    emit applyClicked();
}

Settings::CodeViewerState CodeViewerOptionsPage::getOptions() {
    return m_options;
}

#include "codevieweroptionspage.moc"
