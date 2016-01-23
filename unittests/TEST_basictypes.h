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

#ifndef TEST_BASICTYPES_H
#define TEST_BASICTYPES_H

#include <QObject>

class TEST_basictypes : public QObject
{
    Q_OBJECT
private slots:
    void test_dynamic_cast();
    void test_QString_english();
    void test_QString_non_english();
    void test_DomDocument_english();
    void test_DomDocument_non_english();
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
    void test_SequenceMessage_toString_fromString();
    void test_RoleType_toString_fromString();
    void test_ParameterDirection_toString_fromString();
    void test_ProgrammingLanguage_toString_fromString();
    void test_Region_toString_fromString();
    void test_Corner_toString_fromString();
};

#endif // TEST_BASICTYPES_H
