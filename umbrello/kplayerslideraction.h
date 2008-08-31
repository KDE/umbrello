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
//Added by qt3to4:
#include <QFrame>
#include <QKeyEvent>

/**KPlayer's slider widget. Works around the Qt upside-down slider bug.
 * Taken from kplayer CVS 2003-09-21 (kplayer > 0.3.1) by Jonathan Riddell
  *@author kiriuja
  */
class KPlayerSlider : public QSlider
{
    Q_OBJECT

public:

    explicit KPlayerSlider (Qt::Orientation, QWidget* parent = 0, const char* name = 0);

    /** The KPlayerSlider destructor. Does nothing.
      */
    virtual ~KPlayerSlider();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    int minimum (void) const;
    void setMinimum (int);
    int maximum (void) const;
    void setMaximum (int);
    int singleStep (void) const;
    void setSingleStep (int);
    int pageStep (void) const;
    void setPageStep (int);
    int value (void) const;
    void setValue (int, int = 0); // do not override the virtual setValue

    void setup (int minimum, int maximum, int value, int pageStep, int lineStep = 1);

    virtual void setOrientation (Qt::Orientation);

signals:

    void changed (int);

protected slots:

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
    KPlayerPopupFrame (QWidget* parent = 0/*, const char* name = 0*/)
            : QFrame (parent, Qt::WType_Popup) { }

    /** The KPlayerPopupFrame destructor. Does nothing.
      */
    virtual ~KPlayerPopupFrame() { }

protected:

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
    KPlayerPopupSliderAction (const QObject* receiver, const char* slot, QObject *parent);

    /** The KPlayerPopupSliderAction destructor. Deletes the KPlayerPopupFrame.
      */
    virtual ~KPlayerPopupSliderAction();

    KPlayerSlider* slider (void)
    { return m_slider; }

    /** Plugs the action into the toolbar. Reparents the slider into the toolbar. */
    //virtual int plug (QWidget*, int = -1);
    /** Unplugs the action from the toolbar. Reparents the slider out of the toolbar. */
    //virtual void unplug (QWidget*);

protected slots:

    virtual void slotTriggered();

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
// class KPlayerSliderAction : public KAction
// {
//     Q_OBJECT
//
// public:
//     /** The KPlayerSliderAction constructor. Parameters are passed on to KAction.
//       */
//     KPlayerSliderAction (const QString& text, const KShortcut&, const QObject* receiver,
//                          const char* slot, KActionCollection* parent = 0, const char* name = 0);
//     /** The KPlayerSliderAction destructor. Does nothing.
//       */
//     virtual ~KPlayerSliderAction();
//
//     /** Returns a pointer to the KPlayerSlider object.
//       */
//     KPlayerSlider* slider (QWidget* parent)
//     { return m_slider; }
//
//     /** Plugs the slider into the toolbar.
//       */
// //     virtual int plug (QWidget* widget, int index = -1);
//     /** Unplugs the slider from the toolbar.
//       */
// //     virtual void unplug (QWidget* widget);
//
// protected slots:
//     /** Changes the slider orientation when the toolbar orientation changes.
//       */
//     void orientationChanged (Qt::Orientation);
//
// protected:
//     /** The slider.
//       */
//     KPlayerSlider* m_slider;
// };

#endif
