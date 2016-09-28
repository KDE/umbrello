// https://en.wikipedia.org/wiki/C%2B%2B11#Null_pointer_constant

// #1
char *pc = nullptr;     // OK
int  *pi = nullptr;     // OK
bool   b = nullptr;     // OK. b is false.
//int    i = nullptr;     // error

foo(nullptr);           // calls foo(nullptr_t), not foo(int);
