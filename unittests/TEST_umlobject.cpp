/*
    Copyright 2015  Ralf Habacker  <ralf.habacker@freenet.de>

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

#include "TEST_umlobject.h"

// app include
#include "folder.h"
#include "package.h"
#include "stereotype.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"

// kde includes
#include <KLocalizedString>

const bool IS_NOT_IMPL = false;

//-----------------------------------------------------------------------------

void TEST_UMLObject::test_copyInto()
{
    UMLPackage parent("Test Parent");
    UMLObject a("Test A");
    a.setUMLPackage(&parent);
    UMLObject b("Test B");
    b.setUMLPackage(&parent);
    b.copyInto(&a);
    QCOMPARE(a, b);
}

void TEST_UMLObject::test_clone()
{
    UMLPackage parent("Test Parent");
    UMLObject a("Test A");
    a.setUMLPackage(&parent);
    UMLObject &b = *a.clone();
    QCOMPARE(a, b);
}

void TEST_UMLObject::test_doc()
{
    UMLPackage parent("Test Parent");
    UMLObject a("Test A");
    QCOMPARE(a.hasDoc(), false);
    a.setDoc(QLatin1String("new doc"));
    QCOMPARE(a.hasDoc(), true);
    QCOMPARE(a.doc(), QLatin1String("new doc"));
}

void TEST_UMLObject::test_equal()
{
    UMLPackage parent("Test Parent");
    UMLObject a("Test A", Uml::ID::Reserved);
    a.setUMLPackage(&parent);
    UMLObject b(a);
    UMLObject c("Test A", Uml::ID::Reserved);
    c.setUMLPackage(&parent);
    UMLObject d("Test B", Uml::ID::None);
    QCOMPARE(a, b);
    QCOMPARE(a, c);
    QCOMPARE(b, c);
    QCOMPARE(c == d, false);
}

void TEST_UMLObject::test_fullyQualifiedName()
{
    UMLObject* a = new UMLObject("Test A");
    cleanupOnExit(a);
    QCOMPARE(a->fullyQualifiedName(), QLatin1String("Test A"));

    UMLPackage* topParent = new UMLPackage("Top Parent");
    cleanupOnExit(topParent);
    UMLPackage* parent = new UMLPackage("Test Parent");
    cleanupOnExit(parent);
    parent->setUMLPackage(topParent);
    a->setUMLPackage(parent);
    QCOMPARE(a->umlPackage()->fullyQualifiedName(), a->package());
    QCOMPARE(a->fullyQualifiedName(), QLatin1String("Top Parent::Test Parent::Test A"));
    QCOMPARE(a->fullyQualifiedName(QLatin1String("-")), QLatin1String("Top Parent-Test Parent-Test A"));

    UMLFolder *f = UMLApp::app()->document()->rootFolder(Uml::ModelType::Logical);
    parent->setUMLPackage(f);
    QCOMPARE(a->fullyQualifiedName(QLatin1String("::"), true), QLatin1String("Logical View::Test Parent::Test A"));
}

void TEST_UMLObject::test_isAbstract()
{
    UMLObject a("Test A");
    QCOMPARE(a.isAbstract(), false);
    a.setAbstract(true);
    QCOMPARE(a.isAbstract(), true);
}

void TEST_UMLObject::test_isStatic()
{
    UMLObject a("Test A");
    QCOMPARE(a.isStatic(), false);
    a.setStatic(true);
    QCOMPARE(a.isStatic(), true);
}

class TestUMLObject : public UMLObject
{
public:
    TestUMLObject(const QString& name = QString(), Uml::ID::Type id = Uml::ID::None)
      : UMLObject(name, id)
    {
    }

    UMLObject *secondary() const
    {
        return m_pSecondary.data();
    }
};

void TEST_UMLObject::test_resolveRef()
{
    UMLPackage parent("Test Parent");
    UMLStereotype *stereotype = UMLApp::app()->document()->createStereotype("test");

    UMLObject a("Test A");
    // no resolve
    a.setUMLPackage(&parent);
    QCOMPARE(a.resolveRef(), true);

    // secondary
    a.setSecondaryId(Uml::ID::toString(stereotype->id()));
    QCOMPARE(a.resolveRef(), true);

    // secondary fallback
    a.setSecondaryId(QLatin1String(""));
    a.setSecondaryFallback(Uml::ID::toString(stereotype->id()));
    QCOMPARE(a.resolveRef(), true);

    // unknown stereotype
    TestUMLObject b("Test B");
    UMLStereotype stereotype2("test");
    b.setUMLPackage(&parent);
    b.setSecondaryId(Uml::ID::toString(stereotype2.id()));
    QCOMPARE(b.resolveRef(), true);
    // resolveRef creates an "undef" datatype and assigns it to m_Secondary
    QCOMPARE(b.secondary()->name(), QLatin1String("undef"));
}

void TEST_UMLObject::test_saveAndLoad()
{
    UMLPackage parent("Test Parent");
    UMLObject a("Test A");
    a.setUMLPackage(&parent);
    a.setStereotypeCmd("test");
    QDomDocument doc;
    QDomElement save = a.save("test", doc);
    UMLObject b;
    b.setUMLPackage(&parent);
    QCOMPARE(b.loadFromXMI(save), true);
    QCOMPARE(a, b);
}

void TEST_UMLObject::test_setBaseType()
{
    UMLObject a("Test A");
    QCOMPARE(a.baseType(), UMLObject::ot_UMLObject);
    a.setBaseType(UMLObject::ot_Class);
    QCOMPARE(a.baseType(), UMLObject::ot_Class);
}

void TEST_UMLObject::test_setSterotype()
{
    UMLObject a("Test A");
    QCOMPARE(a.stereotype(), QLatin1String(""));
    a.setStereotypeCmd(QLatin1String("test"));
    QCOMPARE(a.stereotype(), QLatin1String("test"));
}

void TEST_UMLObject::test_setUMLPackage()
{
    UMLPackage parent("Test Parent");
    UMLObject a("Test A");
    QCOMPARE(a.umlPackage(), (UMLPackage*)0);
    a.setUMLPackage(&parent);
    QCOMPARE(a.umlPackage(), &parent);
}


void TEST_UMLObject::test_setVisibility()
{
    UMLObject a("Test A");
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

void TEST_UMLObject::test_toString()
{
    QCOMPARE(UMLObject::toString(UMLObject::ot_Class), QLatin1String("ot_Class"));
    QCOMPARE(UMLObject::toI18nString(UMLObject::ot_Class), i18n("Class &name:"));
}

QTEST_MAIN(TEST_UMLObject)
