namespace namespace_enum_literal {
    /* should display the following literals in tree view * A = 1 * B = 2 * C * D */
    enum test { A = 1, B = 2, C, D };
};  // This last semicolon should not be there but most compilers accept it.
