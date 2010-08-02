/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2010                                               *
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
class NoteWidgetController : public UMLWidgetController
{
public:

    NoteWidgetController(NoteWidget* noteWidget);
    virtual ~NoteWidgetController();

protected:

    virtual void doMouseDoubleClick(QMouseEvent *me);

private:

    NoteWidget* m_noteWidget;  ///< The note widget which uses the controller.

};

#endif
