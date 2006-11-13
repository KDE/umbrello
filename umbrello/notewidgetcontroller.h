/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef NOTEWIDGETCONTROLLER_H
#define NOTEWIDGETCONTROLLER_H

#include "umlwidgetcontroller.h"

class NoteWidget;

/**
 * Controller for NoteWidget.
 *
 * MouseMove and MouseRelease execute the base code and then specific code
 * for note widget.
 *
 * Double click behaviour is edit the text of the note.
 *
 * @author Umbrello UML Modeller Authors <uml-devel@lists.sourceforge.net>
 */
class NoteWidgetController : public UMLWidgetController {
public:

    /**
     * Constructor for NoteWidgetController.
     *
     * @param noteWidget The NoteWidget which uses the controller.
     */
    NoteWidgetController(NoteWidget* noteWidget);

    /**
     * Destructor for NoteWidgetController.
     */
    virtual ~NoteWidgetController();

    /**
     * Overriden from UMLWidgetController.
     * Handles a mouse move event.
     * Executes base code and then sets the geometry of the editor.
     *
     * @param me The QMouseEvent event.
     */
    virtual void mouseMoveEvent(QMouseEvent* me);

    /**
     * Overriden from UMLWidgetController.
     * Handles a mouse release event.
     * Executes base code and then draws the text in the note.
     *
     * @param me The QMouseEvent event.
     */
    virtual void mouseReleaseEvent(QMouseEvent * me);

protected:

    /**
     * Overriden from UMLWidgetController.
     * Executes the action for double click in the widget.
     * Shows the dialog to change the text of the note.
     *
     * @param me The QMouseEvent which triggered the double click event.
     */
    virtual void doMouseDoubleClick(QMouseEvent *me);

private:

    /**
     * The note widget which uses the controller.
     */
    NoteWidget* m_noteWidget;
};

#endif
