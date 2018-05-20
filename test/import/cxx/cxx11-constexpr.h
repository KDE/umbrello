// https://en.wikipedia.org/wiki/C%2B%2B11#constexpr_.E2.80.93_Generalized_constant_expressions

// #1
constexpr int get_five() {return 5;}
int some_value[get_five() + 7]; // Create an array of 12 integers. Legal C++11

// #2
constexpr double earth_gravitational_acceleration = 9.8;
constexpr double moon_gravitational_acceleration = earth_gravitational_acceleration / 6.0;

class ConstExprConstructorDeclaration {
    constexpr ConstExprConstructorDeclaration(QString &param);
};

class ConstExprConstructorDefinition {
    constexpr ConstExprConstructorDefinition(QString &param) {}
};

class ExplicitConstExprConstructorDeclaration {
    explicit constexpr ExplicitConstructorDeclaration(QString &param);
};

class ExplicitConstExprConstructorDefinition {
    explicit constexpr ExplicitConstructorDefinition(QString &param) {}
};
