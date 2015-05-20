#ifndef TEST_BASICTYPES_H
#define TEST_BASICTYPES_H

#include <QObject>

class TEST_basictypes : public QObject
{
    Q_OBJECT
private slots:
    void test_dynamic_cast();
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
