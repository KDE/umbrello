/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLAPPPRIVATE_H
#define UMLAPPPRIVATE_H

// app includes
#include "finddialog.h"
#include "findresults.h"
#include "uml.h"
#include "diagramswindow.h"
#include "objectswindow.h"
#include "stereotypeswindow.h"

// kde includes
#include <KActionCollection>
#include <KToggleAction>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

// qt includes
#include <QFile>
#include <QFileInfo>
#include <QListWidget>
#include <QObject>

class QWidget;

/**
 * Class UMLAppPrivate holds private class members/methods
 * to reduce the size of the public class and to speed up
 * recompiling.
 * The migration to this class is not complete yet.
 **/
class UMLAppPrivate : public QObject
{
    Q_OBJECT
public:
    UMLApp *parent;
    FindDialog findDialog;
    FindResults findResults;
    QListWidget *logWindow;         ///< Logging window.
    KToggleAction *viewDiagramsWindow;
    KToggleAction *viewObjectsWindow;
    KToggleAction *viewStereotypesWindow;
    DiagramsWindow *diagramsWindow;
    ObjectsWindow *objectsWindow;
    StereotypesWindow *stereotypesWindow;

    KTextEditor::Editor *editor;
    KTextEditor::View *view;
    KTextEditor::Document *document;

    explicit UMLAppPrivate(UMLApp *_parent)
      : parent(_parent),
        findDialog(_parent),
        viewDiagramsWindow(0),
        viewObjectsWindow(0),
        viewStereotypesWindow(0),
        diagramsWindow(0),
        objectsWindow(0),
        stereotypesWindow(0),
        view(0),
        document(0)
    {
        editor = KTextEditor::Editor::instance();
        logWindow = new QListWidget;
        connect(logWindow, &QListWidget::itemDoubleClicked, this, &UMLAppPrivate::slotLogWindowItemDoubleClicked);
    }

    ~UMLAppPrivate()
    {
        delete logWindow;
    }

public slots:
    void slotLogWindowItemDoubleClicked(QListWidgetItem *item)
    {
        QStringList columns = item->text().split(QChar::fromLatin1(':'));

        QFileInfo file(columns[0]);
        if (!file.exists())
            return;

        document = editor->createDocument(0);
        view = document->createView(parent);
        view->document()->openUrl(QUrl(columns[0]));
        view->document()->setReadWrite(false);
        view->setCursorPosition(KTextEditor::Cursor(columns[1].toInt()-1,columns[2].toInt()));
        KTextEditor::ConfigInterface *iface = qobject_cast<KTextEditor::ConfigInterface*>(view);
        if(iface)
            iface->setConfigValue(QString::fromLatin1("line-numbers"), true);

        SinglePageDialogBase *dialog = new SinglePageDialogBase(parent);
        dialog->setMainWidget(view);
        dialog->setMinimumSize(800, 800);
        dialog->exec();
        delete dialog;
        delete document;
    }

    void createDiagramsWindow()
    {
        // create the tree viewer
        diagramsWindow = new DiagramsWindow(i18n("&Diagrams"), parent);
        parent->addDockWidget(Qt::LeftDockWidgetArea, diagramsWindow);

        viewDiagramsWindow = parent->actionCollection()->add<KToggleAction>(QLatin1String("view_diagrams_window"));
        viewDiagramsWindow->setText(i18n("Diagrams"));
        connect(viewDiagramsWindow, &KToggleAction::triggered, diagramsWindow, &DiagramsWindow::setVisible);
    }

    void createObjectsWindow()
    {
        // create the object window
        objectsWindow = new ObjectsWindow(i18n("&UML Objects"), parent);
        parent->addDockWidget(Qt::RightDockWidgetArea, objectsWindow);

        viewObjectsWindow = parent->actionCollection()->add<KToggleAction>(QLatin1String("view_objects_window"));
        connect(viewObjectsWindow, SIGNAL(triggered(bool)), objectsWindow, SLOT(setVisible(bool)));
    }

    void createStereotypesWindow()
    {
        // create the tree viewer
        stereotypesWindow = new StereotypesWindow(i18n("&Stereotypes"), parent);
        parent->addDockWidget(Qt::LeftDockWidgetArea, stereotypesWindow);

        viewStereotypesWindow = parent->actionCollection()->add<KToggleAction>(QLatin1String("view_stereotypes_window"));
        viewStereotypesWindow->setText(i18n("Stereotypes"));
        connect(viewStereotypesWindow, &KToggleAction::triggered, stereotypesWindow, &StereotypesWindow::setVisible);
    }

};

#endif
