
// class comments_class_members_test
class comments_class_members_test {
public:
    /* member variable10 c-style single lime comment */
    int variable10;

    /** member variable11 c doxygen style single lime comment */
    int variable11;

    /*
     * member variable20
     * c-style multi line comment
     */
    int variable20;

    /**
     * member variable21
     * c doxygen style multi line comment
     */
    int variable21;

    // member variable30 c++ style single line comment
    int variable30;

    /// member variable31 c++ doxygen style single line comment
    int variable31;

    // member variable40 c++ style multi line comment 1 of 3 lines
    // member variable40 c++ style multi line comment 2 of 3 lines
    // member variable40 c++ style multi line comment 3 of 3 lines
    int variable40;

    /// member variable41 c++ doxygen style multi line comment 1 of 3 lines
    /// member variable41 c++ doxygen style multi line comment 2 of 3 lines
    /// member variable41 c++ doxygen style multi line comment 3 of 3 lines
    int variable41;
};
