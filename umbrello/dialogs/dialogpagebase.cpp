/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "dialogpagebase.h"

/**
 * Constructor
 */
DialogPageBase::DialogPageBase(QWidget *parent)
  : QWidget(parent)
{
}

DialogPageBase::~DialogPageBase()
{
}

/**
 * Return state if page has been modified by user.
 *
 * The state will be used to determine changed pages.
 *
 * @return true page has been modified
 */
bool DialogPageBase::isModified()
{
    return false;
}
