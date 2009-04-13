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

#ifndef KPLAYERSLIDERACTION_H
#define KPLAYERSLIDERACTION_H

#include <kaction.h>

#include <QtGui/QSlider>
#include <QtGui/QFrame>

class QKeyEvent;

/**
 * KPlayer's slider widget. Works around the Qt upside-down slider bug.
 * Taken from kplayer CVS 2003-09-21 (kplayer > 0.3.1) by Jonathan Riddell
 * @author kiriuja
 */
class KPlayerSlider : public QSlider
{
    Q_OBJECT

public:
    explicit KPlayerSlider (Qt::Orientation, QWidget* parent = 0);
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
    void setValue (int, int = 0);

    void setup (int minimum, int maximum, int value, int pageStep, int lineStep = 1);

    virtual void setOrientation (Qt::Orientation);

signals:
    void changed (int);  ///< Emitted when the slider value changes.

protected slots:
    void sliderValueChanged (int);

protected:
    bool m_changing_orientation;  ///< Recursion prevention. Should be private.

    friend class KPlayerSliderAction;
    friend class KPlayerPopupSliderAction;
};

/**
 * KPlayer popup frame.
 * @author kiriuja
 */
class KPlayerPopupFrame : public QFrame
{
    Q_OBJECT

public:
    KPlayerPopupFrame (QWidget* parent = 0);
    virtual ~KPlayerPopupFrame();

protected:
    virtual void keyPressEvent (QKeyEvent*);
};

/**
 * Action representing a popup slider activated by a toolbar button.
 * @author kiriuja
 */
class KPlayerPopupSliderAction : public KAction
{
    Q_OBJECT

public:
    KPlayerPopupSliderAction (const QObject* receiver, const char* slot, QObject *parent);
    virtual ~KPlayerPopupSliderAction();

    /** Returns a pointer to the KPlayerSlider object. */
    KPlayerSlider* slider() { return m_slider; }

    /** Plugs the action into the toolbar. Reparents the slider into the toolbar. */
    //virtual int plug (QWidget*, int = -1);
    /** Unplugs the action from the toolbar. Reparents the slider out of the toolbar. */
    //virtual void unplug (QWidget*);

protected slots:
    virtual void slotTriggered();

protected:
    KPlayerSlider*      m_slider;  ///< The slider.
    KPlayerPopupFrame*  m_frame;   ///< The popup frame.
};

// /**
//  * Slider action suitable for insertion into a toolbar.
//  * @author kiriuja
//  */
// class KPlayerSliderAction : public KAction
// {
//     Q_OBJECT
//
// public:
//     KPlayerSliderAction (const QString& text, const KShortcut&, const QObject* receiver,
//                          const char* slot, KActionCollection* parent = 0, const char* name = 0);
//     virtual ~KPlayerSliderAction();
//
//     /** Returns a pointer to the KPlayerSlider object. */
//     KPlayerSlider* slider (QWidget* parent) { return m_slider; }
//
// //     virtual int plug (QWidget* widget, int index = -1);
// //     virtual void unplug (QWidget* widget);
//
// protected slots:
//     void orientationChanged (Qt::Orientation);
//
// protected:
//     KPlayerSlider* m_slider;  ///< The slider.
// };

#endif
