
/***************************************************************************
                          codeviewerdialog.h  -  description
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

#ifndef CODEVIEWERDIALOG_H
#define CODEVIEWERDIALOG_H

#include <qcolor.h>
#include <qmap.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qtextedit.h>
#include "codeviewerdialogbase.h"
#include "settingsdlg.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QFrame;
class QTextBrowser;

class CodeComment;
class CodeDocument;
class CodeClassFieldDeclarationBlock;
class CodeMethodBlock;
class HierarchicalCodeBlock;

class TextBlockInfo;
class TextBlock;
class ParaInfo;

class UMLObject;

/** This class is sooo ugly I dont know where to begin. For now, its a prototype
  * that works, and thats all we need. In the future, a re-write is mandated to 
  * bring a bit of beauty to this beast. -b.t.
  */
class CodeViewerDialog : public CodeViewerDialogBase
{
    Q_OBJECT
public:

	CodeViewerDialog ( QWidget* parent, CodeDocument * doc, SettingsDlg::CodeViewerState state,
                           const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~CodeViewerDialog ();

	/** return the code viewer state */
	SettingsDlg::CodeViewerState getState( );

protected:

	void insert (const QString & text, TextBlock * parent, bool editable = false, const QColor & fgcolor = QColor("black"), const QColor & bgcolor = QColor("white"), UMLObject * umlobj = 0, const QString & displayName = ""); 

	void loadFromDocument(CodeDocument * doc); 
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

	bool close ( bool alsoDelete ); 

private:

	QMap<int, TextBlock*> *m_paraInfoMap;
	QMap<TextBlock*, TextBlockInfo*> *m_tbInfoMap;
	QPtrList<TextBlock> m_textBlockList;

	SettingsDlg::CodeViewerState m_state;
	QString parentDocName;

	bool m_isHighlighted;
	TextBlock * m_selectedTextBlock;
	TextBlock * m_lastTextBlockToBeEdited;
	CodeDocument * m_parentDocument;

	bool paraIsNotSingleLine (int para);
	void expandSelectedParagraph( int where ); 
	void contractSelectedParagraph( int where ); 
	void updateMethodBlockBody (TextBlock * block);
	void initText ( CodeDocument * doc ); 
	void changeTextBlockHighlighting(TextBlock * tb, bool selected);
	bool isParaEditable (int para);
	bool textBlockIsClickable(UMLObject * obj); 
	int m_lastPara;
	int m_lastPos;
	bool m_newLinePressed;
	bool m_backspacePressed;

public slots:

	void newLinePressed( );
	void backspacePressed( );
	void changeHighlighting(int signal);

protected slots:

	void clear();
	void clicked(int para, int pos ); 
	void doubleClicked(int para, int pos ); 
	void cursorPositionChanged(int para, int pos ); 
	void mouseMoved ( QMouseEvent * e );
    	virtual void languageChange();

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

#endif // CODEVIEWERDIALOG_H
