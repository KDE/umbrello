/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      kiriuja  <kplayer-dev@en-directo.net>         *
 *   copyright (C) 2003-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

/*
 * Taken from kplayer CVS 2003-09-21 (kplayer > 0.3.1) by Jonathan Riddell.
 */

#include "kplayerslideraction.h"

#include <ktoolbar.h>
#include <kdebug.h>

#include <QtGui/QKeyEvent>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>

/**
 * The KPlayerPopupFrame constructor. Parameters are passed on to QFrame.
 */
KPlayerPopupFrame::KPlayerPopupFrame (QWidget* parent)
  : QFrame (parent, Qt::WType_Popup)
{
}

/**
 * The KPlayerPopupFrame destructor. Does nothing.
 */
KPlayerPopupFrame::~KPlayerPopupFrame()
{
}

/**
 * Closes the popup frame when Alt, Tab, Esc, Enter or Return is pressed.
 */
void KPlayerPopupFrame::keyPressEvent (QKeyEvent* ev)
{
    switch ( ev->key() )
    {
    case Qt::Key_Alt:
    case Qt::Key_Tab:
    case Qt::Key_Escape:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        close();
    }
}

/*
void KPlayerPopupFrame::closeEvent (QCloseEvent* ev)
{
  QFrame::closeEvent (ev);
}

void KPlayerPopupFrame::mousePressEvent (QMouseEvent* ev)
{
  QFrame::mousePressEvent (ev);
//if ( ! rect().contains (ev->pos()) )
//  m_outside_mouse_press = true;
}

void KPlayerPopupFrame::mouseReleaseEvent (QMouseEvent* ev)
{
  QFrame::mouseReleaseEvent (ev);
  if ( m_outside_mouse_press )
  {
    m_outside_mouse_press = false;
    if ( ! rect().contains (ev->pos()) )
      close();
  }
}
*/

/**
 * The KPlayerPopupSliderAction constructor. Parameters are passed on to KAction.
 */
KPlayerPopupSliderAction::KPlayerPopupSliderAction (const QObject* receiver, const char* slot,
                                                    QObject *parent)
  : KAction(parent)
{
    m_frame = new KPlayerPopupFrame;
    m_frame->setFrameStyle(QFrame::Raised);
    m_frame->setLineWidth (2);
    m_slider = new KPlayerSlider(Qt::Vertical, m_frame);
    m_frame->resize (36, m_slider->sizeHint().height() + 4);
    m_slider->setGeometry(m_frame->contentsRect());
    // uDebug() << "Popup slider size " << m_slider->width() << "x" << m_slider->height();
    connect (this, SIGNAL(triggered()), this , SLOT(slotTriggered()));
    connect (m_slider, SIGNAL (changed (int)), receiver, slot);
}

/**
 * The KPlayerPopupSliderAction destructor. Deletes the KPlayerPopupFrame.
 */
KPlayerPopupSliderAction::~KPlayerPopupSliderAction()
{
    delete m_frame;
    m_frame = 0;
}

/**
 * Pops up the slider.
 */
void KPlayerPopupSliderAction::slotTriggered()
{
    QPoint point;

    QList<QWidget*> associatedWidgetsList = QWidgetAction::associatedWidgets();

    QWidget* associatedWidget = 0;
    QWidget* associatedToolButton = 0;

    // find the toolbutton which was clicked on
   foreach(associatedWidget, associatedWidgetsList) {
      if (KToolBar* associatedToolBar = dynamic_cast<KToolBar*>(associatedWidget)) {
        associatedToolButton = associatedToolBar->childAt(associatedToolBar->mapFromGlobal(QCursor::pos()));
        if(associatedToolButton) {
          break;  // found the tool button which was clicked
        }
      }
    }

   if ( associatedToolButton  ) {
     point = associatedToolButton->mapToGlobal(QPoint(0,associatedToolButton->height()));

   } else {

     point = QCursor::pos() - QPoint (m_frame->width() / 2, m_frame->height() / 2);
     if ( point.x() + m_frame->width() > QApplication::desktop()->width() )
       point.setX (QApplication::desktop()->width() - m_frame->width());
     if ( point.y() + m_frame->height() > QApplication::desktop()->height() )
       point.setY (QApplication::desktop()->height() - m_frame->height());
     if ( point.x() < 0 )
       point.setX (0);
     if ( point.y() < 0 )
       point.setY (0);
   }

    // uDebug() << "Point: " << point.x() << "x" << point.y() << "\n";
    m_frame->move (point);
    /*if ( kapp && kapp->activeWindow() ) {
        QMouseEvent me (QEvent::MouseButtonRelease, QPoint(0,0), QPoint(0,0), QMouseEvent::LeftButton, QMouseEvent::NoButton);
        QApplication::sendEvent (kapp->activeWindow(), &me);
      }*/
    m_frame->show();
    m_slider->setFocus();
}

// /**
//  * The KPlayerSliderAction constructor. Parameters are passed on to KAction.
//  */
// KPlayerSliderAction::KPlayerSliderAction (const QString& text, const KShortcut& cut,
//         const QObject* receiver, const char* slot, KActionCollection* parent, const char* name)
//         : KAction (new KPlayerSlider(Qt::Horizontal, 0), text, cut, receiver, slot, parent, name)
//         //: KAction (text, 0, parent, name)
// {
//     setAutoSized (true);
//     connect (slider(), SIGNAL (changed (int)), receiver, slot);
// }

// /**
//  * The KPlayerSliderAction destructor. Does nothing.
//  */
// KPlayerSliderAction::~KPlayerSliderAction()
// {
// }

// /**
//  * Plugs the slider into the toolbar.
//  */
// int KPlayerSliderAction::plug (QWidget* widget, int index)
// {
//     //Q_ASSERT (widget);
//     //Q_ASSERT (! isPlugged());
//     //Q_ASSERT (slider());
//     //if ( ! slider() || ! widget || isPlugged() )
//     //  return -1;
//     //Q_ASSERT (widget->inherits ("KToolBar"));
//     //if ( ! widget->inherits ("KToolBar") )
//     //  return -1;
//     //if ( kapp && ! kapp->authorizeKAction (name()) )
//     //  return -1;
//     int result = K3WidgetAction::plug (widget, index);
//     if ( result < 0 )
//         return result;
//     KToolBar* toolbar = (KToolBar*) widget;
//     //int id = getToolButtonID();
//     //uDebug() << "Orientation: " << toolbar->orientation() << "\n";
//     //m_slider->reparent (toolbar, QPoint());
//     //toolbar->insertWidget (id, 0, m_slider, index);
//     //toolbar->setItemAutoSized (id, true);
//     //QWhatsThis::remove (m_slider);
//     //if ( ! whatsThis().isEmpty() )
//     //  QWhatsThis::add (m_slider, whatsThis());
//     //if ( ! text().isEmpty() )
//     //  QToolTip::add (m_slider, text());
//     //addContainer (toolbar, id);
//     //setupToolbar (toolbar->orientation(), toolbar);
//     orientationChanged (toolbar->orientation());
//     connect (toolbar, SIGNAL (orientationChanged (Orientation)), this, SLOT (orientationChanged (Orientation)));
//     //connect (toolbar, SIGNAL (destroyed()), this, SLOT (toolbarDestroyed()));
//     //if ( parentCollection() )
//     //  parentCollection()->connectHighlight (toolbar, this);
//     //return containerCount() - 1;
//     return result;
// }

// /**
//  * Unplugs the slider from the toolbar.
//  */
// void KPlayerSliderAction::unplug (QWidget* widget)
// {
//     //Q_ASSERT (m_slider);
//     //Q_ASSERT (isPlugged());
//     //Q_ASSERT (widget->inherits ("KToolBar"));
//     K3WidgetAction::unplug (widget);
//     if ( ! slider() || ! isPlugged() || widget != slider()->parent() )
//         return;
//     //KToolBar* toolbar = (KToolBar*) widget;
//     disconnect (widget, SIGNAL (orientationChanged (Orientation)), this, SLOT (orientationChanged (Orientation)));
//     //disconnect (toolbar, SIGNAL (destroyed()), this, SLOT (toolbarDestroyed()));
//     //m_slider->reparent (0, QPoint());
// //    int index = findContainer (toolbar);
// //      if ( index == -1 )
// //        return;
// //      bar->removeItem (menuId (index));
// //      removeContainer (index);
// }

// void KPlayerSliderAction::setupToolbar (Orientation orientation, KToolBar* toolbar)
// {
//     if ( orientation == Qt::Horizontal )
//     {
// //      toolbar->setMinimumWidth (300);
// //      toolbar->setMinimumHeight (0);
//       toolbar->setFixedExtentWidth (300);
//       toolbar->setFixedExtentHeight (-1);
// //      toolbar->setHorizontallyStretchable (true);
// //      toolbar->setVerticallyStretchable (false);
//     }
//     else
//     {
// //      toolbar->setMinimumWidth (0);
// //      toolbar->setMinimumHeight (300);
//       toolbar->setFixedExtentWidth (-1);
//       toolbar->setFixedExtentHeight (300);
// //      toolbar->setHorizontallyStretchable (false);
// //      toolbar->setVerticallyStretchable (true);
//     }
// }

// /**
//  * Changes the slider orientation when the toolbar orientation changes.
//  */
// void KPlayerSliderAction::orientationChanged (Qt::Orientation orientation)
// {
//     //if ( sender() && sender()->inherits ("KToolBar") )
//     //  setupToolbar (orientation, (KToolBar*) sender());
//     //Q_ASSERT (m_slider);
//     //Q_ASSERT (isPlugged());
//     if ( slider() )
//         slider()->setOrientation (orientation);
// }

// void KPlayerSliderAction::toolbarDestroyed (void)
// {
//   if ( m_slider )
//     m_slider->reparent (0, QPoint());
// }

/**
 * The KPlayerSlider constructor. Parameters are passed on to QSlider.
 */
KPlayerSlider::KPlayerSlider (Qt::Orientation orientation, QWidget* parent)
  : QSlider (orientation, parent)
{
    setup(300, 2200, 1000, 400);
    m_changing_orientation = false;
    setTickPosition (QSlider::TicksBothSides);
    connect (this, SIGNAL (valueChanged (int)), this, SLOT (sliderValueChanged (int)));
}

/**
 * The KPlayerSlider destructor. Does nothing.
 */
KPlayerSlider::~KPlayerSlider()
{
    // uDebug() << "KPlayerSlider destroyed\n";
}

/**
 * The size hint.
 */
QSize KPlayerSlider::sizeHint() const
{
    QSize hint = QSlider::sizeHint();
    int length = 200;
    if ( orientation() == Qt::Horizontal )
    {
        if ( hint.width() < length )
            hint.setWidth (length);
    }
    else
    {
        if ( hint.height() < length )
            hint.setHeight (length);
    }
    return hint;
}

/**
 * The minimum size hint.
 */
QSize KPlayerSlider::minimumSizeHint() const
{
    // uDebug() << "KPlayerSlider minimum size hint\n";
    QSize hint = QSlider::minimumSizeHint();
    int length = 200;
    if ( orientation() == Qt::Horizontal )
    {
        if ( hint.width() < length )
            hint.setWidth (length);
    }
    else
    {
        if ( hint.height() < length )
            hint.setHeight (length);
    }
    return hint;
}

/**
 * Sets the slider orientation.
 */
void KPlayerSlider::setOrientation (Qt::Orientation o)
{
    if ( o == orientation() )
        return;
    m_changing_orientation = true;
    int minimum = QSlider::minimum();
    int maximum = QSlider::maximum();
    int value = QSlider::value();
    QSlider::setOrientation (o);
    QSlider::setMinimum (- maximum);
    QSlider::setMaximum (- minimum);
    QSlider::setValue (- value);
    m_changing_orientation = false;
}

/**
 * The minimum value.
 */
int KPlayerSlider::minimum (void) const
{
    if ( orientation() == Qt::Horizontal )
        return QSlider::minimum();
    return - QSlider::maximum();
}

/**
 * Sets the minimum value.
 */
void KPlayerSlider::setMinimum (int minimum)
{
    if ( orientation() == Qt::Horizontal )
        QSlider::setMinimum (minimum);
    else
        QSlider::setMaximum (- minimum);
}

/**
 * The maximum value.
 */
int KPlayerSlider::maximum (void) const
{
    if ( orientation() == Qt::Horizontal )
        return QSlider::maximum();
    return - QSlider::minimum();
}

/**
 * Sets the maximum value.
 */
void KPlayerSlider::setMaximum (int maximum)
{
    if ( orientation() == Qt::Horizontal )
        QSlider::setMaximum (maximum);
    else
        QSlider::setMinimum (- maximum);
}

/**
 * The single step.
 */
int KPlayerSlider::singleStep (void) const
{
    return QSlider::singleStep();
}

/**
 * Sets the single step.
 */
void KPlayerSlider::setSingleStep (int singleStep)
{
    QSlider::setSingleStep (singleStep);
}

/**
 * The page step.
 */
int KPlayerSlider::pageStep (void) const
{
    return QSlider::pageStep();
}

/**
 * Sets the page step.
 */
void KPlayerSlider::setPageStep (int pageStep)
{
    QSlider::setPageStep (pageStep);
    setTickInterval (pageStep);
}

/**
 * The current value.
 */
int KPlayerSlider::value (void) const
{
    if ( orientation() == Qt::Horizontal )
        return QSlider::value();
    return - QSlider::value();
}

/**
 * Sets the current value.
 * Do not override the virtual setValue.
 * The extra parameter prevents overriding of the virtual QSlider::setValue.
 */
void KPlayerSlider::setValue (int value, int)
{
    if ( orientation() == Qt::Horizontal )
        QSlider::setValue (value);
    else
        QSlider::setValue (- value);
}

/**
 * Sets up the slider by setting five options in one go.
 */
void KPlayerSlider::setup (int minimum, int maximum, int value, int pageStep, int singleStep)
{
    setMinimum (minimum);
    setMaximum (maximum);
    setSingleStep (singleStep);
    setPageStep (pageStep);
    setValue (value);
}

/**
 * Receives the valueChanged signal from QSlider.
 */
void KPlayerSlider::sliderValueChanged (int)
{
    if ( ! m_changing_orientation )
        emit changed (value());
}

#include "kplayerslideraction.moc"
