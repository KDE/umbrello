
// class comments_class_operators_test
class comments_class_operators_test {
public:
    /* operator * c style single line comment */
    int operator * (int i);

    /** operator / c doxygen style single line comment */
    int operator / (int i);

    /*
     * operator % comment
     * c-style multiline comment
     */
    int operator % (int i);

    /**
     * operator | comment
     * c doxygen style multiline comment
     */
    int operator | (int i);

    // operator && description  c++ style single line comment
    int operator && (int i);

    /// operator || c++ doxygen style single line comment
    int operator || (int i);

    // operator + description line 1 out of 3 (c++ style)
    // operator + description line 2 out of 3 (c++ style)
    // operator + description line 3 out of 3 (c++ style)
    int operator + (int i);

    /// operator - description line 1 out of 2 (c++ doxygen style)
    /// operator - description line 2 out of 2 (c++ doxygen style)
    int operator - (int i);
};
