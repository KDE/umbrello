/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DIALOGBASE_H
#define DIALOGBASE_H

#include "icon_utils.h"

// kde class includes
#include <kpagedialog.h>

//forward declarations
class QFrame;
class UMLWidget;
class KFontChooser;
class KPageWidgetItem;
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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class DialogBase : public KPageDialog
{
    Q_OBJECT

public:
    DialogBase(QWidget *parent);
    QFrame* createPage(const QString& name, const QString& header, Icon_Utils::IconType icon);
    KPageWidgetItem *setupFontPage(UMLWidget *widget);
    void saveFontPageData(UMLWidget *widget);

    KPageWidgetItem *setupStylePage(UMLWidget *widget);
    void saveStylePageData(UMLWidget *widget);

protected:
    KFontChooser *m_fontChooser;
    UMLWidgetStylePage *m_pStylePage;
    KPageWidgetItem *m_pageItem;

};

#endif
