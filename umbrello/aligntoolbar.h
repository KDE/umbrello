/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ALIGNTOOLBAR_H
#define ALIGNTOOLBAR_H

#include <qpixmap.h>
#include <ktoolbar.h>
#include "umlnamespace.h"
#include "umlwidgetlist.h"

class QMainWindow;
class UMLWidget;

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
    AlignToolBar(QMainWindow* parentWindow, const char* name);

    /**
     * Standard deconstructor.
     */
    ~AlignToolBar();

private:

    /**
     * Returns true if the first widget's X is smaller than second's.
     * Used for sorting the UMLWidgetList.
     *
     * @param widget1 The widget to compare.
     * @param widget2 The widget to compare with.
     */
    static bool hasWidgetSmallerX(const UMLWidget* widget1, const UMLWidget* widget2);

    /**
     * Returns true if the first widget's Y is smaller than second's.
     * Used for sorting the UMLWidgetList.
     *
     * @param widget1 The widget to compare.
     * @param widget2 The widget to compare with.
     */
    static bool hasWidgetSmallerY(const UMLWidget* widget1, const UMLWidget* widget2);

    /**
     * Loads toolbar icon.
     */
    void loadPixmaps();

    /**
     * Looks for the smallest x-value of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getSmallestX(const UMLWidgetList &widgetList);

    /**
     * Looks for the smallest y-value of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getSmallestY(const UMLWidgetList &widgetList);

    /**
     * Looks for the biggest x-value of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getBiggestX(const UMLWidgetList &widgetList);

    /**
     * Looks for the biggest y-value of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getBiggestY(const UMLWidgetList &widgetList);

    /**
     * Returns the sum of the heights of the given UMLWidgets
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getHeightsSum(const UMLWidgetList &widgetList);

    /**
     * Returns the sum of the widths of the given UMLWidgets.
     *
     * @param widgetList A list with UMLWidgets.
     */
    int getWidthsSum(const UMLWidgetList &widgetList);

    /**
     * Aligns all the widgets in the list to the left.
     *
     * @param widgetList The list with the widgets to align.
     */
    void alignLeft(UMLWidgetList &widgetList);

    /**
     * Aligns all the widgets in the list to the right.
     *
     * @param widgetList The list with the widgets to align.
     */
    void alignRight(UMLWidgetList &widgetList);

    /**
     * Aligns all the widgets in the list to the top.
     *
     * @param widgetList The list with the widgets to align.
     */
    void alignTop(UMLWidgetList &widgetList);

    /**
     * Aligns all the widgets in the list to the bottom.
     *
     * @param widgetList The list with the widgets to align.
     */
    void alignBottom(UMLWidgetList &widgetList);

    /**
     * Aligns all the widgets in the list to the vertical middle.
     *
     * @param widgetList The list with the widgets to align.
     */
    void alignVerticalMiddle(UMLWidgetList &widgetList);

    /**
     * Aligns all the widgets in the list to the horizontal middle.
     *
     * @param widgetList The list with the widgets to align.
     */
    void alignHorizontalMiddle(UMLWidgetList &widgetList);

    /**
     * Distributes all the widgets in the list at the same vertical distance
     * from one widget to the next.
     *
     * @param widgetList The list with the widgets to distribute.
     */
    void alignVerticalDistribute(UMLWidgetList &widgetList);

    /**
     * Distributes all the widgets in the list at the same horizontal distance
     * from one widget to the next.
     *
     * @param widgetList The list with the widgets to distribute.
     */
    void alignHorizontalDistribute(UMLWidgetList &widgetList);

    /**
     * Sorts the given UMLWidgetList based on the Compare function.
     * The list is cleared and all the widgets are added again in order.
     *
     * The comp function gets two const UMLWidget* params and returns
     * a boolean telling if the first widget was smaller than the second,
     * whatever the "smaller" concept is depending on the sorting to do.
     *
     * @param widgetList The list with the widgets to order.
     * @param comp The comp function to compare the widgets.
     */
    template<typename Compare>
    void sortWidgetList(UMLWidgetList &widgetList, Compare comp);

    /**
     * Used to identify the buttons.
     */
    enum AlignAction
    {
        alac_align_left = 0,
        alac_align_right,
        alac_align_top,
        alac_align_bottom,
        alac_align_vertical_middle,
        alac_align_horizontal_middle,
        alac_align_vertical_distribute,
        alac_align_horizontal_distribute,
        alac_none
    };

    /**
     * Holds the number of buttons.
     */
    static const unsigned nrAlignButtons = (unsigned) alac_none -
                                           (unsigned) alac_align_left;

    /**
     * Holds the icons for the different buttons.
     */
    QPixmap m_Pixmaps[nrAlignButtons];

private slots:

    /**
     * Performs the alignment when a button was clicked.
     *
     * @param btn The clicked button.
     */
    void slotButtonChanged(int btn);
};

#endif
