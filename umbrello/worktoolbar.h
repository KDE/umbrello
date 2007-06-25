/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef WORKTOOLBAR_H
#define WORKTOOLBAR_H

#include <qmap.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <ktoolbar.h>

#include "umlnamespace.h"

class QMainWindow;


/**
 * This is the toolbar that is displayed on the right-hand side of the program
 * window.  For each type of diagram it will change to suit that document.
 *
 * To add a new tool button do the following:
 * - create a button pixmap (symbol)
 * - create a cursor pixmap
 * - add an element to the ToolBar_Buttons enum
 * - adjust function loadPixmaps
 * - adjust function slotCheckToolBar
 *
 * @short The toolbar that is different for each type of diagram.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */


class WorkToolBar : public KToolBar {
    Q_OBJECT
public:

    /**
     * Creates a work tool bar.
     *
     * @param parentWindow      The parent of the toolbar.
     */
    WorkToolBar(QMainWindow *parentWindow, const char *name);

    /**
     * Standard deconstructor.
     */
    ~WorkToolBar();

    /**
     * Sets the current tool to the previously used Tool. This is just
     * as if the user had pressed the button for the other tool.
     */
    void setOldTool();

    /**
     * Sets the current tool to the default tool. (select tool)
     * Calling this function is as if the user had pressed the "arrow"
     * button on the toolbar.
     */
    void setDefaultTool();

    /**
     * Enumeration of all available toolbar buttons.
     */
    enum ToolBar_Buttons
    {
        tbb_Undefined = -1,
        tbb_Arrow,
        tbb_Generalization,
        tbb_Aggregation,
        tbb_Dependency,
        tbb_Association,
        tbb_Containment,
        tbb_Coll_Message,
        tbb_Seq_Message_Synchronous,
        tbb_Seq_Message_Asynchronous,
        tbb_Composition,
        tbb_Relationship,
        tbb_UniAssociation,
        tbb_State_Transition,
        tbb_Activity_Transition,
        tbb_Anchor,//keep anchor as last association until code uses better algorithm for testing
        tbb_Note,
        tbb_Box,
        tbb_Text,
        tbb_Actor,
        tbb_UseCase,
        tbb_Class,
        tbb_Interface,
        tbb_Datatype,
        tbb_Enum,
        tbb_Entity,
        tbb_Package,
        tbb_Component,
        tbb_Node,
        tbb_Artifact,
        tbb_Object,
        tbb_Initial_State,
        tbb_State,
        tbb_End_State,
        tbb_Initial_Activity,
        tbb_Activity,
        tbb_End_Activity,
        tbb_Branch,
        tbb_Fork,
        tbb_DeepHistory,
        tbb_ShallowHistory,
        tbb_Join,
        tbb_StateFork,
        tbb_Junction,
        tbb_Choice,
        tbb_Andline
    };

private:

    typedef QMap<Uml::Diagram_Type,ToolBar_Buttons> OldToolMap;

    /**
     * This inner class holds label, symbol, and cursor of a tool button.
     */
    class ToolButton {
    public:
        QString Label;
        QPixmap Symbol;
        QCursor Cursor;
        ToolButton() : Label(QString("?")), Symbol(QPixmap()), Cursor(QCursor()) { }
        ToolButton(const QString& lbl, const QPixmap& smb, const QCursor& cur) :
        Label(lbl), Symbol(smb), Cursor(cur) { }
    };

    typedef QMap<ToolBar_Buttons, ToolButton> ToolButtonMap;

    ToolBar_Buttons     m_CurrentButtonID;
    OldToolMap          m_map;
    Uml::Diagram_Type   m_Type;
    ToolButtonMap       m_ToolButtons;

    /**
     * Loads a pixmap from file
     */
    QPixmap load(const QString &fileName);

    /**
     * Loads toolbar icon and mouse cursor images from disk
     */
    void loadPixmaps();

    /**
     * Returns the current cursor depending on m_CurrentButtonID
     */
    QCursor currentCursor();

    /**
     * Inserts the button corresponding to the tbb value given
     * and activates the toggle.
     */
    void insertHotBtn(ToolBar_Buttons tbb);

    /**
     * Inserts most associations, just reduces some string
     * duplication (nice to translators)
     */
    void insertBasicAssociations();

signals:
    void sigButtonChanged(int);
public slots:
    void slotCheckToolBar(Uml::Diagram_Type dt);
    void buttonChanged(int b);
    void slotResetToolBar();
};

#endif
