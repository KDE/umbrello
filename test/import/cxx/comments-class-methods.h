
// class comments_class_methods_test
class comments_class_methods_test {
public:
    /* method test00 c style single line comment */
    void test00(char c);

    /** method test21 c doxygen style single line comment */
    void test01(char c);

    /*
     * method test10
     * c-style multiline comment
     */
    void test10(char *);

    /**
     * method test1
     * doxygen c-style multiline comment
     */
    void test11(char *);

    // method test00 c++ style single line comment
    void test20();

    /// method test01 c++ doxygen style single line comment
    void test21();

    // method test30 c++ style multi line comment 1 of 3 lines
    // method test30 c++ style multi line comment 2 of 3 lines
    // method test30 c++ style multi line comment 3 of 3 lines
    void test30();

    /// method test30 c++ doxygen style multi line comment 1 of 3 lines
    /// method test30 c++ doxygen style multi line comment 2 of 3 lines
    /// method test30 c++ doxygen style multi line comment 3 of 3 lines
    void test31();
};
