#include <QtTest>
#include <QtCore>

#include "basictypes.h"

class TEST_basictypes : public QObject
{
    Q_OBJECT
private slots:
    void test_ModelType_toString();
    void test_ModelType_fromString();
    void test_ModelType_forLoop();
    void test_Visibility_toString();
    void test_Visibility_fromString();
};

void TEST_basictypes::test_ModelType_toString()
{
    Uml::ModelType modelDefault;
    QCOMPARE(modelDefault.toString(), QString("Logical"));
    Uml::ModelType model0(Uml::ModelType::Logical);
    QCOMPARE(model0.toString(), QString("Logical"));
    Uml::ModelType model1(Uml::ModelType::UseCase);
    QCOMPARE(model1.toString(), QString("UseCase"));
    Uml::ModelType model2(Uml::ModelType::Component);
    QCOMPARE(model2.toString(), QString("Component"));
    Uml::ModelType model3(Uml::ModelType::Deployment);
    QCOMPARE(model3.toString(), QString("Deployment"));
    Uml::ModelType model4(Uml::ModelType::EntityRelationship);
    QCOMPARE(model4.toString(), QString("EntityRelationship"));
}

void TEST_basictypes::test_ModelType_fromString()
{
    QString modelStr;
    Uml::ModelType model;

    modelStr = "Logical";
    model = Uml::ModelType::fromString(modelStr);
    QVERIFY(model == Uml::ModelType::Logical);
    modelStr = "UseCase";
    model = Uml::ModelType::fromString(modelStr);
    QVERIFY(model == Uml::ModelType::UseCase);
    modelStr = "Component";
    model = Uml::ModelType::fromString(modelStr);
    QVERIFY(model == Uml::ModelType::Component);
    modelStr = "Deployment";
    model = Uml::ModelType::fromString(modelStr);
    QVERIFY(model == Uml::ModelType::Deployment);
    modelStr = "EntityRelationship";
    model = Uml::ModelType::fromString(modelStr);
    QVERIFY(model == Uml::ModelType::EntityRelationship);
}

void TEST_basictypes::test_ModelType_forLoop()
{
    Uml::ModelType list[Uml::ModelType::N_MODELTYPES];
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        list[i] = Uml::ModelType(Uml::ModelType::Value(i));
    }
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        QVERIFY(list[i] == Uml::ModelType::Value(i));
    }
    QVERIFY(list[0] == Uml::ModelType::Logical);
    QVERIFY(list[1] == Uml::ModelType::UseCase);
    QVERIFY(list[2] == Uml::ModelType::Component);
    QVERIFY(list[3] == Uml::ModelType::Deployment);
    QVERIFY(list[4] == Uml::ModelType::EntityRelationship);
}

void TEST_basictypes::test_Visibility_toString()
{
    Uml::Visibility visibilityDefault;
    QCOMPARE(visibilityDefault.toString(), QString("public"));
    QCOMPARE(visibilityDefault.toString(true), QString("+"));
    Uml::Visibility visibility0(Uml::Visibility::Public);
    QCOMPARE(visibility0.toString(), QString("public"));
    QCOMPARE(visibility0.toString(true), QString("+"));
    Uml::Visibility visibility2(Uml::Visibility::Private);
    QCOMPARE(visibility2.toString(), QString("private"));
    QCOMPARE(visibility2.toString(true), QString("-"));
    Uml::Visibility visibility3(Uml::Visibility::Protected);
    QCOMPARE(visibility3.toString(), QString("protected"));
    QCOMPARE(visibility3.toString(true), QString("#"));
    Uml::Visibility visibility4(Uml::Visibility::Implementation);
    QCOMPARE(visibility4.toString(), QString("implementation"));
    QCOMPARE(visibility4.toString(true), QString("~"));
}

void TEST_basictypes::test_Visibility_fromString()
{
    Uml::Visibility visibility;
    visibility = Uml::Visibility::fromString("public");
    QVERIFY(visibility == Uml::Visibility::Public);
    visibility = Uml::Visibility::fromString("+");
    QVERIFY(visibility == Uml::Visibility::Public);
    visibility = Uml::Visibility::fromString("protected");
    QVERIFY(visibility == Uml::Visibility::Protected);
    visibility = Uml::Visibility::fromString("#");
    QVERIFY(visibility == Uml::Visibility::Protected);
    visibility = Uml::Visibility::fromString("private");
    QVERIFY(visibility == Uml::Visibility::Private);
    visibility = Uml::Visibility::fromString("-");
    QVERIFY(visibility == Uml::Visibility::Private);
    visibility = Uml::Visibility::fromString("signals");
    QVERIFY(visibility == Uml::Visibility::Protected);
    visibility = Uml::Visibility::fromString("class");
    QVERIFY(visibility == Uml::Visibility::Private);
    visibility = Uml::Visibility::fromString("anything else");
    QVERIFY(visibility == Uml::Visibility::Public);
}

QTEST_MAIN(TEST_basictypes)
#include "test_basictypes.moc"
