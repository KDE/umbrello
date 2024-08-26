/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2017-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "diagram_utils.h"

// app includes
#include "associationwidget.h"
#include "association.h"
#define DBG_SRC QStringLiteral("Diagram_Utils")
#include "debug_utils.h"
#include "import_utils.h"
#include "messagewidget.h"
#include "object_factory.h"
#include "objectwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "umlscene.h"
#include "widget_factory.h"

//// qt includes
#include <QListWidget>
#include <QMap>
#include <QMimeData>
#include <QRegularExpression>

// Currently this file is not using debug statements. Activate this line when inserting them:
DEBUG_REGISTER_DISABLED(Diagram_Utils)

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
        QStringList cols = l.takeFirst().split(QRegularExpression(QStringLiteral("\\s+")),Qt::SkipEmptyParts);
        if (cols.size() < 1)
            continue;

        if (cols[0] == QStringLiteral("#")) {
            continue;
        }
        /*
         * #0  0x000000000050d0b0 in Import_Utils::importStackTrace(QString const&, UMLScene*) (fileName=..., scene=scene@entry=0x12bd0f0)
         */
        if (cols.size() > 2 && cols[0].startsWith(QStringLiteral("#")))
            return GDB;
        /*
         *  6	Driver::ParseHelper::ParseHelper	driver.cpp	299	0x634c44
         */
        else if (cols[cols.size()-1].startsWith(QStringLiteral("0x")))
            return QtCreatorGDB;
        /*
         * FloatingTextWidget::setPreText
         */
        else if (cols[cols.size()-1].contains(QStringLiteral("::")))
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
 * @param error  return of error string if error happened
 * @return true line could be parsed and return variable has been filled
 * @return false line could not be parsed, no return variable has been filled
 */
bool parseSequenceLine(const QString &s, QString &sequence, QString &package, QString &method, QString &error)
{
    QString identifier;
    QString module;
    QStringList cols = s.split(QRegularExpression(QStringLiteral("\\s+")),Qt::SkipEmptyParts);
    if (cols.size() < 1) {
        error = QStringLiteral("could not parse");
        return false;
    }
    // skip comments
    if (cols[0] == QStringLiteral("#")) {
        return false;
    }

    /*  gdb
     * #0  0x000000000050d0b0 in Import_Utils::importStackTrace(QString const&, UMLScene*) (fileName=..., scene=scene@entry=0x12bd0f0)
     * #25 0x00007fffefe1670c in g_main_context_iteration () from /usr/lib64/libglib-2.0.so.0
     * #0  Import_Utils::importStackTrace (fileName=..., scene=scene@entry=0x137c000) at /home/ralf.habacker/src/umbrello-3/umbrello/codeimport/import_utils.cpp:715
     */
    if (cols.size() > 2 && cols[0].startsWith(QStringLiteral("#"))) {
        QString file;
        sequence = cols.takeFirst();
        if (cols[cols.size()-2] == QStringLiteral("at")) {
            file = cols.takeLast();
            cols.takeLast();
        }
        else if (cols[cols.size()-2] == QStringLiteral("from")) {
            module = cols.takeLast();
            cols.takeLast();
        }

        if (cols[1] == QStringLiteral("in")) {
            cols.takeFirst(); // remove address
            cols.takeFirst(); // in
        }

        identifier = cols.join(QStringLiteral(" "));

        if (identifier.contains(QStringLiteral("::"))) {
            QStringList b = identifier.split( QStringLiteral("::"));
            // TODO handle multiple '::'
            package = b.takeFirst();
            method = b.join(QStringLiteral("::"));
        }
        else {
            method = identifier;
            package = module;
        }
        return true;
    }

    /**
     * Qtcreator/gdb
     * @verbatim
     *  6	Driver::ParseHelper::ParseHelper	driver.cpp	299	0x634c44
     * 31   g_main_context_dispatch /usr/lib64/libglib-2.0.so.0     0x7fffefe16316
     * ignoring
     * ... <more>                                                   0x7ffff41152d9
     * 13  ??                                                       0x7ffff41152d9
     * @endverbatim
     */
    else if (cols[cols.size()-1].startsWith(QStringLiteral("0x"))) {
        if (cols[0] == QStringLiteral("...") || cols[1] == QStringLiteral("??"))
            return false;

        sequence = cols.takeFirst();
        cols.takeLast();  // remove address

        QString line, file;
        if (cols.size() == 2) {
            module = cols.takeLast();
            identifier = cols.join(QStringLiteral(" "));
        } else if (cols.size() > 2) {
            line = cols.takeLast();
            file = cols.takeLast();
            identifier = cols.join(QStringLiteral(" "));
        }

        if (identifier.contains(QStringLiteral("::"))) {
            QStringList b = identifier.split( QStringLiteral("::"));
            method = b.takeLast();
            package = b.join(QStringLiteral("::"));
        }
        else {
            method = identifier;
            package = module;
        }

        if (package.isEmpty() && !file.isEmpty())
            package = file;

        if (!method.endsWith(QStringLiteral(")")))
            method.append(QStringLiteral("()"));

        return true;
    } else if (cols[cols.size()-1].contains(QStringLiteral("::"))) {
       QStringList b = s.split( QStringLiteral("::"));
       method = b.takeLast();
       package = b.join(QStringLiteral("::"));
       return true;
    }
    error = QStringLiteral("unsupported line format");
    return false;
}

/**
 * Import sequence diagram entries from a string list.
 *
 * @param lines String list with sequences
 * @param scene The diagram to import the sequences into.
 * @param sourceHint The source the sequences are imported from
 * @return true Import was successful.
 * @return false Import failed.
 */
bool importSequences(const QStringList &lines, UMLScene *scene, const QString &sourceHint)
{
    // object widget cache map
    QMap<QString, ObjectWidget*> objectsMap;

    // create "client widget"
    UMLDoc *umldoc = UMLApp::app()->document();
    QString name(QStringLiteral("client"));
    UMLObject *left = umldoc->findUMLObject(name, UMLObject::ot_Class);
    if (!left ) {
        left = new UMLObject(nullptr, name);
        left->setBaseType(UMLObject::ot_Class);
    }

    ObjectWidget *leftWidget = (ObjectWidget *)Widget_Factory::createWidget(scene, left);
    leftWidget->activate();
    // required to be savable
    scene->addWidgetCmd(leftWidget);
    objectsMap[name] = leftWidget;

    ObjectWidget *rightWidget = nullptr;
    ObjectWidget *mostRightWidget = leftWidget;
    MessageWidget *messageWidget = nullptr;
    // for further processing
    MessageWidgetList messages;

    QStringList l;
    SequenceLineFormat format = detectSequenceLineFormat(lines);
    if (format == GDB || format == QtCreatorGDB)
        for(const QString &s : lines)
            l.push_front(s);
    else
        l = lines;

    // for each line
    int index = 1;
    for(const QString &line : l) {
        QString stackframe, package, method, error;

        if (!parseSequenceLine(line, stackframe, package, method, error)) {
            if (!error.isEmpty()) {
                QString item = QString::fromLatin1("%1:%2:%3: %4: %5")
                        .arg(sourceHint).arg(index)
                        .arg(1).arg(line).arg(error);
                UMLApp::app()->log(item);
            }
            continue;
        }

        bool createObject = false;
        if (package.contains(method))
            createObject = true;

        if (package.isEmpty())
            package = QStringLiteral("unknown");

        // get or create right object widget
        if (objectsMap.contains(package)) {
            rightWidget = objectsMap[package];
        } else {
            UMLFolder *logicalView = UMLApp::app()->document()->rootFolder(Uml::ModelType::Logical);
            UMLObject *right = Import_Utils::createUMLObjectHierarchy(UMLObject::ot_Class, package, logicalView);

            rightWidget = (ObjectWidget *)Widget_Factory::createWidget(scene, right);
            rightWidget->setX(mostRightWidget->x() + mostRightWidget->width() + 10);
            rightWidget->activate();
            objectsMap[package] = rightWidget;
            scene->addWidgetCmd(rightWidget);
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
        scene->addWidgetCmd(messageWidget);
        messages.insert(0, messageWidget);

        leftWidget = rightWidget;
    }

    if (messages.isEmpty())
        return false;

    // adjust vertical position
    for(MessageWidget *w : messages) {
        w->setY(w->y() + 20);
    }

    // adjust heights starting from the last message
    MessageWidget *previous = messages.takeFirst();
    for(MessageWidget *w : messages) {
        w->setSize(w->width(), previous->y() - w->y() + previous->height() + 5);
        // adjust vertical line length of object widgets
        w->objectWidget(Uml::RoleType::A)->slotMessageMoved();
        w->objectWidget(Uml::RoleType::B)->slotMessageMoved();
        previous = w;
    }
    return true;
}

bool importClassGraph(const QStringList &lines, UMLScene *scene, const QString &sourceHint)
{
    UMLDoc *umldoc = UMLApp::app()->document();

    UMLWidget *lastWidget = nullptr;
    UMLClassifier *c = nullptr;
    QString methodIdentifier(QStringLiteral("()"));
    QMap<QString, QPointer<UMLWidget>> widgetList;
    int lineNumber = 0;
    // for each line
    for(const QString &line: lines) {
        lineNumber++;
        if (line.trimmed().isEmpty() || line.startsWith(QLatin1Char('#')) || line.startsWith(QStringLiteral("//")))
            continue;
        QStringList l = line.split(QStringLiteral(" "));
        if (l.size() == 1) {
            UMLObject *o = umldoc->findUMLObject(l[0], UMLObject::ot_Class);
            if (!o)
                o = Object_Factory::createUMLObject(UMLObject::ot_Class, l[0]);
            c = o->asUMLClassifier();
            // TODO: avoid multiple inserts
            UMLWidget *w = Widget_Factory::createWidget(scene, o);
            if (lastWidget)
                w->setX(lastWidget->x() + lastWidget->width() + 10);
            scene->setupNewWidget(w, false);
            scene->createAutoAssociations(w);
            scene->createAutoAttributeAssociations2(w);
            widgetList[l[0]] = w;
            lastWidget = w;
        } else if (l.size() == 3 && l[1].startsWith(QLatin1Char('-'))) { // associations
            UMLObject *o1 = umldoc->findUMLObject(l[0], UMLObject::ot_Class);
            if (!o1)
                o1 = Object_Factory::createUMLObject(UMLObject::ot_Class, l[0]);
            UMLObject *o2 = umldoc->findUMLObject(l[2], UMLObject::ot_Class);
            if (!o2)
                o2 = Object_Factory::createUMLObject(UMLObject::ot_Class, l[2]);
            bool swapObjects = false;
            Uml::AssociationType::Enum type = Uml::AssociationType::Unknown;
            UMLAssociation  *assoc = nullptr;
            bool newAssoc = false;
            if (l[1] == QStringLiteral("---")) {
                type = Uml::AssociationType::Association;
            } else if (l[1] == QStringLiteral("-->")) {
                type = Uml::AssociationType::UniAssociation;
            } else if (l[1] == QStringLiteral("-<>")) {
                type = Uml::AssociationType::Aggregation;
                swapObjects = true;
            } else if (l[1] == QStringLiteral("--*")) {
                type = Uml::AssociationType::Composition;
                swapObjects = true;
            } else if (l[1] == QStringLiteral("-|>")) {
                type = Uml::AssociationType::Generalization;
            }
            QPointer<UMLWidget> w1 = nullptr;
            QPointer<UMLWidget> w2 = nullptr;
            bool error = false;
            if (swapObjects) {
                w1 = widgetList[l[2]];
                w2 = widgetList[l[0]];
                if (w1 && w2) {
                    if (!assoc) {
                        assoc = umldoc->findAssociation(type, o1, o2);
                        if (!assoc) {
                            assoc = new UMLAssociation(type, o1, o2);
                            newAssoc = true;
                        }
                    }
                }
                else
                    error = true;
            } else {
                w1 = widgetList[l[0]];
                w2 = widgetList[l[2]];
                if (w1 && w2) {
                    if (!assoc) {
                        assoc = umldoc->findAssociation(type, o2, o1);
                        if (!assoc) {
                            assoc = new UMLAssociation(type, o2, o1);
                            newAssoc = true;
                        }
                    }
                }
                else
                    error = true;
            }
            if (!error) {
                if (newAssoc) {
                    assoc->setUMLPackage(umldoc->rootFolder(Uml::ModelType::Logical));
                    umldoc->addAssociation(assoc);
                }
                AssociationWidget* aw = AssociationWidget::create(scene, w1, type, w2, assoc);
                scene->addAssociation(aw);
            } else {
                // in case of error, assoc remains nullptr
                QString item = QString::fromLatin1("%1:%2:%3: %4: %5")
                        .arg(sourceHint).arg(lineNumber)
                        .arg(1).arg(line).arg(QStringLiteral("error:could not add association"));
                UMLApp::app()->log(item);
            }
        } else if (l[0].isEmpty() && c && l.size() == 2) {
            QString name = l.last();
            if (name.contains(methodIdentifier)) {
                name.remove(methodIdentifier);
                UMLOperation *m = Import_Utils::makeOperation(c, name);
                Import_Utils::insertMethod(c, m, Uml::Visibility::Public, QStringLiteral("void"), false, false);
            } else {
                Import_Utils::insertAttribute(c, Uml::Visibility::Public, name, QStringLiteral("int"));
            }
        } else if (l[0].isEmpty() && c && l.size() >= 3) {
            QString name = l.takeLast();
            l.takeFirst();
            QString v = l.first().toLower();
            Uml::Visibility::Enum visibility = Uml::Visibility::fromString(v, true);
            if (visibility == Uml::Visibility::Unknown)
                visibility = Uml::Visibility::Public;
            else
                l.takeFirst();
            QString type = l.join(QStringLiteral(" "));
            if (name.contains(methodIdentifier)) {
                name.remove(methodIdentifier);
                UMLOperation *m = Import_Utils::makeOperation(c, name);
                Import_Utils::insertMethod(c, m, visibility, type, false, false);
            } else {
                Import_Utils::insertAttribute(c, visibility, name, type);
            }
        } else {
            QString item = QString::fromLatin1("%1:%2:%3: %4: %5")
                    .arg(sourceHint).arg(lineNumber)
                    .arg(1).arg(line).arg(QStringLiteral("syntax error"));
            UMLApp::app()->log(item);
        }
    }
    return true;
}

/**
 * Import sequence diagram entries from a string list.
 *
 * @param lines String list with sequences
 * @param scene The diagram to import the sequences into.
 * @param sourceHint Source hint for use in error message in case of error.
 * @return true Import was successful.
 * @return false Import failed.
 */
bool importGraph(const QStringList &lines, UMLScene *scene, const QString &sourceHint)
{
    bool result = false;
    if (scene->isSequenceDiagram())
        result = importSequences(lines, scene, sourceHint);
    else if (scene->isClassDiagram())
        result = importClassGraph(lines, scene, sourceHint);
    if (result)
        scene->updateSceneRect();
    return result;
}

/**
 * Import graph entries from clipboard
 *
 * @param mimeData instance of mime data to import from
 * @param scene The diagram to import the graph into.
 * @return true Import successful.
 * @return false Import failed.
 */
bool importGraph(const QMimeData* mimeData, UMLScene *scene)
{
    QString requestedFormat = QStringLiteral("text/plain");
    if (!mimeData->hasFormat(requestedFormat))
        return false;

    QByteArray payload = mimeData->data(requestedFormat);
    if (!payload.size()) {
        return false;
    }
    QString data = QString::fromUtf8(payload);
    QStringList lines = data.split(QStringLiteral("\n"));

    UMLDoc *doc = UMLApp::app()->document();
    doc->beginPaste();
    bool result = importGraph(lines, scene, QLatin1String("from clipboard"));
    doc->endPaste();
    return result;
}

/**
 * Import graph entries from file
 *
 * @param fileName filename to import the graph from.
 * @param scene The diagram to import the graph into.
 * @return true Import successful.
 * @return false Import failed.
 */
bool importGraph(const QString &fileName, UMLScene *scene)
{
    QFile file(fileName);

    if(!file.open(QIODevice::ReadOnly))
        return false;

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    bool result = importGraph(lines, scene, fileName);
    return result;
}

/**
 * Check if name for a diagram is unique
 *
 * @param type type of diagram to check (set to undefined if to check against all diagrams)
 * @param name name of diagram to check
 * @return true - name is unique
 * @return false - name is not unique
 */
bool isUniqueDiagramName(Uml::DiagramType::Enum type, QString &name)
{
    bool found = false;
    for(UMLView  *view : UMLApp::app()->document()->viewIterator()) {
        if (type == Uml::DiagramType::Undefined || view->umlScene()->type() == type) {
            if (view->umlScene()->name() == name)
                found = true;
        }
    }
    return !found;
}

}  // end namespace Diagram_Utils
