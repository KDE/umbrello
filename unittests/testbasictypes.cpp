/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testbasictypes.h"

// app includes
#include "basictypes.h"

// qt includes
#include <QtTest>
#include <QDomDocument>

//-----------------------------------------------------------------------------

class A
{
public:
  virtual ~A () { }
};

class B : public A
{
};

B* asB(A* p)
{
    return dynamic_cast<B*>(p);
}

A* getPointer()
{
    return 0;
}

void TestBasicTypes::test_dynamic_cast()
{
    QScopedPointer<A> a1(new A);
    B* b1 = dynamic_cast<B*> (a1.data());
    QVERIFY(!b1);
    QScopedPointer<A> a2(new B);
    B* b2 = dynamic_cast<B*> (a2.data());
    QVERIFY(b2);
    QScopedPointer<A> a3((B*)0);
    B* b3 = dynamic_cast<B*> (a3.data());
    QVERIFY(!b3);
    B* b4 = dynamic_cast<B*> (getPointer());
    QVERIFY(!b4);
    B* b5 = asB(getPointer());
    QVERIFY(!b5);
}

void TestBasicTypes::test_QString_english()
{
    QLocale _default = QLocale();
    QLocale english(QLocale::English);
    QLocale::setDefault(english);

    QString value1String(QStringLiteral("123.456"));
    float referenceValue = 123.456;
    qreal referenceDValue = 123.456;

    QString a = QString::number(referenceValue);
    QCOMPARE(value1String, a);

    bool ok;
    float value = value1String.toFloat(&ok);
    QVERIFY(ok);
    QCOMPARE(value, referenceValue);
    qreal dValue = value1String.toDouble(&ok);
    QVERIFY(ok);
    QCOMPARE(dValue, referenceDValue);
    QLocale::setDefault(_default);
}

void TestBasicTypes::test_QString_non_english()
{
    QLocale _default = QLocale();
    QLocale hungarian(QLocale::Hungarian);
    QLocale::setDefault(hungarian);

    QString value1String(QStringLiteral("123.456"));
    float referenceValue = 123.456;
    qreal referenceDValue = 123.456;

    QString a = QString::number(referenceValue);
    QCOMPARE(value1String, a);

    bool ok;
    float value = value1String.toFloat(&ok);
    QVERIFY(ok);
    QCOMPARE(value, referenceValue);
    qreal dValue = value1String.toDouble(&ok);
    QVERIFY(ok);
    QCOMPARE(dValue, referenceDValue);
    QLocale::setDefault(_default);
}

void TestBasicTypes::test_DomDocument_english()
{
    QLocale _default = QLocale();
    QLocale locale(QLocale::English);
    QLocale::setDefault(locale);
    QCOMPARE(QChar(QLatin1Char('.')), locale.decimalPoint());

    float fVar = 123.456;
    double dVar = 123.456;
    QString refValue(QStringLiteral("123.456"));
    QString localeValue;
    localeValue.replace(QLatin1Char('.'), _default.decimalPoint());

    QDomDocument doc("test");
    QDomElement root = doc.createElement("test");
    doc.appendChild(root);
    root.setAttribute("a", fVar);
    QString xml = doc.toString();
    QVERIFY2(xml.contains(refValue), xml.toLatin1().constData());

    // caused by bug in Qt xml
    root.setAttribute("a", dVar);
    xml = doc.toString();
    QVERIFY2(xml.contains(localeValue), xml.toLatin1().constData());

    root.setAttribute("a", QString::number(fVar));
    xml = doc.toString();
    QVERIFY(xml.contains(refValue));

    root.setAttribute("a", QString::number(dVar));
    xml = doc.toString();
    QVERIFY2(xml.contains(refValue), xml.toLatin1().constData());
    QLocale::setDefault(_default);
}

void TestBasicTypes::test_DomDocument_non_english()
{
    QLocale _default = QLocale();
    QLocale locale(QLocale::Hungarian);
    QLocale::setDefault(locale);
    QCOMPARE(QChar(QLatin1Char(',')), locale.decimalPoint());

    float fVar = 123.456;
    double dVar = 123.456;
    QString refValue(QStringLiteral("123.456"));
    QString localeValue;
    localeValue.replace(QLatin1Char('.'), _default.decimalPoint());

    QDomDocument doc("test");
    QDomElement root = doc.createElement("test");
    doc.appendChild(root);

    root.setAttribute("a", fVar);
    QString xml = doc.toString();
    QVERIFY2(xml.contains(refValue), xml.toLatin1().constData());

    // caused by bug in Qt xml
    root.setAttribute("a", dVar);
    xml = doc.toString();
    QVERIFY2(xml.contains(localeValue), xml.toLatin1().constData());

    root.setAttribute("a", QString::number(fVar));
    xml = doc.toString();
    QVERIFY2(xml.contains(refValue), xml.toLatin1().constData());

    root.setAttribute("a", QString::number(dVar));
    xml = doc.toString();
    QVERIFY2(xml.contains(refValue), xml.toLatin1().constData());
    QLocale::setDefault(_default);
}

void TestBasicTypes::test_ModelType_toString()
{
    Uml::ModelType::Enum model0 = Uml::ModelType::Logical;
    QCOMPARE(Uml::ModelType::toString(model0), QString("Logical"));
    Uml::ModelType::Enum model1 = Uml::ModelType::UseCase;
    QCOMPARE(Uml::ModelType::toString(model1), QString("UseCase"));
    Uml::ModelType::Enum model2(Uml::ModelType::Component);
    QCOMPARE(Uml::ModelType::toString(model2), QString("Component"));
    Uml::ModelType::Enum model3(Uml::ModelType::Deployment);
    QCOMPARE(Uml::ModelType::toString(model3), QString("Deployment"));
    Uml::ModelType::Enum model4(Uml::ModelType::EntityRelationship);
    QCOMPARE(Uml::ModelType::toString(model4), QString("EntityRelationship"));
}

void TestBasicTypes::test_ModelType_fromString()
{
    QString modelStr;
    Uml::ModelType::Enum model;

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

void TestBasicTypes::test_ModelType_forLoop()
{
    Uml::ModelType::Enum list[Uml::ModelType::N_MODELTYPES];
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        list[i] = Uml::ModelType::fromInt(i);
    }
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        QVERIFY(list[i] == Uml::ModelType::fromInt(i));
    }
    QVERIFY(list[0] == Uml::ModelType::Logical);
    QVERIFY(list[1] == Uml::ModelType::UseCase);
    QVERIFY(list[2] == Uml::ModelType::Component);
    QVERIFY(list[3] == Uml::ModelType::Deployment);
    QVERIFY(list[4] == Uml::ModelType::EntityRelationship);
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_Visibility_toString()
{
    Uml::Visibility::Enum visibility1 = Uml::Visibility::Public;
    QCOMPARE(Uml::Visibility::toString(visibility1), QString("public"));
    QCOMPARE(Uml::Visibility::toString(visibility1, true), QString("+"));
    Uml::Visibility::Enum visibility2 = Uml::Visibility::Private;
    QCOMPARE(Uml::Visibility::toString(visibility2), QString("private"));
    QCOMPARE(Uml::Visibility::toString(visibility2, true), QString("-"));
    Uml::Visibility::Enum visibility3(Uml::Visibility::Protected);
    QCOMPARE(Uml::Visibility::toString(visibility3), QString("protected"));
    QCOMPARE(Uml::Visibility::toString(visibility3, true), QString("#"));
    Uml::Visibility::Enum visibility4(Uml::Visibility::Implementation);
    QCOMPARE(Uml::Visibility::toString(visibility4), QString("implementation"));
    QCOMPARE(Uml::Visibility::toString(visibility4, true), QString("~"));
}

void TestBasicTypes::test_Visibility_fromString()
{
    Uml::Visibility::Enum visibility;
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

//-----------------------------------------------------------------------------

void TestBasicTypes::test_DiagramType_toString()
{
    Uml::DiagramType::Enum diagram0(Uml::DiagramType::Undefined);
    QCOMPARE(Uml::DiagramType::toString(diagram0), QString("Undefined"));
    Uml::DiagramType::Enum diagram1(Uml::DiagramType::Class);
    QCOMPARE(Uml::DiagramType::toString(diagram1), QString("Class"));
    Uml::DiagramType::Enum diagram2(Uml::DiagramType::UseCase);
    QCOMPARE(Uml::DiagramType::toString(diagram2), QString("UseCase"));
    Uml::DiagramType::Enum diagram3(Uml::DiagramType::Sequence);
    QCOMPARE(Uml::DiagramType::toString(diagram3), QString("Sequence"));
    Uml::DiagramType::Enum diagram4(Uml::DiagramType::Collaboration);
    QCOMPARE(Uml::DiagramType::toString(diagram4), QString("Collaboration"));
    Uml::DiagramType::Enum diagram5(Uml::DiagramType::State);
    QCOMPARE(Uml::DiagramType::toString(diagram5), QString("State"));
    Uml::DiagramType::Enum diagram6(Uml::DiagramType::Activity);
    QCOMPARE(Uml::DiagramType::toString(diagram6), QString("Activity"));
    Uml::DiagramType::Enum diagram7(Uml::DiagramType::Component);
    QCOMPARE(Uml::DiagramType::toString(diagram7), QString("Component"));
    Uml::DiagramType::Enum diagram8(Uml::DiagramType::Deployment);
    QCOMPARE(Uml::DiagramType::toString(diagram8), QString("Deployment"));
    Uml::DiagramType::Enum diagram9(Uml::DiagramType::EntityRelationship);
    QCOMPARE(Uml::DiagramType::toString(diagram9), QString("EntityRelationship"));
}

void TestBasicTypes::test_DiagramType_fromString()
{
    Uml::DiagramType::Enum diagram;
    diagram = Uml::DiagramType::fromString("Undefined");
    QVERIFY(diagram == Uml::DiagramType::Undefined);
    diagram = Uml::DiagramType::fromString("Class");
    QVERIFY(diagram == Uml::DiagramType::Class);
    diagram = Uml::DiagramType::fromString("UseCase");
    QVERIFY(diagram == Uml::DiagramType::UseCase);
    diagram = Uml::DiagramType::fromString("Sequence");
    QVERIFY(diagram == Uml::DiagramType::Sequence);
    diagram = Uml::DiagramType::fromString("Collaboration");
    QVERIFY(diagram == Uml::DiagramType::Collaboration);
    diagram = Uml::DiagramType::fromString("State");
    QVERIFY(diagram == Uml::DiagramType::State);
    diagram = Uml::DiagramType::fromString("Activity");
    QVERIFY(diagram == Uml::DiagramType::Activity);
    diagram = Uml::DiagramType::fromString("Component");
    QVERIFY(diagram == Uml::DiagramType::Component);
    diagram = Uml::DiagramType::fromString("Deployment");
    QVERIFY(diagram == Uml::DiagramType::Deployment);
    diagram = Uml::DiagramType::fromString("EntityRelationship");
    QVERIFY(diagram == Uml::DiagramType::EntityRelationship);
}

void TestBasicTypes::test_DiagramType_forLoop()
{
    Uml::DiagramType::Enum list[Uml::DiagramType::N_DIAGRAMTYPES];
    for (int i = 0; i < Uml::DiagramType::N_DIAGRAMTYPES; ++i) {
        list[i] = Uml::DiagramType::fromInt(i);
    }
    for (int i = 0; i < Uml::DiagramType::N_DIAGRAMTYPES; ++i) {
        QVERIFY(list[i] == Uml::DiagramType::fromInt(i));
    }
    QVERIFY(list[0] == Uml::DiagramType::Undefined);
    QVERIFY(list[1] == Uml::DiagramType::Class);
    QVERIFY(list[2] == Uml::DiagramType::UseCase);
    QVERIFY(list[3] == Uml::DiagramType::Sequence);
    QVERIFY(list[4] == Uml::DiagramType::Collaboration);
    QVERIFY(list[5] == Uml::DiagramType::State);
    QVERIFY(list[6] == Uml::DiagramType::Activity);
    QVERIFY(list[7] == Uml::DiagramType::Component);
    QVERIFY(list[8] == Uml::DiagramType::Deployment);
    QVERIFY(list[9] == Uml::DiagramType::EntityRelationship);
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_AssociationType_toString_fromString()
{
    for (int i = Uml::AssociationType::Generalization; i < Uml::AssociationType::Reserved; ++i) {
        Uml::AssociationType::Enum at = Uml::AssociationType::fromString(
                                      Uml::AssociationType::toString(
                                          Uml::AssociationType::fromInt(i)));
        QVERIFY(Uml::AssociationType::fromInt(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_SignatureType_toString_fromString()
{
    QVERIFY(Uml::SignatureType::NoSig ==
            Uml::SignatureType::fromString(Uml::SignatureType::toString(Uml::SignatureType::NoSig)));
    QVERIFY(Uml::SignatureType::ShowSig ==
            Uml::SignatureType::fromString(Uml::SignatureType::toString(Uml::SignatureType::ShowSig)));
    QVERIFY(Uml::SignatureType::SigNoVis ==
            Uml::SignatureType::fromString(Uml::SignatureType::toString(Uml::SignatureType::SigNoVis)));
    QVERIFY(Uml::SignatureType::NoSigNoVis ==
            Uml::SignatureType::fromString(Uml::SignatureType::toString(Uml::SignatureType::NoSigNoVis)));
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_TextRole_toString_fromString()
{
    for (int i = Uml::TextRole::Floating; i < Uml::TextRole::Reserved; ++i) {
        Uml::TextRole::Enum at = Uml::TextRole::fromString(
                                      Uml::TextRole::toString(
                                          Uml::TextRole::fromInt(i)));
        QVERIFY(Uml::TextRole::fromInt(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_Changeability_toString_fromString()
{
    for (int i = 900; i < 903; ++i) {
        Uml::Changeability::Enum at = Uml::Changeability::fromString(
                                      Uml::Changeability::toString(
                                          Uml::Changeability::fromInt(i)));
        QVERIFY(Uml::Changeability::fromInt(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_SequenceMessage_toString_fromString()
{
    for (int i = 1000; i < 1004; ++i) {
        Uml::SequenceMessage::Enum at = Uml::SequenceMessage::fromString(
                                      Uml::SequenceMessage::toString(
                                          Uml::SequenceMessage::fromInt(i)));
        QVERIFY(Uml::SequenceMessage::fromInt(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_RoleType_toString_fromString()
{
    for (int i = Uml::RoleType::A; i <= Uml::RoleType::B; ++i) {
        Uml::RoleType::Enum at = Uml::RoleType::fromString(
                                      Uml::RoleType::toString(
                                          Uml::RoleType::fromInt(i)));
        QVERIFY(Uml::RoleType::fromInt(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_ParameterDirection_toString_fromString()
{
    for (int i = Uml::ParameterDirection::In; i <= Uml::ParameterDirection::Out; ++i) {
        Uml::ParameterDirection::Enum at = Uml::ParameterDirection::fromString(
                                      Uml::ParameterDirection::toString(
                                          Uml::ParameterDirection::fromInt(i)));
        QVERIFY(Uml::ParameterDirection::fromInt(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_ProgrammingLanguage_toString_fromString()
{
    for (int i = Uml::ProgrammingLanguage::ActionScript; i < Uml::ProgrammingLanguage::Reserved; ++i) {
        Uml::ProgrammingLanguage::Enum at = Uml::ProgrammingLanguage::fromString(
                                      Uml::ProgrammingLanguage::toString(
                                          Uml::ProgrammingLanguage::fromInt(i)));
        QVERIFY(Uml::ProgrammingLanguage::fromInt(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_Region_toString_fromString()
{
    for (int i = Uml::Region::Error; i <= Uml::Region::SouthWest; ++i) {
        Uml::Region::Enum at = Uml::Region::fromString(
                                      Uml::Region::toString(
                                          Uml::Region::fromInt(i)));
        QVERIFY(Uml::Region::fromInt(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TestBasicTypes::test_Corner_toString_fromString()
{
    for (int i = Uml::Corner::TopLeft; i <= Uml::Corner::BottomLeft; i *= 2) {
        Uml::Corner::Enum at = Uml::Corner::fromString(
                                      Uml::Corner::toString(
                                          Uml::Corner::fromInt(i)));
        QVERIFY(Uml::Corner::fromInt(i) == at);
    }
}

//-----------------------------------------------------------------------------

QTEST_MAIN(TestBasicTypes)
