/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003 Brian Thomas  <brian.thomas@gsfc.nasa.gov>         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QLabel>
//#include <QtGui/QTextEdit>
#include <q3textedit.h>
#include <q3popupmenu.h>

#include <kmenu.h>

#include "codeviewerstate.h"
#include "textblocklist.h"

class UMLObject;

class CodeViewerDialog;
class CodeComment;
class CodeDocument;
class CodeClassFieldDeclarationBlock;
class CodeMethodBlock;
class CodeBlockWithComments;
class HierarchicalCodeBlock;
class TextBlock;


class ParaInfo
{
public:
    int start; // this is a relative offset from the beginning of the tblock
    int size;
    QColor fgcolor;
    QColor bgcolor;
    bool isEditable;

    ParaInfo () { isEditable = false; }
};


class TextBlockInfo
{
public:
    QList<ParaInfo*> m_paraList;
    UMLObject * m_parent;
    QString displayName;
    bool isClickable;
    bool isCodeAccessorMethod;

    TextBlockInfo () { m_parent = 0; isClickable = false; isCodeAccessorMethod = false; }
    void setParent(UMLObject *p = 0) { m_parent = p; }
    UMLObject * getParent() { return m_parent; }
};


class CodeEditor : public Q3TextEdit
{
    Q_OBJECT
public:

    explicit CodeEditor ( const QString & text, const QString & context = QString(),
                          CodeViewerDialog * parent = 0, const char * name = 0, CodeDocument * doc = 0);
    explicit CodeEditor ( CodeViewerDialog * parent, const char* name = 0, CodeDocument * doc = 0);
    ~CodeEditor ();

    // return code viewer state
    Settings::CodeViewerState getState( );

protected:

    bool close ();

    // various methods for appending various types of text blocks in the editor.
    void appendText (TextBlock * tblock);
    void appendText (HierarchicalCodeBlock * hblock);
    void appendText (CodeClassFieldDeclarationBlock * db);
    void appendText (TextBlockList * items);
    void appendText (CodeMethodBlock * mb);
    void appendText (CodeComment * comment, TextBlock * parent, UMLObject * umlObj = 0, const QString & compName = "");
    void appendText (CodeBlockWithComments * cb );

    // Rebuild our view of the document. Happens whenever we change
    // some field/aspect of an underlying UML object used to create
    // the view.
    // If connections are right, then the UMLObject will send out the modified()
    // signal which will trigger a call to re-generate the appropriate code within
    // the code document. Our burden is to appropriately prepare the tool: we clear
    // out ALL the textblocks in the QTextEdit widget and then re-show them
    // after the dialog disappears
    void rebuildView( int startCursorPos );

    // override the QT event so we can do appropriate things
    void contentsMouseMoveEvent ( QMouseEvent * e );

    // implemented so we may capture certain key presses, namely backspace
    // and 'return' events.
    void keyPressEvent ( QKeyEvent * e );

    // (re) load the parent code document into the editor
    void loadFromDocument();

    // specialized popup menu for our tool
    Q3PopupMenu * createPopupMenuAt(const QPoint & pos);
    KMenu * createPopup();

//    virtual void mousePressEvent(QMouseEvent * event);

private:

    QString m_parentDocName;
    CodeDocument * m_parentDoc;
    CodeViewerDialog * m_parentDlg;

    int m_lastPara;
    int m_lastPos;

    bool m_newLinePressed;
    bool m_backspacePressed;
    bool m_isHighlighted;
    bool m_showHiddenBlocks;

    TextBlock * m_textBlockToPaste;
    TextBlock * m_selectedTextBlock;
    TextBlock * m_lastTextBlockToBeEdited;

    QMap<TextBlock*, TextBlockInfo*> m_tbInfoMap;
    TextBlockList m_textBlockList;

    // main insert routine. Will append if startline is not supplied.
    void insertText (const QString & text, TextBlock * parent, bool isEditable = false,
                 const QColor & fgcolor = QColor("black"), const QColor & bgcolor = QColor("white"),
                 UMLObject * umlobj = 0, const QString & displayName = "", int startLine = -1);

    void editTextBlock(TextBlock * tBlock, int para);
    void clearText();
    QLabel * getComponentLabel();
    bool paraIsNotSingleLine (int para);
    void expandSelectedParagraph( int where );
    void contractSelectedParagraph( int where );
    void updateTextBlockFromText (TextBlock * block);

    void initText ( CodeDocument * doc );
    void init ( CodeViewerDialog * parentDlg, CodeDocument * parentDoc );

    void changeTextBlockHighlighting(TextBlock * tb, bool selected);
    bool isParaEditable (int para);
    bool textBlockIsClickable(UMLObject * obj);

    // return whether or not the passed string is empty or
    // contains nothing but whitespace
    static bool stringIsBlank( const QString &str );

public slots:

    void insertParagraph ( const QString & text, int para );
    void removeParagraph ( int para );
    void changeHighlighting(int signal);
    void changeShowHidden (int signal);
    void slotRedrawText();

protected slots:

    void clicked(int para, int pos );
    void doubleClicked(int para, int pos );
    void cursorPositionChanged(int para, int pos );
    void slotCopyTextBlock ( );
    void slotCutTextBlock ( );
    void slotPasteTextBlock ( );
    void slotChangeSelectedBlockView();
    void slotChangeSelectedBlockCommentView();
    void slotInsertCodeBlockAfterSelected();
    void slotInsertCodeBlockBeforeSelected();

signals:

    /*
        void sigNewLinePressed ();
        void sigBackspacePressed ();
    */

};


#endif // CODEEDITOR_H
