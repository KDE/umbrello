#ifndef TESTBASE_H
#define TESTBASE_H

#include <QObject>

class TestBase : public QObject
{
    Q_OBJECT
public:
    explicit TestBase(QObject *parent = 0);

protected slots:
    void initTestCase();
    void cleanupTestCase();
};

#endif // TESTBASE_H
