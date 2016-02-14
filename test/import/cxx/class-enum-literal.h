class class_enum_literal {
    public:
        /* should display the following literals in tree view * A = 1 * B = 2 * C * D */
        enum test { A = 1, B = 2, C, D };
};
