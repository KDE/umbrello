/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003  Brian Thomas  <brian.thomas@gsfc.nasa.gov>        *
 *   copyright (C) 2004-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codeeditor.h"

// local includes
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "umldoc.h"
#include "umlrole.h"

#include "codeaccessormethod.h"
#include "codeclassfield.h"
#include "codeclassfielddeclarationblock.h"
#include "codedocument.h"
#include "codeoperation.h"
#include "codemethodblock.h"
#include "classifiercodedocument.h"
#include "ownedhierarchicalcodeblock.h"
#include "codegenfactory.h"

#include "codeviewerdialog.h"
#include "classpropdlg.h"
#include "umlattributedialog.h"
#include "umlroledialog.h"
#include "umloperationdialog.h"

// kde includes
#include <kaction.h>
#include <klocale.h>
#include <kmenu.h>

// qt includes
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QKeySequence>
#include <QtGui/QMenu>
#include <QtGui/QColor>
#include <QtGui/QLabel>
#include <QtGui/QBrush>
#include <QtGui/QLayout>
#include <QtCore/QPointer>
#include <QtCore/QRegExp>

/**
 * Constructor.
 */
CodeEditor::CodeEditor(const QString & text, CodeViewerDialog * parent, CodeDocument * doc)
  : KTextEdit(text, parent)
{
    init(parent, doc);
}

/**
 * Constructor.
 */
CodeEditor::CodeEditor(CodeViewerDialog * parent, CodeDocument * doc)
  : KTextEdit(parent)
{
    init(parent, doc);
}

/**
 * Destructor.
 */
CodeEditor::~CodeEditor()
{
}

/**
 * Clear the display of all text.
 */
void CodeEditor::clearText()
{
    m_selectedTextBlock = 0;

    // now call super-class
    clear();

    // uDebug() << "text block list size=" << m_textBlockList.size();
    while (!m_textBlockList.isEmpty()) {
        /*;TODO:? delete */ m_textBlockList.takeFirst();
    }
    m_tbInfoMap.clear();
}

/**
 * Return code viewer state.
 * @return   state of the code viewer
 */
Settings::CodeViewerState CodeEditor::state()
{
    return m_parentDlg->state();
}

/**
 * Return the label on the dialog window. Some info can be shown.
 * @return   label widget of dialog
 */
QLabel * CodeEditor::componentLabel()
{
    return m_parentDlg->ui_componentLabel;
}

/**
 * TODO: Used only for debugging right now.
 * int para = tc.position();     --> is the character in the editor
 * int pos  = tc.blockNumber();  --> is the row in the editor
 */
void CodeEditor::clicked(int para, int pos)
{
    QString txt = "position:" + QString::number(para) +
                  " / row (block):" + QString::number(pos);
    if (m_parentDlg->ui_highlightCheckBox->isChecked()) {
        TextBlock* tb = findTextBlockAt(para);
        if (tb) {
            TextBlockInfo* info = m_tbInfoMap[tb];
            if (info) {
                txt += " / <b>" + info->displayName() + "</b>";
            }
        }
    }
    componentLabel()->setText(txt);

}

/**
 * Slot which closes this widget. Returns true if the widget was closed;
 * otherwise returns false.
 * Reimplemented from QWidget.
 * @return   state of close action
 */
bool CodeEditor::close()
{
    // capture last code block, if it exists
    if (m_lastTextBlockToBeEdited) {
        updateTextBlockFromText (m_lastTextBlockToBeEdited);
        m_lastTextBlockToBeEdited = 0;
    }
    return KTextEdit::close();
}

/**
 * Allow us to edit, as appropriate, the parent UMLObject of the
 * given text block.
 */
void CodeEditor::editTextBlock(TextBlock * tBlock, int para)
{
    if (tBlock) {
        TextBlockInfo *info = m_tbInfoMap[tBlock];
        if (info) {
            UMLObject *obj = info->parent();
            if (obj) {
                UMLAttribute * at = NULL;
                UMLRole * role = NULL;
                UMLOperation * op = NULL;

                if ( (at = dynamic_cast<UMLAttribute*>(obj)) ) {
                    QPointer<UMLAttributeDialog> dlg = new UMLAttributeDialog(this, at);
                    if ( dlg->exec() == KDialog::Accepted ) {
                        rebuildView(para);
                    }
                    delete dlg;
                }
                else if ( (dynamic_cast<UMLClassifier*>(obj)) ) {
                    if (obj->showPropertiesPagedDialog()) {
                        rebuildView(para);
                    }
                }
                else if ( (role = dynamic_cast<UMLRole*>(obj))) {
                    QPointer<UMLRoleDialog> dlg = new UMLRoleDialog(this, role);
                    if ( dlg->exec() == KDialog::Accepted ) {
                        rebuildView(para);
                    }
                    delete dlg;
                }
                else if ( (op = dynamic_cast<UMLOperation*>(obj)) ) 
                    //else if( (cop = dynamic_cast<CodeOperation*>(tBlock)) )
                {
                    QPointer<UMLOperationDialog> dlg = new UMLOperationDialog(this, op);
                    if ( dlg->exec() == KDialog::Accepted ) {
                        rebuildView(para);
                    }
                    delete dlg;
                }
                else {
                    uError() << "UNKNOWN parent for textBlock";
                }
            }
        }
    }
    else {
        DEBUG(DBG_SRC) << "TextBlock is NULL!";
    }
}

/**
 * Return whether or not the passed string is empty or
 * contains nothing but whitespace.
 * Note: The logic is the other way round. In this way
 *       we do not need all the "!" in if statements.
 */
bool CodeEditor::isNonBlank(const QString &str)
{
    if (str.isEmpty() || str.trimmed().isEmpty()) {
        return false;
    }
    return true;
}

/**
 * Implemented so we may capture certain key presses, namely backspace
 * and 'return' events.
 */
void CodeEditor::keyPressEvent(QKeyEvent * e)
{
    DEBUG(DBG_SRC) << "KEY PRESS EVENT:[" << e->text().toLatin1() << "] ascii CODE:" << e->key();

    if (e->key() == 8) {  // || (e->key() == 127))  // what about delete?
        m_backspacePressed = true;
    }
    // Q: can the MAC or WIN/DOS sequences occur?
    if ((e->key() == 10) || (e->key() == 13) || (e->text() == "\r\n")) {
        m_newLinePressed = true;
    }
    KTextEdit::keyPressEvent(e);
}

/**
 * (Re) Load the parent code document into the editor.
 */
void CodeEditor::loadFromDocument()
{
    // clear the tool
    clearText();

    // set caption on tool
    QString caption = m_parentDoc->getFileName() + m_parentDoc->getFileExtension();
    setWindowTitle( tr2i18n( caption.toLatin1() ) );
    DEBUG(DBG_SRC) << "set window title to: " << caption;

    // header for document
    QString header = m_parentDoc->getHeader()->toString();
    QString componentName = QString("header for file ") + caption;
    if (isNonBlank(header)) {
        DEBUG(DBG_SRC) << "header for document: " << header;
        insertText(header, m_parentDoc->getHeader(), false, state().fontColor,
               state().nonEditBlockColor, 0, componentName);
    }

    // now all the text blocks in the document
    TextBlockList * items = m_parentDoc->getTextBlockList();
    DEBUG(DBG_SRC) << "TextBlockList: " << items->count();
    appendText(items);

    textCursor().setPosition(0);
}

/**
 * Main insert routine. Will append if startLine is not supplied or -1.
 * @param text          the text which has to be inserted
 * @param parent        the parent @ref TextBlock
 * @param editable      flag if editable
 * @param fgcolor       foreground color
 * @param bgcolor       background color
 * @param umlobj        the UML object
 * @param displayName   the name which can be displayed
 * @param startLine     the starting line
 */
void CodeEditor::insertText(const QString & text, TextBlock * parent,
    bool editable, const QColor & fgcolor, const QColor & bgcolor,
    UMLObject * umlobj, const QString & displayName, int startLine)
{
    // set some params
    bool isInsert = false;
    setTextColor(fgcolor);

    // it is an append op if startLine is -1, otherwise it is
    // an actual insert, which is more complicated
    if (startLine == -1) {
        startLine = 0;
        if (!m_textBlockList.isEmpty()) {
            TextBlock* lastTb = m_textBlockList.last();
            if (m_tbInfoMap.contains(lastTb)) {
                TextBlockInfo *tbi = m_tbInfoMap[lastTb];
                if (tbi && !tbi->m_paraList.isEmpty()) {
                    ParaInfo* pi = tbi->m_paraList.last();
                    startLine = pi->end + 1;
                }
            }
        }
        KTextEdit::append(text); // put actual text in. Use insert instead of append so history is preserved?
    }
    else {
        isInsert = true;
        textCursor().setPosition(startLine);
        textCursor().insertText(text);
    }

    int endLine = text.count('\n') + startLine;
    // now do 'paragraph' background highlighting
    if (m_isHighlighted) {
        for (int ln = startLine; ln <= endLine; ++ln) {
            setParagraphBackgroundColor(ln, bgcolor);
        }
    }

    // record paragraph information
    // Did we already start recording info for this parent object?
    TextBlockInfo * tbinfo;
    if (m_tbInfoMap.contains(parent)) {
        tbinfo = m_tbInfoMap[parent];
    }
    else {
        tbinfo = new TextBlockInfo();
        tbinfo->setDisplayName(displayName);
        tbinfo->isCodeAccessorMethod = dynamic_cast<CodeAccessorMethod*>(parent) ? true : false;
        m_tbInfoMap.insert(parent, tbinfo);
    }

    // set a parent, if it is not already set
    if (umlobj && !tbinfo->parent()) {
        tbinfo->setDisplayName(displayName);
        tbinfo->setParent(umlobj);
        tbinfo->isClickable = textBlockIsClickable(umlobj);
    }

    // now mark all lines that we just inserted as belonging to the parent
    for (int ln = startLine; ln <= endLine; ++ln) {
        m_textBlockList.insert(ln, parent);
    }

    // create the object that records this particular "paragraph"
    ParaInfo * item = new ParaInfo();
    item->start = startLine;
    item->size = text.size();
    item->end  = endLine;
    item->fgcolor = fgcolor;
    item->bgcolor = bgcolor;
    item->isEditable = editable;
    DEBUG(DBG_SRC) << "startLine: "<< item->start << " / endLine: " << item->end
                   << " / size: " << item->size << " /\ntext: " << text;

    if (isInsert) {
        // now we have to fix the 'start' value for all the para
        // info blocks that correspond to textblocks that we inserted
        // inside of. This means parent tblock paragraph locations
        // that are greater than zero in that type of textblock

        int increase = item->size + 1;
        QMap<TextBlock*,TextBlockInfo*>::Iterator it;
        for ( it = m_tbInfoMap.begin(); it != m_tbInfoMap.end(); ++it ) {
            TextBlock * tblock = it.key();
            TextBlockInfo * thisTbInfo = it.value();
            int firstLoc = m_textBlockList.indexOf(tblock);

            foreach (ParaInfo * pi, thisTbInfo->m_paraList) {
                int minPara = pi->start + firstLoc;

                // only worth doing if in range of the whole representation
                ParaInfo * lastPi = thisTbInfo->m_paraList.last();
                if (!pi->start &&
                    (startLine > (lastPi->start + firstLoc + lastPi->size) || endLine < minPara)) {
                    break;
                }

                // now, only for those paraInfo blocks which
                // have exceeded our last line, we increase them
                if (pi->start && minPara >= endLine) {
                    pi->start += increase;
                }
            }
        }

    }
    if (tbinfo) {
        tbinfo->m_paraList.append(item);
    }
}

/**
 * Appends a @ref TextBlockList to the widget.
 * @param items   list of @ref TextBlock items
 */
void CodeEditor::appendText(TextBlockList * items)
{
    DEBUG(DBG_SRC) << "text block list";
    foreach (TextBlock* tb, *items) {
        // types of things we may cast our text block into
        // This isnt efficient, and is a vote for recording
        // code block types in an enumerated list somewhere,
        // as well as a generic attribute "blockType" we could
        // quickly access, rather than casting. -b.t.
        HierarchicalCodeBlock * hb = 0;
        CodeMethodBlock * mb = 0;
        CodeClassFieldDeclarationBlock * db = 0;
        CodeBlockWithComments * cb = 0;
        // CodeComment * cm = 0;
        if ( (hb = dynamic_cast<HierarchicalCodeBlock *>(tb)) )
            appendText(hb);
        else if ( (mb = dynamic_cast<CodeMethodBlock*>(tb)) )
            appendText(mb);
        else if ( (db = dynamic_cast<CodeClassFieldDeclarationBlock*>(tb)) )
            appendText(db);
        else if ( (cb = dynamic_cast<CodeBlockWithComments*>(tb)) )
            appendText(cb);
        /*
             // No! Shouldn't be any 'naked' comments floating about. Always
             // are assocated with a parent code block.
             else if ( (cm = dynamic_cast<CodeComment*>(tb)) )
                    appendText(cm);
        */
        else
            appendText(tb); // No cast worked. Just do a text block
    }
}

/**
 * Appends a @ref CodeComment to the widget.
 * @param comment         the code comment to add
 * @param parent          the parent text block
 * @param umlObj          the UML object
 * @param componentName   the name of the component
 */
void CodeEditor::appendText(CodeComment * comment, TextBlock * parent, UMLObject * umlObj, const QString & componentName)
{
    DEBUG(DBG_SRC) << "comment";
    if (!comment->getWriteOutText() && !m_showHiddenBlocks)
        return;

    QColor bgcolor = state().nonEditBlockColor;
    if (!comment->getWriteOutText() && m_showHiddenBlocks)
        bgcolor = state().hiddenColor;

    QString indent = comment->getIndentationString();
    QString text = comment->toString(); // use comment formatting, NOT formatMultiLineText(comment->toString(), indent, "\n");
    if (isNonBlank(text))
        insertText(text, parent, true, state().fontColor, bgcolor, umlObj, componentName);
}

/**
 * Appends a @ref CodeBlockWithComments to the widget.
 * @param cb   the code block to add
 */
void CodeEditor::appendText(CodeBlockWithComments * cb)
{
    DEBUG(DBG_SRC) << "code block with comments";
    if (!cb->getWriteOutText() && !m_showHiddenBlocks)
        return;

    QString indent = cb->getIndentationString();
    QString body = cb->formatMultiLineText(cb->getText(), indent, "\n");

    QColor bgcolor = state().editBlockColor;
    QString componentName = QString("CodeBlock");

    appendText(cb->getComment(), cb, 0, componentName);

    if (!cb->getWriteOutText() && m_showHiddenBlocks)
        bgcolor = state().hiddenColor;

    if (isNonBlank(body))
        insertText(body, cb, true, state().fontColor, bgcolor, 0);
}

/**
 * Appends a @ref CodeClassFieldDeclarationBlock to the widget.
 * @param db   the code class field declaration block to add
 */
void CodeEditor::appendText(CodeClassFieldDeclarationBlock * db)
{
    DEBUG(DBG_SRC) << "code class field declaration block";
    if (!db->getWriteOutText() && !m_showHiddenBlocks)
        return;

    QString indent = db->getIndentationString();
    QString body = db->formatMultiLineText (db->getText(), indent, "\n");

    UMLObject * parentObj = db->getParentClassField()->getParentObject();

    QColor bgcolor = state().editBlockColor;
    QString componentName;
    if (parentObj)
    {
        if (db->getParentClassField()->parentIsAttribute()) {
            componentName = m_parentDocName + "::attribute_field(" + parentObj->name() + ')';
        }
        else {
            UMLRole * role = dynamic_cast<UMLRole*>(parentObj);
            componentName = m_parentDocName + "::association_field(" + role->name() + ')';
        }
        bgcolor = state().umlObjectColor;
    }

    appendText(db->getComment(), db, parentObj,componentName);

    if (!db->getWriteOutText() && m_showHiddenBlocks)
        bgcolor = state().hiddenColor;

    if (isNonBlank(body))
        insertText(body, db, false, state().fontColor, bgcolor, parentObj);
}

/**
 * Appends a @ref CodeMethodBlock to the widget.
 * @param mb   the code method block to add
 */
void CodeEditor::appendText(CodeMethodBlock * mb)
{
    DEBUG(DBG_SRC) << "code  method block";
    // Note: IF CodeAccessors are hidden, we DON'T show
    // it even when requested as the hiddeness of these methods
    // should be controled by the class fields, not the user in the editor.
    if (!mb->getWriteOutText() && (!m_showHiddenBlocks || dynamic_cast<CodeAccessorMethod*>(mb)))
        return;

    QColor bgcolor = state().umlObjectColor;
    QString indent = mb->getIndentationString();
    QString bodyIndent = mb->getIndentationString(mb->getIndentationLevel()+1);

    QString startText = mb->formatMultiLineText ( mb->getStartMethodText(), indent, "\n");
    QString body = mb->formatMultiLineText (mb->getText(), bodyIndent, "\n");
    QString endText = mb->formatMultiLineText( mb->getEndMethodText(), indent, "\n");

    if (body.isEmpty())
        body = " \n";

    if (!mb->getWriteOutText() && m_showHiddenBlocks) {
        // it gets the 'hidden' color
        bgcolor = state().hiddenColor;
    }

    QString componentName = QString("<b>parentless method\?</b>");

    // ugly, but we need to know if there is a parent object here.
    CodeOperation * op = dynamic_cast<CodeOperation*>(mb);
    CodeAccessorMethod * accessor = dynamic_cast<CodeAccessorMethod*>(mb);
    UMLObject * parentObj = 0;
    if (op) {
        parentObj = op->getParentOperation();
        if (((UMLOperation*)parentObj)->isConstructorOperation())
            componentName = m_parentDocName + "::operation("+ parentObj->name()+") constructor method";
        else
            componentName = m_parentDocName + "::operation("+ parentObj->name()+") method";
    }
    if (accessor) {
        parentObj = accessor->getParentObject();
        if (accessor->getParentClassField()->parentIsAttribute()) {
            componentName = m_parentDocName + "::attribute_field(" + parentObj->name() + ") accessor method";
        }
        else {
            UMLRole * role = dynamic_cast<UMLRole*>(parentObj);
            componentName = m_parentDocName + "::association_field(" + role->name() + ") accessor method";
        }
    }

    //appendText(mb->getComment(), mb, parentObj, componentName);
    appendText(mb->getComment(), mb->getComment(), parentObj, componentName);

    if (isNonBlank(startText))
        insertText(startText, mb, false, state().fontColor, bgcolor, parentObj);
    // always insert body for methods. IF we don't, we create a
    // situation where the user cannot edit the body (!)
    insertText(body, mb, true, state().fontColor, bgcolor, parentObj);
    if (isNonBlank(endText))
        insertText(endText, mb, false, state().fontColor, bgcolor, parentObj);
}

/**
 * Appends a @ref TextBlock to the widget.
 * @param tb   the text block to add
 */
void CodeEditor::appendText(TextBlock * tb)
{
    DEBUG(DBG_SRC) << "text block";
    if (!tb->getWriteOutText() && !m_showHiddenBlocks)
        return;

    QColor bgcolor = state().nonEditBlockColor;
    if (!tb->getWriteOutText() && m_showHiddenBlocks)
        bgcolor = state().hiddenColor;

    QString str = tb->toString();
    insertText(str, tb, false, state().fontColor, bgcolor);
}

/**
 * Appends a @ref HierarchicalCodeBlock to the widget.
 * @param hblock   the hierarchical code block to add
 */
void CodeEditor::appendText(HierarchicalCodeBlock * hblock)
{
    DEBUG(DBG_SRC) << "hierarchical code block";
    if (!hblock->getWriteOutText() && !m_showHiddenBlocks)
        return;

    OwnedHierarchicalCodeBlock * test = dynamic_cast<OwnedHierarchicalCodeBlock *>(hblock);
    UMLObject * parentObj = 0;
    QString componentName;
    QColor paperColor = state().nonEditBlockColor;
    if (test) {
        parentObj = test->getParentObject();
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(parentObj);
        if (c) {
            QString typeStr;
            if (c->isInterface())
                typeStr = "Interface";
            else
                typeStr = "Class";
            componentName = m_parentDocName + "::" + typeStr + '(' + parentObj->name() + ')';
        }
        else {
            componentName = m_parentDocName + "::UNKNOWN(" + parentObj->name() + ')';
        }

        paperColor = state().umlObjectColor;
    }

    if (!hblock->getWriteOutText() && m_showHiddenBlocks)
        paperColor = state().hiddenColor;

    TextBlockList * items = hblock->getTextBlockList();
    QString indent = hblock->getIndentationString();
    QString startText = hblock->formatMultiLineText ( hblock->getStartText(), indent, "\n");
    QString endText = hblock->formatMultiLineText( hblock->getEndText(), indent, "\n");

    appendText(hblock->getComment(), hblock, parentObj, componentName);

    if (isNonBlank(startText))
        insertText(startText, hblock, false, state().fontColor, paperColor, parentObj);
    appendText(items);
    if (isNonBlank(endText))
        insertText(endText, hblock, false, state().fontColor, paperColor);
}

/**
 * Insert a paragraph at a given position.
 * @param text   the paragraph text
 * @param para   the position where to add the text
 */
void CodeEditor::insertParagraph(const QString & text, int para)
{
    textCursor().setPosition(para);
    textCursor().insertText(text);
}

/**
 * Remove a paragraph from a given position.
 * @param para   the position from where to remove the text
 */
void CodeEditor::removeParagraph(int para)
{
    textCursor().setPosition(para);
    textCursor().select(QTextCursor::BlockUnderCursor);
    textCursor().removeSelectedText();
}

/**
 * All umlobjects which may have pop-up boxes should return true here.
 * Yes, a CRAPPY way of doing this. Im not proud. =b.t.
 */
bool CodeEditor::textBlockIsClickable(UMLObject * obj)
{
    if ( dynamic_cast<UMLAttribute*>(obj) )
        return true;
    else if ( dynamic_cast<UMLClassifier*>(obj) )
        return true;
    else if ( dynamic_cast<UMLRole*>(obj) )
        return true;
    else if ( dynamic_cast<UMLOperation*>(obj) )
        return true;

    return false;
}

/**
 * Slot to change the view of the selected block.
 * This is called from a popup menu item.
 */
void CodeEditor::slotChangeSelectedBlockView()
{
    TextBlock * tb = m_selectedTextBlock;
    if (tb) {
        tb->setWriteOutText(tb->getWriteOutText() ? false : true );
        rebuildView(m_lastPara);
    }
}

/**
 * Change the status of the comment writeOutText value to
 * opposite of current value.
 */
void CodeEditor::slotChangeSelectedBlockCommentView()
{
    TextBlock * tb = m_selectedTextBlock;
    CodeBlockWithComments * cb = NULL;
    if (tb && (cb = dynamic_cast<CodeBlockWithComments*>(tb))) {
        CodeComment* codcom = cb->getComment();
        if (codcom) {
            codcom->setWriteOutText(codcom->getWriteOutText() ? false : true );
            rebuildView( m_lastPara );
        }
    }
}

/**
 * Slot to insert a code block before the selection.
 */
void CodeEditor::slotInsertCodeBlockBeforeSelected()
{
    TextBlock * tb = m_selectedTextBlock;
    CodeBlockWithComments * newBlock = m_parentDoc->newCodeBlockWithComments();
    newBlock->setText("<<INSERT>>");
    newBlock->getComment()->setWriteOutText(false);

    m_parentDoc->insertTextBlock(newBlock, tb, false);

    int location = m_textBlockList.indexOf(m_selectedTextBlock); // find first para of selected block

    QString body = newBlock->formatMultiLineText(newBlock->getText(), newBlock->getIndentationString(), "\n");

    insertText(body, newBlock, true, state().fontColor,
           state().editBlockColor, 0, QString("CodeBlock"), location);
}

/**
 * Slot to insert a code block after the selection.
 */
void CodeEditor::slotInsertCodeBlockAfterSelected()
{
    TextBlock * tb = m_selectedTextBlock;
    CodeBlockWithComments * newBlock = m_parentDoc->newCodeBlockWithComments();
    newBlock->setText("<<INSERT>>");
    newBlock->getComment()->setWriteOutText(false);

    m_parentDoc->insertTextBlock(newBlock, tb, true);

    // find last para of selected block
    TextBlockInfo *tbinfo = m_tbInfoMap[m_selectedTextBlock];
    ParaInfo * lastpi = tbinfo->m_paraList.last();
    int location = m_textBlockList.indexOf(m_selectedTextBlock) + lastpi->start + lastpi->size + 1;

    QString body = newBlock->formatMultiLineText(newBlock->getText(), newBlock->getIndentationString(), "\n");

    insertText(body, newBlock, true, state().fontColor,
           state().editBlockColor, 0, QString("CodeBlock"), location);
}

/**
 * Shows the context menu.
 * Reimplemented from QWidget::contextMenuEvent().
 */
void CodeEditor::contextMenuEvent(QContextMenuEvent * event)
{
    KMenu* menu = createPopup();
    menu->exec(event->globalPos());
    delete menu;
}

/**
 * Create the popup menu.
 * @return   the popup menu
 */
KMenu * CodeEditor::createPopup()
{
    DEBUG(DBG_SRC) << "called...";

    KMenu * menu = new KMenu(this);

    TextBlock * tb = m_selectedTextBlock;
    if (tb) {
        if (tb->getWriteOutText()) {
            KAction* hideAct = new KAction(i18n("Hide"), this);
            hideAct->setShortcut(Qt::Key_H);
            connect(hideAct, SIGNAL(triggered()), this, SLOT(slotChangeSelectedBlockView()));
            menu->addAction(hideAct);
        }
        else {
            KAction* showAct = new KAction(i18n("Show"), this);
            showAct->setShortcut(Qt::Key_S);
            connect(showAct, SIGNAL(triggered()), this, SLOT(slotChangeSelectedBlockView()));
            menu->addAction(showAct);
        }

        CodeBlockWithComments * cb = dynamic_cast<CodeBlockWithComments*>(tb);
        if (cb) {
            if (cb->getComment()->getWriteOutText()) {
                KAction* hideCommAct = new KAction(i18n("Hide Comment"), this);
                hideCommAct->setShortcut(Qt::CTRL + Qt::Key_H);
                connect(hideCommAct, SIGNAL(triggered()), this, SLOT(slotChangeSelectedBlockCommentView()));
                menu->addAction(hideCommAct);
            }
            else {
                KAction* showCommAct = new KAction(i18n("Show Comment"), this);
                showCommAct->setShortcut(Qt::CTRL + Qt::Key_S);
                connect(showCommAct, SIGNAL(triggered()), this, SLOT(slotChangeSelectedBlockCommentView()));
                menu->addAction(showCommAct);
            }
        }
        menu->addSeparator();

        KAction* insCodeBeforeAct = new KAction(i18n("Insert Code Block Before"), this);
        insCodeBeforeAct->setShortcut(Qt::CTRL + Qt::Key_B);
        connect(insCodeBeforeAct, SIGNAL(triggered()), this, SLOT(slotInsertCodeBlockBeforeSelected()));
        menu->addAction(insCodeBeforeAct);

        KAction* insCodeAfterAct = new KAction(i18n("Insert Code Block After"), this);
        insCodeAfterAct->setShortcut(Qt::CTRL + Qt::Key_A);
        connect(insCodeAfterAct, SIGNAL(triggered()), this, SLOT(slotInsertCodeBlockAfterSelected()));
        menu->addAction(insCodeAfterAct);

        menu->addSeparator();

        KAction* copyAct = new KAction(i18n("Copy"), this);
        copyAct->setShortcut(Qt::CTRL + Qt::Key_C);
        connect(copyAct, SIGNAL(triggered()), this, SLOT(slotCopyTextBlock()));
        menu->addAction(copyAct);

        KAction* pasteAct = new KAction(i18n("Paste"), this);
        pasteAct->setShortcut(Qt::CTRL + Qt::Key_V);
        connect(pasteAct, SIGNAL(triggered()), this, SLOT(slotPasteTextBlock()));
        menu->addAction(pasteAct);

        KAction* cutAct = new KAction(i18n("Cut"), this);
        cutAct->setShortcut(Qt::CTRL + Qt::Key_X);
        connect(cutAct, SIGNAL(triggered()), this, SLOT(slotCutTextBlock()));
        menu->addAction(cutAct);

        // enable/disable based on conditions
        if (m_selectedTextBlock == m_parentDoc->getHeader())
            insCodeBeforeAct->setEnabled(false);

        if (!m_textBlockToPaste)
            pasteAct->setEnabled(false);

        if (!tb->canDelete())
            cutAct->setEnabled(false);

        // manythings cant be copied. Right now, lets just limit ourselves to
        // owned things and hierarchicalcodeblocks
        if (dynamic_cast<OwnedCodeBlock*>(m_selectedTextBlock) ||
                dynamic_cast<HierarchicalCodeBlock*>(m_selectedTextBlock))
            copyAct->setEnabled(false);

        // TBD
        // m_selectedTextBlock->insertCodeEditMenuItems(menu, this);
    }

    return menu;
}

/**
 * Slot to copy a text block.
 */
void CodeEditor::slotCopyTextBlock()
{
    // make a copy
    if (dynamic_cast<HierarchicalCodeBlock*>(m_selectedTextBlock))
        m_textBlockToPaste = m_parentDoc->newHierarchicalCodeBlock();
    else if (dynamic_cast<CodeBlockWithComments*>(m_selectedTextBlock))
        m_textBlockToPaste = m_parentDoc->newCodeBlockWithComments();
    else if (dynamic_cast<CodeBlock*>(m_selectedTextBlock))
        m_textBlockToPaste = m_parentDoc->newCodeBlock();
    else if (dynamic_cast<CodeComment*>(m_selectedTextBlock))
        m_textBlockToPaste = CodeGenFactory::newCodeComment(m_parentDoc);
    else {
        uError() << " ERROR: CodeEditor can not copy selected block:" << m_selectedTextBlock << " of unknown type";
        m_textBlockToPaste = 0;
        return; // error!
    }
    m_textBlockToPaste->setAttributesFromObject(m_selectedTextBlock);
}

/**
 * Slot to cut a text block.
 */
void CodeEditor::slotCutTextBlock()
{
    // make a copy first
    slotCopyTextBlock();

    // This could cause problems, but we are OK as
    // long as we only try to delete 'canDelete' textblocks
    if (m_selectedTextBlock->canDelete()) {
        // just in case there are pending edits
        // we don't want to lose them
        if (m_lastTextBlockToBeEdited && m_lastTextBlockToBeEdited == (CodeBlock*) m_selectedTextBlock) {
            updateTextBlockFromText (m_lastTextBlockToBeEdited);
            m_lastTextBlockToBeEdited = 0;
        }

        m_parentDoc->removeTextBlock(m_selectedTextBlock);
        rebuildView(m_lastPara);
        // removeTextBlock(m_selectedTextBlock);
        m_selectedTextBlock = 0;
    }
}

/**
 * Slot to paste a text block.
 */
void CodeEditor::slotPasteTextBlock()
{
    if (m_textBlockToPaste) {
        m_parentDoc->insertTextBlock(m_textBlockToPaste, m_selectedTextBlock);
        m_textBlockToPaste = 0;
        rebuildView(m_lastPara);
    }
}

/**
 * Slot to redraw the text.
 */
void CodeEditor::slotRedrawText()
{
    rebuildView(m_lastPara);
}

/**
 * Initialisation routine which is used in the construtors.
 * @param parentDlg   the parent @ref CodeViewerDialog
 * @param parentDoc   the parent @ref CodeDocument
 */
void CodeEditor::init(CodeViewerDialog * parentDlg, CodeDocument * parentDoc)
{
    DEBUG_REGISTER(DBG_SRC);

    // safety to insure that we are up to date
    parentDoc->synchronize();

    setObjectName("CodeEditor");

    m_parentDlg = parentDlg;
    m_parentDoc = parentDoc;

    setUndoRedoEnabled( false );
//    setCursor( QCursor( 0 ) );  // this line crashes the whole application
    setMouseTracking( true );
    setReadOnly (true);
    m_isHighlighted = state().blocksAreHighlighted;
    m_showHiddenBlocks = state().showHiddenBlocks;

    m_newLinePressed = false;
    m_backspacePressed = false;
    m_textBlockToPaste = 0;
    m_selectedTextBlock = 0;
    m_lastTextBlockToBeEdited = 0;

    setFont( state().font );

    // set name of parent doc
    ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument*>(m_parentDoc);
    if (cdoc)
        m_parentDocName = cdoc->getParentClassifier()->name();
    else
        m_parentDocName = "";

    // set some viewability parameters
    //int margin = fontMetrics().height();

    setTextBackgroundColor(state().paperColor);

    // setMargin(margin);

    // connect(this, SIGNAL(newLinePressed()), this, SLOT(newLinePressed()));
    // connect(this, SIGNAL(backspacePressed()), this, SLOT(backspacePressed()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(slotCursorPositionChanged()));

    // do this last
    loadFromDocument();
}

/**
 * Read the text under the cursor and add it to the text block.
 * @param block   the text block to which the text is added
 */
void CodeEditor::updateTextBlockFromText(TextBlock * block)
{
    if (block) {
        CodeMethodBlock * cmb = dynamic_cast<CodeMethodBlock*>(block);
        //QString baseIndent = block->getNewEditorLine(block->getIndentationLevel() + (cmb ? 1 : 0));
        QString baseIndent = block->getIndentationString(block->getIndentationLevel() + (cmb ? 1 : 0));

        TextBlockInfo *info = m_tbInfoMap[block];
        int pstart = m_textBlockList.indexOf(block);
        QString content;

        // Assemble content from editiable paras
        if (info) {
            QList<ParaInfo*> list = info->m_paraList;
            foreach (ParaInfo * item, list) {
                if (item->isEditable) {
                    int lastpara = item->start+pstart+item->size;
                    int endEdit = block->lastEditableLine();
                    int lastLineToAddNewLine = lastpara + endEdit;
                    for (int para=(item->start+pstart); para<=lastpara; ++para) {
                        textCursor().setPosition(para);
                        QString paraTxt = textCursor().block().text();
                        QString line = block->unformatText(paraTxt, baseIndent);
                        content += line;
                        // \n are implicit in the editor (!) so we should put them
                        // back in, if there is any content from the line
                        if (!line.isEmpty() && para != lastLineToAddNewLine)
                            content += '\n';
                    }
                }
            }
        }

        if (content.isEmpty()) {
            uDebug() << "nothing to add!";
        }
        else {
            uDebug() << "UPDATE GOT CONTENT:[" << content.toLatin1() << "] to block: " << block;
            block->setText(content);

            // if a parent for the block, try to set its documentation
            // as long as it is NOT an accessor codeblock.
            if (info) {
                UMLObject * parentObj = info->parent();
                if (parentObj && !info->isCodeAccessorMethod) {
                    parentObj->setDoc(content);
                }
            }

            // make note that it is now user generated
            if (cmb) {
                cmb->setContentType(CodeBlock::UserGenerated);
            }
        }
    }
}

/**
 * Slot for cursor position changed signal.
 */
void CodeEditor::slotCursorPositionChanged()
{
    QTextCursor tc = textCursor();
    int para = tc.position();
    int pos  = tc.blockNumber();
    clicked(para, pos);

    // safety.. this is endemic of a 'bad' pointer event and can crash us otherwise
    if (pos < 0)
        return;

    //      bool lastParaIsEditable = isReadOnly() ? false : true;
    bool lastParaIsEditable = isParaEditable(m_lastPara);

    // IF last para where cursor is coming from was editable
    // we have a variety of things to look out for.
    if (lastParaIsEditable) {
        // If we got here as the result of a newline, then expansion
        // of a para editablity occurs.
        if ((para-1) == m_lastPara && m_newLinePressed )
            expandSelectedParagraph ( m_lastPara );

        // conversely, we contract the zone of editablity IF we
        // got to current position as result of backspace
        if ((para+1) == m_lastPara && m_backspacePressed)
            contractSelectedParagraph( para );

    }

    // now check if the current paragraph is really editiable, and if so,
    // do some things
    bool editPara = isParaEditable(para);
    if (editPara) {
        TextBlock * tBlock = m_textBlockList.at(para);
        if (!tBlock) {
            uWarning() << "no text block found in list at position " << para;
            return;
        }
        DEBUG(DBG_SRC) << tBlock;

        CodeMethodBlock * cmb = dynamic_cast<CodeMethodBlock*>(tBlock);
        if (!cmb) {
            uWarning() << "cast to CodeMethodBlock failed";
            return;
        }

        // auto-indent new lines
        textCursor().setPosition(para);
        QString currentParaText = textCursor().block().text();
        QString baseIndent = tBlock->getNewEditorLine(tBlock->getIndentationLevel()+(cmb ? 1 : 0));
        // cerr<<"AUTO INDENT:["<<baseIndent.latin1()<<"] isMethod?"<<(cmb?"true":"false")<<endl;
        int minPos = baseIndent.length();

        // add indent chars to the current line, if missing
        if (!m_backspacePressed && !currentParaText.contains(QRegExp('^'+baseIndent))) {
            textCursor().setPosition(para);
            textCursor().insertText(baseIndent);
//:TODO:            setCursorPosition(para, pos+minPos);  // crashes the application !?
            return;
        }

        if (pos < minPos) {
            bool priorParaIsEditable = isParaEditable(para-1);
            if (m_backspacePressed && para && priorParaIsEditable) {
                textCursor().setPosition(para-1);
//:TODO:unused                int endOfPriorLine = textCursor().block().length();
                // In this case, we remove old (para) line, and tack its
                // contents on the line we are going to.
                QString contents = textCursor().block().text();
                contents = contents.right(contents.length()-m_lastPos+1);

                // this next thing happens when we arent deleting last line
                // of editable text, so we want to append whats left of this line
                // onto the one we are backspacing into
                if (paraIsNotSingleLine(para)) {
                    removeParagraph(para);
                    //:TODO: insertAt(contents, (para-1), endOfPriorLine);
                    textCursor().setPosition(para-1);
                    textCursor().insertText(contents);
//:TODO:                    setCursorPosition((para-1), endOfPriorLine);  // crashes the application !?
                }

            }
            else {
                // well, if the following is true, then they
                // are trying to hack away at the last line, which
                // we cant allow to entirely disappear. Lets preserve
                // the indentation
                if (m_backspacePressed && !priorParaIsEditable) {
                    textCursor().setPosition(para);
                    QString contents = textCursor().block().text();
                    contents = contents.right(contents.length()-m_lastPos+1);
                    contents = baseIndent + contents.left(contents.length()-1); // left is to remove trailing space
                    insertParagraph(contents,para+1);
                    removeParagraph(para);

                    // furthermore, If it is nothing but indentation + whitespace
                    // we switch this back to Auto-Generated.
                    if (cmb && contents.contains(QRegExp('^'+baseIndent+"\\s$"))) {
                        cmb->setContentType(CodeBlock::AutoGenerated);
                        cmb->syncToParent();
                    }
                }
                // send them to the first spot in the line which is editable
//:TODO:                setCursorPosition(para, minPos);  // crashes the application !?
            }
            return;
        }
    }

    // look for changes in editability, if they occur, we need to record
    // the edits which have been made
    if ((editPara && !m_lastTextBlockToBeEdited) || (!editPara && m_lastTextBlockToBeEdited))
    {
        setReadOnly(editPara ? false : true);

        // IF this is a different text block, update the body of the method
        // it belongs to
        if (m_lastTextBlockToBeEdited && (m_lastTextBlockToBeEdited != m_textBlockList.at(para) || !editPara))
        {
            updateTextBlockFromText (m_lastTextBlockToBeEdited);
            m_lastTextBlockToBeEdited = 0;
        }

        if (editPara)
            m_lastTextBlockToBeEdited = m_textBlockList.at(para);
        else
            m_lastTextBlockToBeEdited = 0;
    }

    m_lastPara = para;
    m_lastPos = pos;
    m_newLinePressed = false;
    m_backspacePressed = false;
}

/**
 * Check whether a block at a given position is on a single line.
 * @param para   the index of the block in the list
 * @return   flag whether block is on a single line
 */
bool CodeEditor::paraIsNotSingleLine(int para)
{
    TextBlock * tBlock = m_textBlockList.at(para);
    if (tBlock) {
        int pstart = m_textBlockList.indexOf(tBlock);
        TextBlockInfo *info = m_tbInfoMap[tBlock];
        QList<ParaInfo*> list = info->m_paraList;
        foreach (ParaInfo * item, list) {
            if ((pstart+item->start) <= para && (item->start+pstart+item->size) >= para )
                if (item->size > 0)
                    return true;
        }
    }
    return false;
}

/**
 * Find the text block in which the character position is located.
 * @param characterPos   the given character position
 * @return   the text block in which the character position is found
 */
TextBlock* CodeEditor::findTextBlockAt(int characterPos)
{
    int charCount = 0;
    for (int tbIdx = 0; tbIdx < m_textBlockList.count(); ++tbIdx) {
        TextBlock* tb = m_textBlockList.at(tbIdx);
        if (m_tbInfoMap.contains(tb)) {
            TextBlockInfo *tbi = m_tbInfoMap[tb];
            if (tbi && !tbi->m_paraList.isEmpty()) {
                for (int idx = 0; idx < tbi->m_paraList.count(); ++idx) {
                    ParaInfo* pi = tbi->m_paraList.at(idx);
                    charCount += pi->size;
                    if (characterPos < charCount) {
                        return tb;
                    }
                }
            }
        }
    }
    return 0;
}

/**
 * Checks if paragraph is editable or not.
 * Method is used only in @ref slotCursorPositionChanged.
 * @param para   the desired paragraph (character number)
 * @return   flag if paragraph is editable or not
 */
bool CodeEditor::isParaEditable(int para)
{
    int endLine = 0;
    if (!m_textBlockList.isEmpty()) {
        TextBlock* lastTb = m_textBlockList.last();
        if (m_tbInfoMap.contains(lastTb)) {
            TextBlockInfo *tbi = m_tbInfoMap[lastTb];
            if (tbi && !tbi->m_paraList.isEmpty()) {
                ParaInfo* pi = tbi->m_paraList.last();
                endLine = pi->end + 1;
            }
        }
    }
    if ((para < 0) || (para >= document()->characterCount())) {
        DEBUG(DBG_SRC) << "para:" << para << " not in range 0.." << document()->characterCount();
        return false;
    }

    TextBlock * tBlock = findTextBlockAt(para);
    if (tBlock) {
        int editStart = tBlock->firstEditableLine();
        int editEnd = tBlock->lastEditableLine();
        bool hasEditableRange = (editStart > 0 || editEnd < 0) ? true : false;
        TextBlockInfo *info = m_tbInfoMap[tBlock];
        if (info) {
            int pstart = m_textBlockList.indexOf(tBlock);
            int relativeLine = para - pstart;
            QList<ParaInfo*> list = info->m_paraList;
            foreach (ParaInfo * item, list) {
                if (item->start+pstart <= para && item->start+pstart+item->size >= para) {
                    if (item->isEditable && hasEditableRange) {
                        if ( relativeLine >= editStart && relativeLine <= (item->size + editEnd) )
                            return true;
                        else
                            return false;
                    }
                    else
                        return item->isEditable;
                }
            }
        }
        else {
            DEBUG(DBG_SRC) << "TextBlockInfo not found in info map!";
        }
    }
    else {
        DEBUG(DBG_SRC) << "TextBlock not found at position " << para;
    }
    return false;
}

/**
 * :TODO:
 */
void CodeEditor::changeTextBlockHighlighting(TextBlock * tBlock, bool selected)
{
    if (tBlock) {
        TextBlockInfo *info = m_tbInfoMap[tBlock];
        QList<ParaInfo*> list = info->m_paraList;
        int pstart = m_textBlockList.indexOf(tBlock);
        foreach (ParaInfo * item, list) {
            for (int p=(item->start+pstart); p<=(item->start+pstart+item->size); ++p) {
                if (selected) {
                    if (info->isClickable) {
                        setParagraphBackgroundColor(p, state().selectedColor);
                    }
                    else {
                        setParagraphBackgroundColor(p, state().nonEditBlockColor);
                    }
                }
                else if (m_isHighlighted) {
                    setParagraphBackgroundColor(p, item->bgcolor);
                }
                else {
                    setParagraphBackgroundColor(p, state().paperColor);
                }
            }
        }
    }
}

/**
 * Set the background color at the cursor position.
 * @param position   the character position
 * @param color      the desired color
 */
void CodeEditor::setParagraphBackgroundColor(int position, const QColor& color)
{
    textCursor().setPosition(position);
    QTextCharFormat format;
    format.setBackground(color);
    textCursor().setCharFormat(format);
}

/**
 * :TODO:
 */
void CodeEditor::changeShowHidden(int signal)
{
    if (signal)
        m_showHiddenBlocks = true;
    else
        m_showHiddenBlocks = false;

    rebuildView(m_lastPara);
}

/**
 * Colorizes/uncolorizes type for ALL paragraphs.
 */
void CodeEditor::changeHighlighting(int signal)
{
    int total_para = textCursor().blockNumber()-1;
    if (signal) {
        // we want to highlight
        m_isHighlighted = true;
        for (int para = 0; para < total_para; ++para) {
            TextBlock * tblock = m_textBlockList.at(para);
            changeTextBlockHighlighting(tblock,false);
        }

    }
    else {
        // we DON'T want to highlight
        m_isHighlighted = false;
        for (int para = 0; para < total_para; ++para) {
            setParagraphBackgroundColor(para, state().paperColor);
        }
    }

    // now redo the "selected" para, should it exist
    if (m_selectedTextBlock) {
        changeTextBlockHighlighting(m_selectedTextBlock,true);
    }
}

/**
 * :TODO:
 */
void CodeEditor::contractSelectedParagraph(int paraToRemove)
{
    if ((paraToRemove >= 0) && (paraToRemove < m_textBlockList.size())) {
        TextBlock * tBlock = m_textBlockList.at(paraToRemove);
        if (tBlock) {
            int pstart = m_textBlockList.indexOf(tBlock);
            TextBlockInfo *info = m_tbInfoMap[tBlock];
            QList<ParaInfo*> list = info->m_paraList;
            bool lowerStartPosition = false;
            foreach (ParaInfo * item, list) {
                if (lowerStartPosition) {
                    item->start -= 1;
                }
                if ((pstart+item->start) <= paraToRemove && (item->start+pstart+item->size) >= paraToRemove) {
                    item->size -= 1;
                    // a little cheat.. we don't want to remove last line as we need
                    // to leave a place that can be 'edited' by the tool IF the user
                    // changes their mind about method body content
                    if (item->size < 0) {
                        item->size = 0;
                    }
                    lowerStartPosition = true;
                }
            }
            m_textBlockList.removeAt(paraToRemove);
        }
    }
}

/**
 * :TODO:
 */
void CodeEditor::expandSelectedParagraph(int priorPara)
{
    TextBlock * tBlock = m_textBlockList.at(priorPara);
    if (tBlock) {
        // add this tBlock in
        m_textBlockList.insert(priorPara, tBlock);
        TextBlockInfo *info = m_tbInfoMap[tBlock];
        QList<ParaInfo*> list = info->m_paraList;
        int pstart = m_textBlockList.indexOf(tBlock);

        // now update the paragraph information
        bool upStartPosition = false;
        foreach (ParaInfo * item, list) {
            // AFTER we get a match, then following para's need to have start position upped too
            if (upStartPosition)
                item->start += 1;

            if ((pstart+item->start) <= priorPara && (item->start+pstart+item->size) >= priorPara) {
                item->size += 1;
                slotCursorPositionChanged();  //:TODO: m_lastPara, m_lastPos);
                upStartPosition = true;
            }
        }
    }
}

/**
 * Override the QT event so we can do appropriate things.
 */
void CodeEditor::mouseDoubleClickEvent(QMouseEvent * e)
{
    QTextCursor tc = cursorForPosition(e->pos());
    int para = tc.position();
    int pos  = tc.blockNumber();
    clicked(para, pos);

    m_lastPara = para;
    m_lastPos = pos;

    // ugh. more ugliness. We want to be able to call up the
    // correct editing dialog for the given attribute.
    if ((para >= 0) && (para < document()->characterCount())) {
        TextBlock * tBlock = findTextBlockAt(para);
        editTextBlock(tBlock, para);
    }
    else {
        DEBUG(DBG_SRC) << "para:" << para << " not in range 0.." << document()->characterCount();
    }
}

/**
 * Override the QT event so we can do appropriate things.
 */
void CodeEditor::contentsMouseMoveEvent(QMouseEvent * e)
{
    QTextCursor tc = cursorForPosition(e->pos());
    int para = tc.position();
    int pos  = tc.blockNumber();
    clicked(para, pos);

    if (para < 0)
        return; // shouldn't happen..

    TextBlock * tblock = m_textBlockList.at(para);
    if (tblock && m_selectedTextBlock != tblock ) {
        TextBlockInfo * info = m_tbInfoMap[tblock];

        // unhighlight old selected textblock regardless of whether
        // it was selected or not.
        changeTextBlockHighlighting(m_selectedTextBlock,false);

        // highlight new block
        changeTextBlockHighlighting(tblock,true);

        // FIX: update the label that shows what type of component this is
        componentLabel()->setText("<b>"+info->displayName()+"</b>");

        m_selectedTextBlock = tblock;

        if (m_lastTextBlockToBeEdited) {
            updateTextBlockFromText (m_lastTextBlockToBeEdited);
            m_lastTextBlockToBeEdited = 0;
        }
    }

    // record this as the last paragraph
}

/**
 * Rebuild our view of the document. Happens whenever we change
 * some field/aspect of an underlying UML object used to create
 * the view.
 * If connections are right, then the UMLObject will send out the modified()
 * signal which will trigger a call to re-generate the appropriate code within
 * the code document. Our burden is to appropriately prepare the tool: we clear
 * out ALL the textblocks in the KTextEdit widget and then re-show them
 * after the dialog disappears.
 */
void CodeEditor::rebuildView(int startCursorPos)
{
    Q_UNUSED(startCursorPos);  //:TODO:
    loadFromDocument();
    // make a minima attempt to leave the cursor (view of the code) where
    // we started
//:TODO:    int new_nrof_para = paragraphs() -1;
//:TODO:    setCursorPosition((startCursorPos < new_nrof_para ? startCursorPos : 0), 0);  //:TODO: crashes the application
}

#include "codeeditor.moc"
