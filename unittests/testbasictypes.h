/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TEST_BASICTYPES_H
#define TEST_BASICTYPES_H

#include <QObject>

class TestBasicTypes : public QObject
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
