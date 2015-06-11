#include "testbase.h"

#include "uml.h"

// qt includes
#include <QApplication>

static QApplication *app;
static UMLApp *umlApp;

TestBase::TestBase(QObject *parent)
  : QObject(parent)
{
}

void TestBase::initTestCase()
{
    // FIXME  we need to force QTEST_MAIN to use the GUI variant
    char **argv = { 0 };
    int argc = 0;
    app = new QApplication(argc , argv);
    QWidget *w = new QWidget;
    umlApp = new UMLApp(w);
}

void TestBase::cleanupTestCase()
{
    delete umlApp;
    delete app;
}
