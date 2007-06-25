/***************************************************************************
                          kplayerslideraction.h
                          ---------------------
    begin                : Sat Jan 11 2003
    copyright            : (C) 2003 by kiriuja
    email                : kplayer-dev@en-directo.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERSLIDERACTION_H
#define KPLAYERSLIDERACTION_H

#include <kaction.h>
#include <qslider.h>

/**KPlayer's slider widget. Works around the Qt upside-down slider bug.
 * Taken from kplayer CVS 2003-09-21 (kplayer > 0.3.1) by Jonathan Riddell
  *@author kiriuja
  */
class KPlayerSlider : public QSlider
{
    Q_OBJECT

public:
    /** The KPlayerSlider constructor. Parameters are passed on to QSlider.
      */
    explicit KPlayerSlider (Qt::Orientation, QWidget* parent = 0, const char* name = 0);
    /** The KPlayerSlider destructor. Does nothing.
      */
    virtual ~KPlayerSlider();

    /** The size hint.
     */
    virtual QSize sizeHint() const;
    /** The minimum size hint.
     */
    virtual QSize minimumSizeHint() const;

    /** The minimum value.
      */
    int minValue (void) const;
    /** Sets the minimum value.
      */
    void setMinValue (int);
    /** The maximum value.
      */
    int maxValue (void) const;
    /** Sets the maximum value.
      */
    void setMaxValue (int);
    /** The line step.
      */
    int lineStep (void) const;
    /** Sets the line step.
      */
    void setLineStep (int);
    /** The page step.
      */
    int pageStep (void) const;
    /** Sets the page step.
      */
    void setPageStep (int);
    /** The current value.
      */
    int value (void) const;
    /** Sets the current value. The extra parameter prevents overriding of the virtual QSlider::setValue.
      */
    void setValue (int, int = 0); // do not override the virtual setValue

    /** Sets up the slider by setting five options in one go.
      */
    void setup (int minValue, int maxValue, int value, int pageStep, int lineStep = 1);
    /** Sets the slider orientation.
      */
    virtual void setOrientation (Qt::Orientation);

signals:
    /** Emitted when the slider value changes.
      */
    void changed (int);

protected slots:
    /** Receives the valueChanged signal from QSlider.
      */
    void sliderValueChanged (int);

protected:
    // Recursion prevention. Should be private.
    bool m_changing_orientation;

    friend class KPlayerSliderAction;
    friend class KPlayerPopupSliderAction;
};

/**KPlayer popup frame.
  *@author kiriuja
  */
class KPlayerPopupFrame : public QFrame
{
    Q_OBJECT

public:
    /** The KPlayerPopupFrame constructor. Parameters are passed on to QFrame.
      */
    KPlayerPopupFrame (QWidget* parent = 0, const char* name = 0)
            : QFrame (parent, name, Qt::WType_Popup) { }
    /** The KPlayerPopupFrame destructor. Does nothing.
      */
    virtual ~KPlayerPopupFrame() { }

protected:
    /** Closes the popup frame when Alt, Tab, Esc, Enter or Return is pressed.
      */
    virtual void keyPressEvent (QKeyEvent*);
};

/**Action representing a popup slider activated by a toolbar button.
  *@author kiriuja
  */
class KPlayerPopupSliderAction : public KAction
{
    Q_OBJECT

public:
    /** The KPlayerPopupSliderAction constructor. Parameters are passed on to KAction.
      */
    KPlayerPopupSliderAction (const QString& text, const QString& pix, const KShortcut& shortcut,
                              const QObject* receiver, const char* slot, QObject* parent = 0, const char* name = 0);
    /** The KPlayerPopupSliderAction destructor. Deletes the KPlayerPopupFrame.
      */
    virtual ~KPlayerPopupSliderAction();

    /** Returns a pointer to the KPlayerSlider object.
      */
    KPlayerSlider* slider (void)
    { return m_slider; }

    /** Plugs the action into the toolbar. Reparents the slider into the toolbar. */
    //virtual int plug (QWidget*, int = -1);
    /** Unplugs the action from the toolbar. Reparents the slider out of the toolbar. */
    //virtual void unplug (QWidget*);

protected slots:
    /** Pops up the slider.
      */
    virtual void slotActivated (void);

protected:
    /** The slider.
      */
    KPlayerSlider* m_slider;
    /** The popup frame.
      */
    KPlayerPopupFrame* m_frame;
};

/**Slider action suitable for insertion into a toolbar.
  *@author kiriuja
  */
class KPlayerSliderAction : public KWidgetAction
{
    Q_OBJECT

public:
    /** The KPlayerSliderAction constructor. Parameters are passed on to KAction.
      */
    KPlayerSliderAction (const QString& text, const KShortcut&, const QObject* receiver,
                         const char* slot, KActionCollection* parent = 0, const char* name = 0);
    /** The KPlayerSliderAction destructor. Does nothing.
      */
    virtual ~KPlayerSliderAction();

    /** Returns a pointer to the KPlayerSlider object.
      */
    KPlayerSlider* slider (void)
    { return (KPlayerSlider*) widget(); }

    /** Plugs the slider into the toolbar.
      */
    virtual int plug (QWidget* widget, int index = -1);
    /** Unplugs the slider from the toolbar.
      */
    virtual void unplug (QWidget* widget);

protected slots:
    /** Changes the slider orientation when the toolbar orientation changes.
      */
    void orientationChanged (Qt::Orientation);

protected:
    /** The slider.
      */
    //KPlayerSlider* m_slider;
};

#endif
