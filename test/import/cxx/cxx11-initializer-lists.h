// https://en.wikipedia.org/wiki/C%2B%2B11#Initializer_lists

// #1
class SequenceClass {
public:
    SequenceClass(std::initializer_list<int> list);
};

// #2
SequenceClass some_var = {1, 4, 5, 6};

// #3
void function_name(std::initializer_list<float> list);

function_name({1.0f, -3.45f, -0.4f});


// #4
std::vector<std::string> v = { "xyzzy", "plugh", "abracadabra" };
std::vector<std::string> v({ "xyzzy", "plugh", "abracadabra" });
std::vector<std::string> v{ "xyzzy", "plugh", "abracadabra" }; // see "Uniform initialization" below
