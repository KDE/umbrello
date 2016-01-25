// https://en.wikipedia.org/wiki/C%2B%2B11#Type_inference

// #1
auto some_strange_callable_type = std::bind(&some_function, _2, _1, some_object);
auto other_variable = 5;

// #2
int some_int;
decltype(some_int) other_integer_variable = 5;

// #3
for (auto itr = myvec.cbegin(); itr != myvec.cend(); ++itr) ;

// #4
for (auto& x : myvec) ;

// #5
#include <vector>
int main() {
    const std::vector<int> v(1);
    auto a = v[0];        // a has type int
    decltype(v[1]) b = 1; // b has type const int&, the return type of
                          //   std::vector<int>::operator[](size_type) const
    auto c = 0;           // c has type int
    auto d = c;           // d has type int
    decltype(c) e;        // e has type int, the type of the entity named by c
    decltype((c)) f = c;  // f has type int&, because (c) is an lvalue
    decltype(0) g;        // g has type int, because 0 is an rvalue
}
