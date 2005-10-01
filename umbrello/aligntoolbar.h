/*
 *  copyright (C) 2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ALIGNTOOLBAR_H
#define ALIGNTOOLBAR_H

#include <ktoolbar.h>
#include "umlnamespace.h"
#include "umlwidgetlist.h"


/**
 * This toolbar provides tools for alignment. Widgets can only be aligned, when
 * there are at least 2 widgets (not associations) are selected
 *
 * @short Toolbar providing alignment tools.
 * @author Sebastian Stein <seb.kde@hpfsc.de>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AlignToolBar : public KToolBar {
    Q_OBJECT
public:

    /**
     * Creates a bar with tools for alignment.
     *
     * @param parentWindow      The parent of the toolbar.
     * @param name              The name of the toolbar.
     */
    AlignToolBar(Q3MainWindow* parentWindow, const char* name);

    /**
     * Standard deconstructor.
     */
    ~AlignToolBar();

private:

    /**
     * Loads toolbar icon
     */
    void loadPixmaps();

    /**
     * looks for the smallest x-value of the given UMLWidgets
     *
     * @param widgetList a list with UMLWidgest
     */
    int getSmallestX(UMLWidgetList widgetList);

    /**
     * looks for the smallest y-value of the given UMLWidgets
     *
     * @param widgetList a list with UMLWidgest
     */
    int getSmallestY(UMLWidgetList widgetList);

    /**
     * looks for the biggest x-value of the given UMLWidgets
     *
     * @param widgetList a list with UMLWidgest
     */
    int getBiggestX(UMLWidgetList widgetList);

    /**
     * looks for the biggest y-value of the given UMLWidgets
     *
     * @param widgetList a list with UMLWidgest
     */
    int getBiggestY(UMLWidgetList widgetList);

    /**
     * used to identify the buttons
     */
    enum AlignAction
    {
        alac_align_left = 0,
        alac_align_right,
        alac_align_top,
        alac_align_bottom,
        alac_align_vertical_middle,
        alac_align_horizontal_middle,
        alac_none
    };

    /**
     * holds the number of buttons
     */
    static const unsigned nrAlignButtons = (unsigned) alac_align_horizontal_middle -
                                           (unsigned) alac_align_left + 1;

    /**
     * holds the icons for the different buttons
     */
    QPixmap m_Pixmaps[nrAlignButtons];

private slots:

    /**
     * performs the alignment when a button was clicked
     *
     * @param btn the clicked button
     */ 
    void slotButtonChanged(int btn);
};

#endif
