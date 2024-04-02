/*
    CodeTextEdit: Text edit widget with line numbers and highlighted current line.
    SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>
    Code based on examples of the Qt Toolkit under BSD license,
    <http://doc.qt.nokia.com/4.6/widgets-codeeditor.html>.
    SPDX-FileCopyrightText: 2010 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CODETEXTEDIT_H
#define CODETEXTEDIT_H

#include <QObject>
#include <QPlainTextEdit>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class CodeTextHighlighter;
class LineNumberArea;

class CodeTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeTextEdit(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Q_SLOT void updateLineNumberAreaWidth(int newBlockCount);
    Q_SLOT void highlightCurrentLine();
    Q_SLOT void updateLineNumberArea(const QRect &, int);

private:
    QWidget *m_lineNumberArea;
    CodeTextHighlighter *m_highlighter;
};


class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeTextEdit *editor) : QWidget(editor) {
        m_codeEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeTextEdit *m_codeEditor;
};

#endif // CODETEXTBOX_H
