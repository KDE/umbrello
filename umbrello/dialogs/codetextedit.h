/*
    CodeTextEdit: Text edit widget with line numbers and highlighted current line.
    Copyright 2010  Nokia Corporation and/or its subsidiary(-ies) <qt-info@nokia.com>
                    Code based on examples of the Qt Toolkit under BSD license,
                    <http://doc.qt.nokia.com/4.6/widgets-codeeditor.html>.
    Copyright 2010  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CODETEXTEDIT_H
#define CODETEXTEDIT_H

#include <QtGui/QPlainTextEdit>
#include <QtCore/QObject>

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
    CodeTextEdit(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *m_lineNumberArea;
    CodeTextHighlighter *m_highlighter;
};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeTextEdit *editor) : QWidget(editor) {
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
