/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "aligntoolbar.h"

// system includes
#include <algorithm>
#include <vector>

// qt includes
#include <qmainwindow.h>

// kde includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// app includes
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"

AlignToolBar::AlignToolBar(QMainWindow* parentWindow, const char* )
        : KToolBar(parentWindow,Qt::DockRight,false) {
    // load images for the buttons
    loadPixmaps();

    // create the buttons
    insertButton(m_Pixmaps[alac_align_left], alac_align_left, true, i18n("Align Left"));
    insertButton(m_Pixmaps[alac_align_right], alac_align_right, true, i18n("Align Right"));
    insertButton(m_Pixmaps[alac_align_top], alac_align_top, true, i18n("Align Top"));
    insertButton(m_Pixmaps[alac_align_bottom], alac_align_bottom, true, i18n("Align Bottom"));
    insertButton(m_Pixmaps[alac_align_vertical_middle], alac_align_vertical_middle, true, i18n("Align Vertical Middle"));
    insertButton(m_Pixmaps[alac_align_horizontal_middle], alac_align_horizontal_middle, true, i18n("Align Horizontal Middle"));
    insertButton(m_Pixmaps[alac_align_vertical_distribute], alac_align_vertical_distribute, true, i18n("Align Vertical Distribute"));
    insertButton(m_Pixmaps[alac_align_horizontal_distribute], alac_align_horizontal_distribute, true, i18n("Align Horizontal Distribute"));

    setOrientation( Qt::Vertical );
    setVerticalStretchable( true );

    // gets called whenever a button in the toolbar is clicked
    connect( this, SIGNAL( released( int ) ), this, SLOT( slotButtonChanged (int ) ) );
}

AlignToolBar::~AlignToolBar() {
}

/* ------ private functions ------ */

bool AlignToolBar::hasWidgetSmallerX(const UMLWidget* widget1, const UMLWidget* widget2) {
    return widget1->getX() < widget2->getX();
}

bool AlignToolBar::hasWidgetSmallerY(const UMLWidget* widget1, const UMLWidget* widget2) {
    return widget1->getY() < widget2->getY();
}

void AlignToolBar::loadPixmaps() {
    KStandardDirs* dirs = KGlobal::dirs();
    QString dataDir = dirs->findResourceDir( "data", "umbrello/pics/object.png" );
    dataDir += "/umbrello/pics/";

    m_Pixmaps[alac_align_left].load( dataDir + "align_left.png" );
    m_Pixmaps[alac_align_right].load( dataDir + "align_right.png" );
    m_Pixmaps[alac_align_top].load( dataDir + "align_top.png" );
    m_Pixmaps[alac_align_bottom].load( dataDir + "align_bottom.png" );
    m_Pixmaps[alac_align_vertical_middle].load( dataDir + "align_vert_middle.png" );
    m_Pixmaps[alac_align_horizontal_middle].load( dataDir + "align_hori_middle.png" );
    m_Pixmaps[alac_align_vertical_distribute].load( dataDir + "align_vert_distribute.png" );
    m_Pixmaps[alac_align_horizontal_distribute].load( dataDir + "align_hori_distribute.png" );

    return;
}

int AlignToolBar::getSmallestX(const UMLWidgetList &widgetList) {
    UMLWidgetListIt it(widgetList);
    UMLWidget* widget;

    int smallestX = it.toFirst()->getX();
    ++it;

    while ((widget = it.current()) != 0) {
        ++it;
        if (smallestX > widget->getX())
            smallestX = widget->getX();
    }

    return smallestX;
}

int AlignToolBar::getSmallestY(const UMLWidgetList &widgetList) {
    UMLWidgetListIt it(widgetList);
    UMLWidget* widget;

    int smallestY = it.toFirst()->getY();
    ++it;

    while ((widget = it.current()) != 0) {
        ++it;
        if (smallestY > widget->getY())
            smallestY = widget->getY();
    }

    return smallestY;
}

int AlignToolBar::getBiggestX(const UMLWidgetList &widgetList) {
    UMLWidgetListIt it(widgetList);
    UMLWidget* widget;

    int biggestX = it.toFirst()->getX();
    biggestX += it.current()->getWidth();
    ++it;

    while ((widget = it.current()) != 0) {
        ++it;
        if (biggestX < widget->getX() + widget->getWidth())
            biggestX = widget->getX() + widget->getWidth();
    }

    return biggestX;
}

int AlignToolBar::getBiggestY(const UMLWidgetList &widgetList) {
    UMLWidgetListIt it(widgetList);
    UMLWidget* widget;

    int biggestY = it.toFirst()->getY();
    biggestY += it.current()->getHeight();
    ++it;

    while ((widget = it.current()) != 0) {
        ++it;
        if (biggestY < widget->getY() + widget->getHeight())
            biggestY = widget->getY() + widget->getHeight();
    }

    return biggestY;
}

int AlignToolBar::getHeightsSum(const UMLWidgetList &widgetList) {
    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    int heightsSum = 0;

    it.toFirst();
    while ((widget = it.current()) != 0) {
        ++it;
        heightsSum += widget->getHeight();
    }

    return heightsSum;
}

int AlignToolBar::getWidthsSum(const UMLWidgetList &widgetList) {
    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    int widthsSum = 0;

    it.toFirst();
    while ((widget = it.current()) != 0) {
        ++it;
        widthsSum += widget->getWidth();
    }

    return widthsSum;
}

void AlignToolBar::alignLeft(UMLWidgetList &widgetList) {
    int smallestX = getSmallestX(widgetList);

    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    it.toFirst();
    while ((widget = it.current()) != 0) {
        ++it;
        widget->setX(smallestX);
    }
}

void AlignToolBar::alignRight(UMLWidgetList &widgetList) {
    int biggestX = getBiggestX(widgetList);

    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    it.toFirst();
    while ((widget = it.current()) != 0) {
        ++it;
        widget->setX(biggestX - widget->getWidth());
    }
}

void AlignToolBar::alignTop(UMLWidgetList &widgetList) {
    int smallestY = getSmallestY(widgetList);

    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    it.toFirst();
    while ((widget = it.current()) != 0) {
        ++it;
        widget->setY(smallestY);
    }
}

void AlignToolBar::alignBottom(UMLWidgetList &widgetList) {
    int biggestY = getBiggestY(widgetList);

    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    it.toFirst();
    while ((widget = it.current()) != 0) {
        ++it;
        widget->setY(biggestY - widget->getHeight());
    }
}

void AlignToolBar::alignVerticalMiddle(UMLWidgetList &widgetList) {
    int smallestX = getSmallestX(widgetList);
    int biggestX = getBiggestX(widgetList);
    int middle = int((biggestX - smallestX) / 2) + smallestX;

    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    it.toFirst();
    while ((widget = it.current()) != 0) {
        ++it;
        widget->setX(middle - int(widget->getWidth() / 2));
    }
}

void AlignToolBar::alignHorizontalMiddle(UMLWidgetList &widgetList) {
    int smallestY = getSmallestY(widgetList);
    int biggestY = getBiggestY(widgetList);
    int middle = int((biggestY - smallestY) / 2) + smallestY;

    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    it.toFirst();
    while ((widget = it.current()) != 0) {
        ++it;
        widget->setY(middle - int(widget->getHeight() / 2));
    }
}

void AlignToolBar::alignVerticalDistribute(UMLWidgetList &widgetList) {
    int smallestY = getSmallestY(widgetList);
    int biggestY = getBiggestY(widgetList);
    int heightsSum = getHeightsSum(widgetList);
    int distance = int(((biggestY - smallestY) - heightsSum) / (widgetList.count()-1.0) + 0.5);

    sortWidgetList(widgetList, hasWidgetSmallerY);

    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    it = UMLWidgetListIt(widgetList);

    UMLWidget* widgetPrev = it.toFirst();
    ++it;
    while ((widget = it.current()) != 0) {
        ++it;
        widget->setY(widgetPrev->getY() + widgetPrev->getHeight() + distance);
        widgetPrev = widget;
    }
}

void AlignToolBar::alignHorizontalDistribute(UMLWidgetList &widgetList) {
    int smallestX = getSmallestX(widgetList);
    int biggestX = getBiggestX(widgetList);
    int widthsSum = getWidthsSum(widgetList);
    int distance = int(((biggestX - smallestX) - widthsSum) / (widgetList.count()-1.0) + 0.5);

    sortWidgetList(widgetList, hasWidgetSmallerX);

    UMLWidget* widget;
    UMLWidgetListIt it(widgetList);

    it = UMLWidgetListIt(widgetList);

    UMLWidget* widgetPrev = it.toFirst();
    ++it;
    while ((widget = it.current()) != 0) {
        ++it;
        widget->setX(widgetPrev->getX() + widgetPrev->getWidth() + distance);
        widgetPrev = widget;
    }
}

template<typename Compare>
void AlignToolBar::sortWidgetList(UMLWidgetList &widgetList, Compare comp) {
    std::vector<UMLWidget*> widgetVector;
    UMLWidgetListIt it(widgetList);
    while (it.current() != 0) {
        widgetVector.push_back(*it);
        ++it;
    }
    sort(widgetVector.begin(), widgetVector.end(), comp);

    widgetList.clear();

    for (std::vector<UMLWidget*>::iterator it=widgetVector.begin(); it != widgetVector.end(); ++it) {
        widgetList.append(*it);
    }
}

/* ------ private slots ------ */

void AlignToolBar::slotButtonChanged(int btn) {
    UMLView* view = UMLApp::app()->getCurrentView();
    UMLWidgetList widgetList;
    UMLWidget* widget;

    // get the list with selected widgets (not associations)
    view->getSelectedWidgets(widgetList);
    UMLWidgetListIt it(widgetList);

    // at least 2 widgets must be selected
    if (widgetList.count() > 1) {
        // now perform alignment according to the clicked button
        switch (btn) {
        case alac_align_left:
            alignLeft(widgetList);
            break;

        case alac_align_right:
            alignRight(widgetList);
            break;

        case alac_align_top:
            alignTop(widgetList);
            break;

        case alac_align_bottom:
            alignBottom(widgetList);
            break;

        case alac_align_vertical_middle:
            alignVerticalMiddle(widgetList);
            break;

        case alac_align_horizontal_middle:
            alignHorizontalMiddle(widgetList);
            break;

        case alac_align_vertical_distribute:
            alignVerticalDistribute(widgetList);
            break;

        case alac_align_horizontal_distribute:
            alignHorizontalDistribute(widgetList);
            break;

        } // switch (btn)

        // update associations
        it.toFirst();
        while ((widget = it.current()) != 0) {
            ++it;
            widget->updateWidget();
        }
        UMLApp::app()->getDocument()->setModified();
    } else {
        KMessageBox::messageBox(0, KMessageBox::Information,
                                i18n("For alignment you have to select at least 2 objects like classes or actors. You can not align associations."),
                                i18n("Information"), i18n("&OK"), QString(""),
                                "showAlignInformation");
    } // if (widgetList.count() > 1)

    return;
}

#include "aligntoolbar.moc"
