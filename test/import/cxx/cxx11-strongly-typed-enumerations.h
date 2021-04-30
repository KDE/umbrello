// https://en.wikipedia.org/wiki/C%2B%2B11#Strongly_typed_enumerations

// #1
enum class Enumeration {
    Val1,
    Val2,
    Val3 = 100,
    Val4 // = 101
};

// #2
enum class Enum2 : unsigned int {Val1, Val2};

// #3
enum Enum3 : unsigned long {Val1 = 1, Val2};

// #4
//enum Enum1;                      // Illegal in C++03 and C++11; the underlying type cannot be determined.
enum Enum4 : unsigned int;       // Legal in C++11, the underlying type is explicitly specified.
enum class Enum5;                // Legal in C++11, the underlying type is int.

enum class Enum6 : unsigned int; // Legal in C++11.
//enum Enum2 : unsigned short;     // Illegal in C++11, because Enum2 was previously declared with a different underlying type.
