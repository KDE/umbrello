/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <brian.thomas@gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include "codeviewerstate.h"
#include "textblocklist.h"

#include <ktextedit.h>

#include <QList>
#include <QString>

class CodeViewerDialog;
class CodeComment;
class CodeDocument;
class CodeClassFieldDeclarationBlock;
class CodeMethodBlock;
class CodeBlockWithComments;
class HierarchicalCodeBlock;
class KMenu;
class QLabel;
class TextBlock;
class UMLObject;

class ParaInfo
{
public:
    int start; // this is a relative offset from the beginning of the tblock
    int size;         ///< number of characters
    int end;          ///< end line number
    QColor fgcolor;
    QColor bgcolor;
    bool isEditable;

    ParaInfo ()
      : start(0),
        size(0),
        end(0),
        isEditable(false) { }
};


class TextBlockInfo
{
public:
    QList<ParaInfo*> m_paraList;
    UMLObject *      m_parent;
    QString          m_displayName;
    bool isClickable;
    bool isCodeAccessorMethod;

    TextBlockInfo () { m_parent = 0; isClickable = false; isCodeAccessorMethod = false; }
    void setParent(UMLObject *p = 0) { m_parent = p; }
    UMLObject * parent() { return m_parent; }
    void setDisplayName(const QString& name) { m_displayName = name; }
    QString displayName() { return m_displayName; }
};


class CodeEditor : public KTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(const QString & text, CodeViewerDialog * parent = 0, CodeDocument * doc = 0);
    explicit CodeEditor(CodeViewerDialog * parent = 0, CodeDocument * doc = 0);
    ~CodeEditor();

    Settings::CodeViewerState state();

protected:
    bool close ();

    void appendText(TextBlock * tblock);
    void appendText(HierarchicalCodeBlock * hblock);
    void appendText(CodeClassFieldDeclarationBlock * db);
    void appendText(TextBlockList * items);
    void appendText(CodeMethodBlock * mb);
    void appendText(CodeComment * comment, TextBlock * parent, UMLObject * umlObj = 0, const QString & compName = QString());
    void appendText(CodeBlockWithComments * cb);

    void rebuildView(int startCursorPos);

    void contextMenuEvent(QContextMenuEvent * event);
    void contentsMouseMoveEvent(QMouseEvent * e);
    void keyPressEvent(QKeyEvent * e);
    void mouseDoubleClickEvent(QMouseEvent * e);

    void loadFromDocument();

#if QT_VERSION >= 0x050000
    QMenu * createPopup();
#else
    KMenu * createPopup();
#endif

private:
    QString            m_parentDocName;
    CodeDocument *     m_parentDoc;
    CodeViewerDialog * m_parentDialog;

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

    void insertText(const QString & text, TextBlock * parent, bool isEditable = false,
                 const QColor & fgcolor = QColor("black"), const QColor & bgcolor = QColor("white"),
                 UMLObject * umlobj = 0, const QString & displayName = QString(), int startLine = -1);

    void editTextBlock(TextBlock * tBlock, int para);
    void clearText();
    QLabel * componentLabel();
    bool paraIsNotSingleLine(int para);
    void expandSelectedParagraph(int where);
    void contractSelectedParagraph(int where);
    void updateTextBlockFromText(TextBlock * block);

    void initText(CodeDocument * doc);
    void init(CodeViewerDialog * parentDialog, CodeDocument * parentDoc);

    void changeTextBlockHighlighting(TextBlock * tb, bool selected);
    void setParagraphBackgroundColor(int position, const QColor& color);
    bool isParaEditable(int para);
    bool textBlockIsClickable(UMLObject * obj);

    TextBlock* findTextBlockAt(int characterPos);

    void clicked(int para, int pos);

    static bool isNonBlank(const QString &str);

public slots:
    void insertParagraph(const QString & text, int para);
    void removeParagraph(int para);
    void changeHighlighting(int signal);
    void changeShowHidden(int signal);
    void slotRedrawText();

protected slots:
    void slotCursorPositionChanged();
    void slotCopyTextBlock();
    void slotCutTextBlock();
    void slotPasteTextBlock();
    void slotChangeSelectedBlockView();
    void slotChangeSelectedBlockCommentView();
    void slotInsertCodeBlockAfterSelected();
    void slotInsertCodeBlockBeforeSelected();

signals:
    // void sigNewLinePressed();
    // void sigBackspacePressed();

};

#endif // CODEEDITOR_H
