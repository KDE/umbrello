/*
    Copyright 2011  Andi Fischer  <andi.fischer@hispeed.ch>

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
    void test_DiagramType_toString();
    void test_DiagramType_fromString();
    void test_DiagramType_forLoop();
    void test_AssociationType_toString_fromString();
    void test_SignatureType_toString_fromString();
    void test_TextRole_toString_fromString();
    void test_Changeability_toString_fromString();
};

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

void TEST_basictypes::test_DiagramType_toString()
{
    Uml::DiagramType diagramDefault;
    QCOMPARE(diagramDefault.toString(), QString("Undefined"));
    Uml::DiagramType diagram0(Uml::DiagramType::Undefined);
    QCOMPARE(diagram0.toString(), QString("Undefined"));
    Uml::DiagramType diagram1(Uml::DiagramType::Class);
    QCOMPARE(diagram1.toString(), QString("Class"));
    Uml::DiagramType diagram2(Uml::DiagramType::UseCase);
    QCOMPARE(diagram2.toString(), QString("UseCase"));
    Uml::DiagramType diagram3(Uml::DiagramType::Sequence);
    QCOMPARE(diagram3.toString(), QString("Sequence"));
    Uml::DiagramType diagram4(Uml::DiagramType::Collaboration);
    QCOMPARE(diagram4.toString(), QString("Collaboration"));
    Uml::DiagramType diagram5(Uml::DiagramType::State);
    QCOMPARE(diagram5.toString(), QString("State"));
    Uml::DiagramType diagram6(Uml::DiagramType::Activity);
    QCOMPARE(diagram6.toString(), QString("Activity"));
    Uml::DiagramType diagram7(Uml::DiagramType::Component);
    QCOMPARE(diagram7.toString(), QString("Component"));
    Uml::DiagramType diagram8(Uml::DiagramType::Deployment);
    QCOMPARE(diagram8.toString(), QString("Deployment"));
    Uml::DiagramType diagram9(Uml::DiagramType::EntityRelationship);
    QCOMPARE(diagram9.toString(), QString("EntityRelationship"));
}

void TEST_basictypes::test_DiagramType_fromString()
{
    Uml::DiagramType diagram;
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

void TEST_basictypes::test_DiagramType_forLoop()
{
    Uml::DiagramType list[Uml::DiagramType::N_DIAGRAMTYPES];
    for (int i = 0; i < Uml::DiagramType::N_DIAGRAMTYPES; ++i) {
        list[i] = Uml::DiagramType(Uml::DiagramType::Value(i));
    }
    for (int i = 0; i < Uml::DiagramType::N_DIAGRAMTYPES; ++i) {
        QVERIFY(list[i] == Uml::DiagramType::Value(i));
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

void TEST_basictypes::test_AssociationType_toString_fromString()
{
    for (int i = Uml::AssociationType::Generalization; i < Uml::AssociationType::Relationship; ++i) {
        Uml::AssociationType at = Uml::AssociationType::fromString(
                                      Uml::AssociationType::toString(
                                          Uml::AssociationType::Value(i)));
        QVERIFY(Uml::AssociationType::Value(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TEST_basictypes::test_SignatureType_toString_fromString()
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

void TEST_basictypes::test_TextRole_toString_fromString()
{
    for (int i = Uml::TextRole::Floating; i < Uml::TextRole::ChangeB; ++i) {
        Uml::TextRole at = Uml::TextRole::fromString(
                                      Uml::TextRole::toString(
                                          Uml::TextRole::Value(i)));
        QVERIFY(Uml::TextRole::Value(i) == at);
    }
}

//-----------------------------------------------------------------------------

void TEST_basictypes::test_Changeability_toString_fromString()
{
    for (int i = 900; i < 903; ++i) {
        Uml::Changeability at = Uml::Changeability::fromString(
                                      Uml::Changeability::toString(
                                          Uml::Changeability::Value(i)));
        QVERIFY(Uml::Changeability::Value(i) == at);
    }
}

//-----------------------------------------------------------------------------

QTEST_MAIN(TEST_basictypes)
#include "test_basictypes.moc"
