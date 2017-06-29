/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2017                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "diagram_utils.h"

// app includes
#include "debug_utils.h"
#include "messagewidget.h"
#include "object_factory.h"
#include "objectwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlscene.h"
#include "widget_factory.h"

//// qt includes
#include <QMap>
#include <QMimeData>
#include <QRegExp>

DEBUG_REGISTER_DISABLED(Diagram_Utils)
#undef DBG_SRC
#define DBG_SRC QLatin1String("Diagram_Utils")

namespace Diagram_Utils {

/**
 * Detect sequence line format
 * @param lines
 * @return
 */
SequenceLineFormat detectSequenceLineFormat(const QStringList &lines)
{
    QStringList l = lines;
    while(l.size() > 0) {
        QStringList cols = l.takeFirst().split(QRegExp(QLatin1String("\\s+")),QString::SkipEmptyParts);
        if (cols.size() < 1)
            continue;

        if (cols[0] == QLatin1String("#")) {
            continue;
        }
        /*
         * #0  0x000000000050d0b0 in Import_Utils::importStackTrace(QString const&, UMLScene*) (fileName=..., scene=scene@entry=0x12bd0f0)
         */
        if (cols.size() > 2 && cols[0].startsWith(QLatin1String("#")))
            return GDB;
        /*
         *  6	Driver::ParseHelper::ParseHelper	driver.cpp	299	0x634c44
         */
        else if (cols[cols.size()-1].startsWith(QLatin1String("0x")))
            return QtCreatorGDB;
        /*
         * FloatingTextWidget::setPreText
         */
        else if (cols[cols.size()-1].contains(QLatin1String("::")))
            return Simple;
        else
            return Invalid;
    }
    return Invalid;
}

/**
 * Helper function to parse sequence line. The supported formats are:
 *
 * @param s string to parse
 * @param sequence return of sequence number
 * @param package return of package
 * @param method return of method
 * @return true line could be parsed and return variable has been filled
 * @return false line could not be parsed, no return variable has been filled
 */
bool parseSequenceLine(const QString &s, QString &sequence, QString &package, QString &method)
{
    QString identifier;
    QString module;
    QStringList cols = s.split(QRegExp(QLatin1String("\\s+")),QString::SkipEmptyParts);
    if (cols.size() < 1) {
        DEBUG(DBG_SRC) << "could not parse" << s;
        return false;
    }
    // skip comments
    if (cols[0] == QLatin1String("#")) {
        return false;
    }

    /*  gdb
     * #0  0x000000000050d0b0 in Import_Utils::importStackTrace(QString const&, UMLScene*) (fileName=..., scene=scene@entry=0x12bd0f0)
     * #25 0x00007fffefe1670c in g_main_context_iteration () from /usr/lib64/libglib-2.0.so.0
     * #0  Import_Utils::importStackTrace (fileName=..., scene=scene@entry=0x137c000) at /home/ralf.habacker/src/umbrello-3/umbrello/codeimport/import_utils.cpp:715
     */
    if (cols.size() > 2 && cols[0].startsWith(QLatin1String("#"))) {
        QString file;
        sequence = cols.takeFirst();
        if (cols[cols.size()-2] == QLatin1String("at")) {
            file = cols.takeLast();
            cols.takeLast();
        }
        else if (cols[cols.size()-2] == QLatin1String("from")) {
            module = cols.takeLast();
            cols.takeLast();
        }

        if (cols[1] == QLatin1String("in")) {
            cols.takeFirst(); // remove address
            cols.takeFirst(); // in
        }

        identifier = cols.join(QLatin1String(" "));

        if (identifier.contains(QLatin1String("::"))) {
            QStringList b = identifier.split( QLatin1String("::"));
            // TODO handle multiple '::'
            package = b.takeFirst();
            method = b.join(QLatin1String("::"));
        }
        else {
            method = identifier;
            package = module;
        }
        return true;
    }

    /**
     * Qtcreator/gdb
     *  6	Driver::ParseHelper::ParseHelper	driver.cpp	299	0x634c44
     * 31   g_main_context_dispatch /usr/lib64/libglib-2.0.so.0     0x7fffefe16316
     * ignoring
     * ... <more>                                                   0x7ffff41152d9
     * 13  ??                                                       0x7ffff41152d9
     */
    else if (cols[cols.size()-1].startsWith(QLatin1String("0x"))) {
        if (cols[0] == QLatin1String("...") || cols[1] == QLatin1String("??"))
            return false;

        sequence = cols.takeFirst();
        cols.takeLast();  // remove address

        QString line, file;
        if (cols.size() == 2) {
            module = cols.takeLast();
            identifier = cols.join(QLatin1String(" "));
        } else if (cols.size() > 2) {
            line = cols.takeLast();
            file = cols.takeLast();
            identifier = cols.join(QLatin1String(" "));
        }

        if (identifier.contains(QLatin1String("::"))) {
            QStringList b = identifier.split( QLatin1String("::"));
            method = b.takeLast();
            package = b.join(QLatin1String("::"));
        }
        else {
            method = identifier;
            package = module;
        }

        if (package.isEmpty() && !file.isEmpty())
            package = file;

        if (!method.endsWith(QLatin1String(")")))
            method.append(QLatin1String("()"));

        return true;
    } else if (cols[cols.size()-1].contains(QLatin1String("::"))) {
       QStringList b = s.split( QLatin1String("::"));
       method = b.takeLast();
       package = b.join(QLatin1String("::"));
       return true;
    }
    return false;
}

/**
 * Import sequence diagram entries from a string list.
 *
 * @param lines String list with sequences
 * @param scene The diagram to import the sequences into.
 * @return true Import was successful.
 * @return false Import failed.
 */
bool importSequences(const QStringList &lines, UMLScene *scene)
{
    // object widget cache map
    QMap<QString, ObjectWidget*> objectsMap;

    // create "client widget"
    UMLDoc *umldoc = UMLApp::app()->document();
    QString name(QLatin1String("client"));
    UMLObject *left = umldoc->findUMLObject(name, UMLObject::ot_Class);
    if (!left ) {
        left = new UMLObject(0, name);
        left->setBaseType(UMLObject::ot_Class);
    }

    ObjectWidget *leftWidget = (ObjectWidget *)Widget_Factory::createWidget(scene, left);
    leftWidget->activate();
    // required to be savable
    scene->addWidget(leftWidget);
    objectsMap[name] = leftWidget;

    ObjectWidget *rightWidget = 0;
    ObjectWidget *mostRightWidget = leftWidget;
    MessageWidget *messageWidget = 0;
    // for further processing
    MessageWidgetList messages;

    QStringList l;
    SequenceLineFormat format = detectSequenceLineFormat(lines);
    if (format == GDB || format == QtCreatorGDB)
        foreach(const QString &s, lines)
            l.push_front(s);
    else
        l = lines;

    // for each line
    int index = 1;
    foreach(const QString &line, l) {
        QString stackframe, package, method;

        if (!parseSequenceLine(line, stackframe, package, method))
            continue;

        bool createObject = false;
        if (package.contains(method))
            createObject = true;

        if (package.isEmpty())
            package = QLatin1String("unknown");

        // get or create right object widget
        if (objectsMap.contains(package)) {
            rightWidget = objectsMap[package];
        } else {
            UMLObject *right = umldoc->findUMLObject(package, UMLObject::ot_Class);
            if (!right)
                right = umldoc->findUMLObject(package, UMLObject::ot_Package);
            if (!right) {
                right = Object_Factory::createUMLObject(UMLObject::ot_Class, package);
            }

            rightWidget = (ObjectWidget *)Widget_Factory::createWidget(scene, right);
            rightWidget->setX(mostRightWidget->x() + mostRightWidget->width() + 10);
            rightWidget->activate();
            objectsMap[package] = rightWidget;
            scene->addWidget(rightWidget);
            mostRightWidget = rightWidget;
        }

        // create message
        int y = 10 + (messageWidget ? messageWidget->y() + messageWidget->height() : 20);
        messageWidget = new MessageWidget(scene, leftWidget, rightWidget, y,
                                          createObject ? Uml::SequenceMessage::Creation : Uml::SequenceMessage::Synchronous);
        messageWidget->setCustomOpText(method);
        messageWidget->setSequenceNumber(QString::number(index++));
        messageWidget->calculateWidget();
        messageWidget->activate();
        messageWidget->setY(y);
        // to make it savable
        scene->messageList().append(messageWidget);
        messages.insert(0, messageWidget);

        leftWidget = rightWidget;
    }

    if (messages.isEmpty())
        return false;

    // adjust vertical position
    foreach(MessageWidget *w, messages) {
        w->setY(w->y() + 20);
    }

    // adjust heights starting from the last message
    MessageWidget *previous = messages.takeFirst();
    foreach(MessageWidget *w, messages) {
        w->setSize(w->width(), previous->y() - w->y() + previous->height() + 5);
        // adjust vertical line length of object widgets
        w->objectWidget(Uml::RoleType::A)->slotMessageMoved();
        w->objectWidget(Uml::RoleType::B)->slotMessageMoved();
        previous = w;
    }
    return true;
}

/**
 * Import sequence diagram entries from clipboard
 *
 * @param mimeData instance of mime data to import from
 * @param scene The diagram to import the sequences into.
 * @return true Import successful.
 * @return false Import failed.
 */
bool importSequences(const QMimeData* mimeData, UMLScene *scene)
{
    QString requestedFormat = QLatin1String("text/plain");
    if (!mimeData->hasFormat(requestedFormat))
        return false;

    QByteArray payload = mimeData->data(requestedFormat);
    if (!payload.size()) {
        return false;
    }
    QString data = QString::fromUtf8(payload);
    QStringList lines = data.split(QLatin1String("\n"));

    UMLDoc *doc = UMLApp::app()->document();
    doc->beginPaste();
    bool result = importSequences(lines, scene);
    doc->endPaste();
    return result;
}

/**
 * Import sequence diagram entries from file
 *
 * @param fileName filename to import the sequences from.
 * @param scene The diagram to import the sequences into.
 * @return true Import successful.
 * @return false Import failed.
 */
bool importSequences(const QString &fileName, UMLScene *scene)
{
    QFile file(fileName);

    if(!file.open(QIODevice::ReadOnly))
        return false;

    QStringList lines;
    QTextStream in(&file);
    in.setCodec("UTF-8");
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    return importSequences(lines, scene);
}

}  // end namespace Diagram_Utils
