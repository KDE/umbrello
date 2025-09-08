/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testumlobject.h"

// app include
#include "umlattribute.h"
#include "classifier.h"
#include "folder.h"
#include "operation.h"
#include "package.h"
#include "stereotype.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QXmlStreamWriter>

//-----------------------------------------------------------------------------

void TestUMLObject::test_copyInto()
{
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLObject a(QStringLiteral("Test A"));
    a.setUMLPackage(&parent);
    UMLObject b(QStringLiteral("Test B"));
    b.setUMLPackage(&parent);
    b.copyInto(&a);
    QCOMPARE(a, b);
    UMLClassifier c(QStringLiteral("Test Classifier"));
    UMLOperation op(&c, QStringLiteral("Test Parent"));
    UMLAttribute at(&op, QStringLiteral("Attribute"));
    UMLAttribute at2(&op,QStringLiteral("Attribute 2"));
    at2.copyInto(&at);
    QCOMPARE(at, at2);
    QCOMPARE(at2.umlParent(), at.umlParent());
}

void TestUMLObject::test_clone()
{
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLObject a(QStringLiteral("Test A"));
    a.setUMLPackage(&parent);
    UMLObject &b = *a.clone();
    QCOMPARE(a, b);
}

void TestUMLObject::test_doc()
{
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLObject a(QStringLiteral("Test A"));
    QCOMPARE(a.hasDoc(), false);
    a.setDoc(QStringLiteral("new doc"));
    QCOMPARE(a.hasDoc(), true);
    QCOMPARE(a.doc(), QStringLiteral("new doc"));
}

void TestUMLObject::test_equal()
{
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLObject a(QStringLiteral("Test A"), Uml::ID::Reserved);
    a.setUMLPackage(&parent);
    UMLObject b(a);
    UMLObject c(QStringLiteral("Test A"), Uml::ID::Reserved);
    c.setUMLPackage(&parent);
    UMLObject d(QStringLiteral("Test B"), Uml::ID::None);
    QCOMPARE(a, b);
    QCOMPARE(a, c);
    QCOMPARE(b, c);
    QCOMPARE(c == d, false);
}

void TestUMLObject::test_fullyQualifiedName()
{
    UMLObject* a = new UMLObject(QStringLiteral("Test A"));
    cleanupOnExit(a);
    QCOMPARE(a->fullyQualifiedName(), QStringLiteral("Test A"));

    UMLPackage* topParent = new UMLPackage(QStringLiteral("Top Parent"));
    cleanupOnExit(topParent);
    UMLPackage* parent = new UMLPackage(QStringLiteral("Test Parent"));
    cleanupOnExit(parent);
    parent->setUMLPackage(topParent);
    a->setUMLPackage(parent);
    QCOMPARE(a->umlPackage()->fullyQualifiedName(), a->package());
    QCOMPARE(a->fullyQualifiedName(), QStringLiteral("Top Parent::Test Parent::Test A"));
    QCOMPARE(a->fullyQualifiedName(QStringLiteral("-")), QStringLiteral("Top Parent-Test Parent-Test A"));

    UMLFolder *f = UMLApp::app()->document()->rootFolder(Uml::ModelType::Logical);
    parent->setUMLPackage(f);
    QCOMPARE(a->fullyQualifiedName(QStringLiteral("::"), true), QStringLiteral("Logical View::Test Parent::Test A"));
}

void TestUMLObject::test_isAbstract()
{
    UMLObject a(QStringLiteral("Test A"));
    QCOMPARE(a.isAbstract(), false);
    a.setAbstract(true);
    QCOMPARE(a.isAbstract(), true);
}

void TestUMLObject::test_isStatic()
{
    UMLObject a(QStringLiteral("Test A"));
    QCOMPARE(a.isStatic(), false);
    a.setStatic(true);
    QCOMPARE(a.isStatic(), true);
}

typedef TestUML<UMLObject, const QString &> TESTUMLObject;

void TestUMLObject::test_resolveRef()
{
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLStereotype *stereotype = UMLApp::app()->document()->createStereotype(QStringLiteral("test"));

    UMLObject a(QStringLiteral("Test A"));
    // no resolve
    a.setUMLPackage(&parent);
    QCOMPARE(a.resolveRef(), true);

    // secondary
    a.setSecondaryId(Uml::ID::toString(stereotype->id()));
    QCOMPARE(a.resolveRef(), true);

    // secondary fallback
    a.setSecondaryId(QStringLiteral(""));
    a.setSecondaryFallback(Uml::ID::toString(stereotype->id()));
    QCOMPARE(a.resolveRef(), true);

    // unknown stereotype
    TESTUMLObject b(QStringLiteral("Test B"));
    UMLStereotype stereotype2(QStringLiteral("test"));
    b.setUMLPackage(&parent);
    b.setSecondaryId(Uml::ID::toString(stereotype2.id()));
    QCOMPARE(b.resolveRef(), true);
    // resolveRef creates an "undef" datatype and assigns it to m_Secondary
    QCOMPARE(b.secondary()->name(), QStringLiteral("undef"));
}

void TestUMLObject::test_saveAndLoad()
{
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLObject a(QStringLiteral("Test A"));
    a.setUMLPackage(&parent);
    a.setStereotypeCmd(QStringLiteral("test"));

    // save
    QString xml;
    QXmlStreamWriter writer(&xml);
    a.save1(writer, QStringLiteral("test"));
    writer.writeEndElement();

    // convert XML string to QDomElement
    QString error;
    int line;
    QDomDocument doc;
    QVERIFY(doc.setContent(xml, &error, &line));
    QDomElement save = doc.firstChild().toElement();

    // load
    UMLObject b;
    b.setUMLPackage(&parent);
    QCOMPARE(b.loadFromXMI(save), true);
    QCOMPARE(a, b);
}

void TestUMLObject::test_setBaseType()
{
    UMLObject a(QStringLiteral("Test A"));
    QCOMPARE(a.baseType(), UMLObject::ot_UMLObject);
    a.setBaseType(UMLObject::ot_Class);
    QCOMPARE(a.baseType(), UMLObject::ot_Class);
}

void TestUMLObject::test_setStereotype()
{
    UMLObject a(QStringLiteral("Test A"));
    QCOMPARE(a.stereotype(), QStringLiteral(""));
    a.setStereotypeCmd(QStringLiteral("test"));
    QCOMPARE(a.stereotype(), QStringLiteral("test"));
}

void TestUMLObject::test_setUMLPackage()
{
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLObject a(QStringLiteral("Test A"));
    QCOMPARE(a.umlPackage(), (UMLPackage*)0);
    a.setUMLPackage(&parent);
    QCOMPARE(a.umlPackage(), &parent);
}


void TestUMLObject::test_setVisibility()
{
    UMLObject a(QStringLiteral("Test A"));
    QVERIFY(a.visibility() == Uml::Visibility::Public);
    a.setVisibilityCmd(Uml::Visibility::Protected);
    QVERIFY(a.visibility() == Uml::Visibility::Protected);
    a.setVisibilityCmd(Uml::Visibility::Private);
    QVERIFY(a.visibility() == Uml::Visibility::Private);
    a.setVisibilityCmd(Uml::Visibility::Implementation);
    QVERIFY(a.visibility() == Uml::Visibility::Implementation);
    a.setVisibilityCmd(Uml::Visibility::FromParent);
    QVERIFY(a.visibility() == Uml::Visibility::FromParent);
}

void TestUMLObject::test_toString()
{
    QCOMPARE(UMLObject::toString(UMLObject::ot_Class), QStringLiteral("ot_Class"));
    QCOMPARE(UMLObject::toI18nString(UMLObject::ot_Class), i18n("Class &name:"));
}

void TestUMLObject::test_dynamic_cast()
{
    QScopedPointer<UMLObject> a1(new UMLClassifier);
    const UMLClassifier *b = a1->asUMLClassifier();
    QVERIFY(b);
    UMLObject *a2 = nullptr;
    b = a2->asUMLClassifier();
    QVERIFY(!b);
}

void TestUMLObject::test_isUMLXXX()
{
    UMLObject a(QStringLiteral("Test A"));
    QVERIFY(a.isUMLObject());
    a.setBaseType(UMLObject::ObjectType::ot_Actor);
    QVERIFY(a.isUMLActor());
    a.setBaseType(UMLObject::ObjectType::ot_Artifact);
    QVERIFY(a.isUMLArtifact());
    a.setBaseType(UMLObject::ObjectType::ot_Association);
    QVERIFY(a.isUMLAssociation());
    a.setBaseType(UMLObject::ObjectType::ot_Attribute);
    QVERIFY(a.isUMLAttribute());
    a.setBaseType(UMLObject::ObjectType::ot_Category);
    QVERIFY(a.isUMLCategory());
    a.setBaseType(UMLObject::ObjectType::ot_CheckConstraint);
    QVERIFY(a.isUMLCheckConstraint());
    a.setBaseType(UMLObject::ObjectType::ot_Class);
    QVERIFY(a.isUMLClassifier());
    a.setBaseType(UMLObject::ObjectType::ot_Component);
    QVERIFY(a.isUMLComponent());
    a.setBaseType(UMLObject::ObjectType::ot_Datatype);
    QVERIFY(a.isUMLDatatype());
    a.setBaseType(UMLObject::ObjectType::ot_Entity);
    QVERIFY(a.isUMLEntity());
    a.setBaseType(UMLObject::ObjectType::ot_EntityAttribute);
    QVERIFY(a.isUMLEntityAttribute());
    a.setBaseType(UMLObject::ObjectType::ot_EntityConstraint);
    QVERIFY(a.isUMLEntityConstraint());
    a.setBaseType(UMLObject::ObjectType::ot_Enum);
    QVERIFY(a.isUMLEnum());
    a.setBaseType(UMLObject::ObjectType::ot_EnumLiteral);
    QVERIFY(a.isUMLEnumLiteral());
    a.setBaseType(UMLObject::ObjectType::ot_Folder);
    QVERIFY(a.isUMLFolder());
    a.setBaseType(UMLObject::ObjectType::ot_ForeignKeyConstraint);
    QVERIFY(a.isUMLForeignKeyConstraint());
    a.setBaseType(UMLObject::ObjectType::ot_Instance);
    QVERIFY(a.isUMLInstance());
    a.setBaseType(UMLObject::ObjectType::ot_InstanceAttribute);
    QVERIFY(a.isUMLInstanceAttribute());
//    UMLClassifier has isInterface()
//    a.setBaseType(UMLObject::ObjectType::ot_Interface);
//    QVERIFY(a.isUMLInterface());
    a.setBaseType(UMLObject::ObjectType::ot_Node);
    QVERIFY(a.isUMLNode());
    a.setBaseType(UMLObject::ObjectType::ot_Operation);
    QVERIFY(a.isUMLOperation());
    a.setBaseType(UMLObject::ObjectType::ot_Package);
    QVERIFY(a.isUMLPackage());
    a.setBaseType(UMLObject::ObjectType::ot_Port);
    QVERIFY(a.isUMLPort());
    a.setBaseType(UMLObject::ObjectType::ot_Role);
    QVERIFY(a.isUMLRole());
    a.setBaseType(UMLObject::ObjectType::ot_Stereotype);
    QVERIFY(a.isUMLStereotype());
//    a.setBaseType(UMLObject::ObjectType::ot_SubSystem);
//    QVERIFY(a.isUMLSubSystem());
    a.setBaseType(UMLObject::ObjectType::ot_Template);
    QVERIFY(a.isUMLTemplate());
    a.setBaseType(UMLObject::ObjectType::ot_UMLObject);
    QVERIFY(a.isUMLObject());
    a.setBaseType(UMLObject::ObjectType::ot_UniqueConstraint);
    QVERIFY(a.isUMLUniqueConstraint());
    a.setBaseType(UMLObject::ObjectType::ot_UseCase);
    QVERIFY(a.isUMLUseCase());
}

QTEST_MAIN(TestUMLObject)
