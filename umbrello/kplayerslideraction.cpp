/***************************************************************************
    begin                : Sat Jan 11 2003
    copyright            : (C) 2003 by kiriuja
    email                : kplayer-dev@en-directo.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* Taken from kplayer CVS 2003-09-21 (kplayer > 0.3.1) by Jonathan Riddell
 * Changes from kplayer original marked by CHANGED
 */

#include <kapplication.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kdebug.h>

//CHANGED #include "kplayersettings.h"
#include "kplayerslideraction.h"
#include "kplayerslideraction.moc"

void KPlayerPopupFrame::keyPressEvent (QKeyEvent* ev)
{
    switch ( ev -> key() )
    {
    case Qt::Key_Alt:
    case Qt::Key_Tab:
    case Qt::Key_Escape:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        close();
    }
}

/*void KPlayerPopupFrame::closeEvent (QCloseEvent* ev)
{
  QFrame::closeEvent (ev);
}

void KPlayerPopupFrame::mousePressEvent (QMouseEvent* ev)
{
  QFrame::mousePressEvent (ev);
//if ( ! rect().contains (ev -> pos()) )
//  m_outside_mouse_press = true;
}

void KPlayerPopupFrame::mouseReleaseEvent (QMouseEvent* ev)
{
  QFrame::mouseReleaseEvent (ev);
  if ( m_outside_mouse_press )
  {
    m_outside_mouse_press = false;
    if ( ! rect().contains (ev -> pos()) )
      close();
  }
}*/

KPlayerPopupSliderAction::KPlayerPopupSliderAction (const QString& text,
        const QString& pix, const KShortcut& shortcut, const QObject* receiver,
        const char* slot, QObject* parent, const char* name)
        : KAction (text, pix, shortcut, parent, name)
{
    m_frame = new KPlayerPopupFrame;
    m_frame -> setFrameStyle (QFrame::PopupPanel | QFrame::Raised);
    m_frame -> setLineWidth (2);
    m_slider = new KPlayerSlider (Qt::Vertical, m_frame);
    m_frame -> resize (36, m_slider -> sizeHint().height() + 4);
    m_slider -> setGeometry (m_frame -> contentsRect());
    //CHANGED  kdDebug() << "Popup slider size " << m_slider -> width() << "x" << m_slider -> height() << "\n";
    connect (m_slider, SIGNAL (changed (int)), receiver, slot);
}

KPlayerPopupSliderAction::~KPlayerPopupSliderAction()
{
    delete m_frame;
    m_frame = 0;
}

/*int KPlayerPopupSliderAction::plug (QWidget* widget, int index)
{
  Q_ASSERT (m_slider);
  Q_ASSERT (widget);
  Q_ASSERT (! isPlugged());
  if ( ! m_slider || ! widget || isPlugged() )
    return -1;
  Q_ASSERT (widget -> inherits ("KToolBar"));
  if ( ! widget -> inherits ("KToolBar") )
    return -1;
  int retval = KAction::plug (widget, index);
//  if ( retval >= 0 )
//    m_slider -> reparent (widget, QPoint());
  return retval;
}

void KPlayerPopupSliderAction::unplug (QWidget* widget)
{
  Q_ASSERT (m_slider);
  Q_ASSERT (widget);
  Q_ASSERT (isPlugged());
  Q_ASSERT (widget -> inherits ("KToolBar"));
  if ( ! m_slider || ! widget || ! isPlugged() || ! widget -> inherits ("KToolBar") )
    return;
//m_slider -> reparent (0, QPoint());
  KAction::unplug (widget);
}*/

void KPlayerPopupSliderAction::slotActivated (void)
{
    KAction::slotActivated();
    QWidget* button = 0;
    if ( sender() )
    {
        //CHANGED    kdDebug() << "Sender class name: " << sender() -> className() << "\n";
        if ( sender() -> inherits ("KToolBarButton") )
            button = (QWidget*) sender();
        else if ( sender() -> inherits ("KToolBar") )
        {
            KToolBar* toolbar = (KToolBar*) sender();
            int index = findContainer (toolbar);
            if ( index >= 0 )
                button = toolbar -> getButton (itemId (index));
        }
    }
    QPoint point;
    if ( button )
        point = button -> mapToGlobal (QPoint (0, button -> height()));
    else
    {
        point = QCursor::pos() - QPoint (m_frame -> width() / 2, m_frame -> height() / 2);
        if ( point.x() + m_frame -> width() > QApplication::desktop() -> width() )
            point.setX (QApplication::desktop() -> width() - m_frame -> width());
        if ( point.y() + m_frame -> height() > QApplication::desktop() -> height() )
            point.setY (QApplication::desktop() -> height() - m_frame -> height());
        if ( point.x() < 0 )
            point.setX (0);
        if ( point.y() < 0 )
            point.setY (0);
    }
    //CHANGED  kdDebug() << "Point: " << point.x() << "x" << point.y() << "\n";
    m_frame -> move (point);
    /*if ( kapp && kapp -> activeWindow() )
      {
        QMouseEvent me (QEvent::MouseButtonRelease, QPoint(0,0), QPoint(0,0), QMouseEvent::LeftButton, QMouseEvent::NoButton);
        QApplication::sendEvent (kapp -> activeWindow(), &me);
      }*/
    m_frame -> show();
    m_slider -> setFocus();
}

KPlayerSliderAction::KPlayerSliderAction (const QString& text, const KShortcut& cut,
        const QObject* receiver, const char* slot, KActionCollection* parent, const char* name)
        : KWidgetAction (new KPlayerSlider (Qt::Horizontal, 0, name), text, cut, receiver, slot, parent, name)
        //: KAction (text, 0, parent, name)
{
    setAutoSized (true);
    connect (slider(), SIGNAL (changed (int)), receiver, slot);
}

KPlayerSliderAction::~KPlayerSliderAction()
{
}

int KPlayerSliderAction::plug (QWidget* widget, int index)
{
    //Q_ASSERT (widget);
    //Q_ASSERT (! isPlugged());
    //Q_ASSERT (slider());
    //if ( ! slider() || ! widget || isPlugged() )
    //  return -1;
    //Q_ASSERT (widget -> inherits ("KToolBar"));
    //if ( ! widget -> inherits ("KToolBar") )
    //  return -1;
    //if ( kapp && ! kapp -> authorizeKAction (name()) )
    //  return -1;
    int result = KWidgetAction::plug (widget, index);
    if ( result < 0 )
        return result;
    KToolBar* toolbar = (KToolBar*) widget;
    //int id = getToolButtonID();
    //kdDebug() << "Orientation: " << toolbar -> orientation() << "\n";
    //m_slider -> reparent (toolbar, QPoint());
    //toolbar -> insertWidget (id, 0, m_slider, index);
    //toolbar -> setItemAutoSized (id, true);
    //QWhatsThis::remove (m_slider);
    //if ( ! whatsThis().isEmpty() )
    //  QWhatsThis::add (m_slider, whatsThis());
    //if ( ! text().isEmpty() )
    //  QToolTip::add (m_slider, text());
    //addContainer (toolbar, id);
    //setupToolbar (toolbar -> orientation(), toolbar);
    orientationChanged (toolbar -> orientation());
    connect (toolbar, SIGNAL (orientationChanged (Orientation)), this, SLOT (orientationChanged (Orientation)));
    //connect (toolbar, SIGNAL (destroyed()), this, SLOT (toolbarDestroyed()));
    //if ( parentCollection() )
    //  parentCollection() -> connectHighlight (toolbar, this);
    //return containerCount() - 1;
    return result;
}

void KPlayerSliderAction::unplug (QWidget* widget)
{
    //Q_ASSERT (m_slider);
    //Q_ASSERT (isPlugged());
    //Q_ASSERT (widget -> inherits ("KToolBar"));
    KWidgetAction::unplug (widget);
    if ( ! slider() || ! isPlugged() || widget != slider() -> parent() )
        return;
    //KToolBar* toolbar = (KToolBar*) widget;
    disconnect (widget, SIGNAL (orientationChanged (Orientation)), this, SLOT (orientationChanged (Orientation)));
    //disconnect (toolbar, SIGNAL (destroyed()), this, SLOT (toolbarDestroyed()));
    //m_slider -> reparent (0, QPoint());
    /*int index = findContainer (toolbar);
      if ( index == -1 )
        return;
      bar -> removeItem (menuId (index));
      removeContainer (index);*/
}

/*void KPlayerSliderAction::setupToolbar (Orientation orientation, KToolBar* toolbar)
{
    if ( orientation == Qt::Horizontal )
    {
//      toolbar -> setMinimumWidth (300);
//      toolbar -> setMinimumHeight (0);
      toolbar -> setFixedExtentWidth (300);
      toolbar -> setFixedExtentHeight (-1);
//      toolbar -> setHorizontallyStretchable (true);
//      toolbar -> setVerticallyStretchable (false);
    }
    else
    {
//      toolbar -> setMinimumWidth (0);
//      toolbar -> setMinimumHeight (300);
      toolbar -> setFixedExtentWidth (-1);
      toolbar -> setFixedExtentHeight (300);
//      toolbar -> setHorizontallyStretchable (false);
//      toolbar -> setVerticallyStretchable (true);
    }
}*/

void KPlayerSliderAction::orientationChanged (Qt::Orientation orientation)
{
    //if ( sender() && sender() -> inherits ("KToolBar") )
    //  setupToolbar (orientation, (KToolBar*) sender());
    //Q_ASSERT (m_slider);
    //Q_ASSERT (isPlugged());
    if ( slider() )
        slider() -> setOrientation (orientation);
}

/*void KPlayerSliderAction::toolbarDestroyed (void)
{
  if ( m_slider )
    m_slider -> reparent (0, QPoint());
}*/

KPlayerSlider::KPlayerSlider (Qt::Orientation orientation, QWidget* parent, const char* name)
//CHANGED  : QSlider (orientation, parent, name)
        : QSlider (300, 2200, 400, 1000, orientation, parent, name)
{
    m_changing_orientation = false;
    setTickmarks (QSlider::Both);
    connect (this, SIGNAL (valueChanged (int)), this, SLOT (sliderValueChanged (int)));
}

KPlayerSlider::~KPlayerSlider()
{
    //CHANGED  kdDebug() << "KPlayerSlider destroyed\n";
}

QSize KPlayerSlider::sizeHint() const
{
    QSize hint = QSlider::sizeHint();
    //CHANGED int length = kPlayerSettings() -> preferredSliderLength();
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

QSize KPlayerSlider::minimumSizeHint() const
{
    //kdDebug() << "KPlayerSlider minimum size hint\n";
    QSize hint = QSlider::minimumSizeHint();
    //CHANGED  int length = kPlayerSettings() -> minimumSliderLength();
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

void KPlayerSlider::setOrientation (Qt::Orientation o)
{
    if ( o == orientation() )
        return;
    m_changing_orientation = true;
    int minValue = QSlider::minValue();
    int maxValue = QSlider::maxValue();
    int value = QSlider::value();
    QSlider::setOrientation (o);
    QSlider::setMinValue (- maxValue);
    QSlider::setMaxValue (- minValue);
    QSlider::setValue (- value);
    m_changing_orientation = false;
}

int KPlayerSlider::minValue (void) const
{
    if ( orientation() == Qt::Horizontal )
        return QSlider::minValue();
    return - QSlider::maxValue();
}

void KPlayerSlider::setMinValue (int minValue)
{
    if ( orientation() == Qt::Horizontal )
        QSlider::setMinValue (minValue);
    else
        QSlider::setMaxValue (- minValue);
}

int KPlayerSlider::maxValue (void) const
{
    if ( orientation() == Qt::Horizontal )
        return QSlider::maxValue();
    return - QSlider::minValue();
}

void KPlayerSlider::setMaxValue (int maxValue)
{
    if ( orientation() == Qt::Horizontal )
        QSlider::setMaxValue (maxValue);
    else
        QSlider::setMinValue (- maxValue);
}

int KPlayerSlider::lineStep (void) const
{
    return QSlider::lineStep();
}

void KPlayerSlider::setLineStep (int lineStep)
{
    QSlider::setLineStep (lineStep);
}

int KPlayerSlider::pageStep (void) const
{
    return QSlider::pageStep();
}

void KPlayerSlider::setPageStep (int pageStep)
{
    QSlider::setPageStep (pageStep);
    setTickInterval (pageStep);
}

int KPlayerSlider::value (void) const
{
    if ( orientation() == Qt::Horizontal )
        return QSlider::value();
    return - QSlider::value();
}

void KPlayerSlider::setValue (int value, int)
{
    if ( orientation() == Qt::Horizontal )
        QSlider::setValue (value);
    else
        QSlider::setValue (- value);
}

void KPlayerSlider::setup (int minValue, int maxValue, int value, int pageStep, int lineStep)
{
    setMinValue (minValue);
    setMaxValue (maxValue);
    setLineStep (lineStep);
    setPageStep (pageStep);
    setValue (value);
}

void KPlayerSlider::sliderValueChanged (int)
{
    if ( ! m_changing_orientation )
        emit changed (value());
}
