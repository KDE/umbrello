/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "aligntoolbar.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

AlignToolBar::AlignToolBar(QMainWindow* parentWindow, const char* name)
  : KToolBar(parentWindow,Qt::DockRight,false,name)
{
	// load images for the buttons
	loadPixmaps();

	// create the buttons
	insertButton(m_Pixmaps[alac_align_left], alac_align_left, true, i18n("Align Left"));
	insertButton(m_Pixmaps[alac_align_right], alac_align_right, true, i18n("Align Right"));
	insertButton(m_Pixmaps[alac_align_top], alac_align_top, true, i18n("Align Top"));
	insertButton(m_Pixmaps[alac_align_bottom], alac_align_bottom, true, i18n("Align Bottom"));
	insertButton(m_Pixmaps[alac_align_vertical_middle], alac_align_vertical_middle, true, i18n("Align Vertical Middle"));
	insertButton(m_Pixmaps[alac_align_horizontal_middle], alac_align_horizontal_middle, true, i18n("Align Horizontal Middle"));

	setOrientation( Vertical );
	setVerticalStretchable( true );

	// gets called whenever a button in the toolbar is clicked
	connect( this, SIGNAL( released( int ) ), this, SLOT( slotButtonChanged (int ) ) );
}

AlignToolBar::~AlignToolBar()
{
}

/* ------ private functions ------ */

int AlignToolBar::getSmallestX(UMLWidgetList widgetList)
{
	UMLWidgetListIt it(widgetList);
	UMLWidget * widget;

	int smallest_x = it.toFirst()->getX();
	++it;

	while ((widget = it.current()) != 0)
	{
		++it;
		if (smallest_x > widget->getX())
			smallest_x = widget->getX();
	}

	return smallest_x;
}

int AlignToolBar::getSmallestY(UMLWidgetList widgetList)
{
	UMLWidgetListIt it(widgetList);
	UMLWidget * widget;

	int smallest_y = it.toFirst()->getY();
	++it;

	while ((widget = it.current()) != 0)
	{
		++it;
		if (smallest_y > widget->getY())
			smallest_y = widget->getY();
	}

	return smallest_y;
}

int AlignToolBar::getBiggestX(UMLWidgetList widgetList)
{
	UMLWidgetListIt it(widgetList);
	UMLWidget * widget;

	int biggest_x = it.toFirst()->getX();
	biggest_x += it.current()->getWidth();
	++it;

	while ((widget = it.current()) != 0)
	{
		++it;
		if (biggest_x < widget->getX() + widget->getWidth())
			biggest_x = widget->getX() + widget->getWidth();
	}

	return biggest_x;
}

int AlignToolBar::getBiggestY(UMLWidgetList widgetList)
{
	UMLWidgetListIt it(widgetList);
	UMLWidget * widget;

	int biggest_y = it.toFirst()->getY();
	biggest_y += it.current()->getHeight();
	++it;

	while ((widget = it.current()) != 0)
	{
		++it;
		if (biggest_y < widget->getY() + widget->getHeight())
			biggest_y = widget->getY() + widget->getHeight();
	}

	return biggest_y;
}

void AlignToolBar::loadPixmaps()
{
	KStandardDirs * dirs = KGlobal::dirs();
	QString dataDir = dirs -> findResourceDir( "data", "umbrello/pics/object.png" );
	dataDir += "/umbrello/pics/";

	m_Pixmaps[alac_align_left].load( dataDir + "align_left.png" );
	m_Pixmaps[alac_align_right].load( dataDir + "align_right.png" );
	m_Pixmaps[alac_align_top].load( dataDir + "align_top.png" );
	m_Pixmaps[alac_align_bottom].load( dataDir + "align_bottom.png" );
	m_Pixmaps[alac_align_vertical_middle].load( dataDir + "align_vert_middle.png" );
	m_Pixmaps[alac_align_horizontal_middle].load( dataDir + "align_hori_middle.png" );

	return;
}

/* ------ private slots ------ */

void AlignToolBar::slotButtonChanged(int btn)
{
	UMLView* view = UMLApp::app()->getDocument()->getCurrentView();
	UMLWidgetList widgetList;
	UMLWidget * widget;

	int smallest_x = 0;
	int biggest_x = 0;
	int smallest_y = 0;
	int biggest_y = 0;
	int middle = 0;

	// get the list with selected widgets (not associations)
	view->getSelectedWidgets(widgetList);
	UMLWidgetListIt it(widgetList);

	// at least 2 widgets must be selected
	if (widgetList.count() > 1)
	{
		// now perform alignment according to the clicked button
		switch (btn)
		{
			// align left
			case alac_align_left:
					smallest_x = getSmallestX(widgetList);
			
					it.toFirst();
					while ((widget = it.current()) != 0)
					{
						++it;
						widget->setX(smallest_x);
					}
				break;

			// align right
			case alac_align_right:
					biggest_x = getBiggestX(widgetList);
			
					it.toFirst();
					while ((widget = it.current()) != 0)
					{
						++it;
						widget->setX(biggest_x - widget->getWidth());
					}
				break;

			// align top
			case alac_align_top:
					smallest_y = getSmallestY(widgetList);
			
					it.toFirst();
					while ((widget = it.current()) != 0)
					{
						++it;
						widget->setY(smallest_y);
					}
				break;

			// align bottom
			case alac_align_bottom:
					biggest_y = getBiggestY(widgetList);
			
					it.toFirst();
					while ((widget = it.current()) != 0)
					{
						++it;
						widget->setY(biggest_y - widget->getHeight());
					}
				break;

			// align vertical middle
			case alac_align_vertical_middle:
					smallest_x = getSmallestX(widgetList);
					biggest_x = getBiggestX(widgetList);
					middle = int((biggest_x - smallest_x) / 2) + smallest_x;

					it.toFirst();
					while ((widget = it.current()) != 0)
					{
						++it;
						widget->setX(middle - int(widget->getWidth() / 2));
					}
				break;

			// align horizontal middle
			case alac_align_horizontal_middle:
					smallest_y = getSmallestY(widgetList);
					biggest_y = getBiggestY(widgetList);
					middle = int((biggest_y - smallest_y) / 2) + smallest_y;

					it.toFirst();
					while ((widget = it.current()) != 0)
					{
						++it;
						widget->setY(middle - int(widget->getHeight() / 2));
					}
				break;

		} // switch (btn)

		// update associations
		it.toFirst();
		while ((widget = it.current()) != 0)
		{
			++it;
			widget->updateWidget();
		}
		UMLApp::app()->getDocument()->setModified();
	} else {
		KMessageBox::messageBox(0, KMessageBox::Information,
			i18n("For alignment you have to select at least 2 objects like classes or actors. You can not align associations."),
			i18n("Information"), i18n("&Ok"), QString("")
#if KDE_IS_VERSION(3,2,0)
			, "showAlignInformation"
#endif
			);
	} // if (widgetList.count() > 1)

	return;
}

#include "aligntoolbar.moc"
