
/***************************************************************************
                          codeeditor.h  -  description
                             -------------------
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Brian Thomas
    email                : brian.thomas@gsfc.nasa.gov
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <qstring.h>
#include <qtextedit.h>
#include "settingsdlg.h"
#include "codeviewerdialog.h"

class CodeComment;
class CodeDocument;
class CodeClassFieldDeclarationBlock;
class CodeMethodBlock;
class HierarchicalCodeBlock;

class TextBlockInfo;
class TextBlock;
class ParaInfo;

class CodeEditor : public QTextEdit
{
    Q_OBJECT
public:

	CodeEditor ( const QString & text, const QString & context = QString::null, CodeViewerDialog * parent = 0, const char * name = 0 , CodeDocument * doc = 0);
	CodeEditor ( CodeViewerDialog * parent, const char* name = 0, CodeDocument * doc = 0);
	~CodeEditor ();

	// return code viewer state
	SettingsDlg::CodeViewerState getState( );

protected:

	bool close ( bool alsoDelete ); 

        void insert (const QString & text, TextBlock * parent, bool editable = false, const QColor & fgcolor = QColor("black"), 
                     const QColor & bgcolor = QColor("white"), UMLObject * umlobj = 0, const QString & displayName = "");

        void insertText (TextBlock * tblock);
        void insertText (HierarchicalCodeBlock * hblock);
        void insertText (CodeClassFieldDeclarationBlock * db );
        void insertText (QPtrList<TextBlock> * items);
        void insertText (CodeMethodBlock * mb);
        void insertText (CodeComment * comment, TextBlock * parent, UMLObject * umlObj = 0, const QString & compName="");

        void editTextBlock(TextBlock * tBlock, int para);

        // Rebuild our view of the document. Happens whenever we change
        // some field/aspect of an underlying UML object used to create
        // the view.
        // If connections are right, then the UMLObject will send out the modified()
        // signal which will trigger a call to re-generate the appropriate code within
        // the code document. Our burden is to appropriately prepare the tool: we clear
        // out ALL the textblocks in the QTextEdit widget and then re-show them
        // after the dialog disappears
        void rebuildView( int startCursorPos );

	void contentsMouseMoveEvent ( QMouseEvent * e );

	// implemented so we may capture certain key presses, namely backspace
	// and 'return' events.
	void keyPressEvent ( QKeyEvent * e );

        void loadFromDocument();

private:

	QString parentDocName;
	CodeDocument * m_parentDoc;
	CodeViewerDialog * m_parentDlg;

	bool paraIsNotSingleLine (int para);
        void expandSelectedParagraph( int where );
        void contractSelectedParagraph( int where );
        void updateMethodBlockBody (TextBlock * block);
        void initText ( CodeDocument * doc );
	void init ( CodeViewerDialog * parentDlg, CodeDocument * parentDoc );
        void changeTextBlockHighlighting(TextBlock * tb, bool selected);
        bool isParaEditable (int para);
        bool textBlockIsClickable(UMLObject * obj);
        int m_lastPara;
        int m_lastPos;
        bool m_newLinePressed;
        bool m_backspacePressed;

        bool m_isHighlighted;
        TextBlock * m_selectedTextBlock;
        TextBlock * m_lastTextBlockToBeEdited;

        QMap<int, TextBlock*> *m_paraInfoMap;
        QMap<TextBlock*, TextBlockInfo*> *m_tbInfoMap;
        QPtrList<TextBlock> m_textBlockList;

	QLabel * getComponentLabel();

	void clearText();

public slots:

	void insertParagraph ( const QString & text, int para );
	void removeParagraph ( int para );
        void changeHighlighting(int signal);


protected slots:

        void clicked(int para, int pos );
        void doubleClicked(int para, int pos );
        void cursorPositionChanged(int para, int pos );

signals:

/*
	void sigNewLinePressed ();
	void sigBackspacePressed ();
*/

};

class ParaInfo {
public:
        int start; // this is a relative offset from the beginning of the tblock
        int size;
        QColor fgcolor;
        QColor bgcolor;
        bool isEditable;

        ParaInfo () { isEditable = false; }
};

class TextBlockInfo {
public:
        QPtrList<ParaInfo> m_paraList;
        UMLObject * m_parent;
        QString displayName;
        bool isClickable;

        TextBlockInfo () { m_parent = 0; isClickable = false; }
        void setParent(UMLObject *p = 0) { m_parent = p; }
        UMLObject * getParent() { return m_parent; }

};

#endif // CODEEDITOR_H
