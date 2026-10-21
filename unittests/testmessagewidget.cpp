/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testmessagewidget.h"

#include "messagewidget.h"
#include "objectwidget.h"
#include "umlfolder.h"
#include "umlscene.h"
#include "umlview.h"

#include <QGraphicsSceneMouseEvent>

class TestableMessageWidget : public MessageWidget
{
public:
    using MessageWidget::MessageWidget;

    bool inResizeArea(QGraphicsSceneMouseEvent *event)
    {
        return isInResizeArea(event);
    }
};

void TestMessageWidget::test_selfMessageResizeArea()
{
    UMLFolder folder(QStringLiteral("testfolder"));
    UMLView view(&folder);
    UMLScene *scene = view.umlScene();

    UMLObject object(nullptr);
    ObjectWidget objectWidget(scene, &object);
    scene->addWidgetCmd(&objectWidget);

    // A self-message has identical X coordinates at both ends.
    TestableMessageWidget message(scene, &objectWidget, 100, 100,
                                  Uml::SequenceMessage::Synchronous);
    scene->addWidgetCmd(&message);

    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMouseMove);

    // The bottom-right corner must be inside the resize area.
    event.setScenePos(QPointF(message.x() + message.width() - 1,
                              message.y() + message.height() - 1));
    QVERIFY(message.inResizeArea(&event));

    // A point above the bottom resize strip must not match.
    event.setScenePos(QPointF(message.x() + message.width() - 1,
                              message.y() + message.height() - 10));
    QVERIFY(!message.inResizeArea(&event));
}

QTEST_MAIN(TestMessageWidget)
