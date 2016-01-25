// https://en.wikipedia.org/wiki/C%2B%2B11#Uniform_initialization

// #1
struct BasicStruct {
    int x;
    double y;
};

struct AltStruct {
    AltStruct(int x, double y) : x_{x}, y_{y} {}

    private:
        int x_;
        double y_;
};

BasicStruct var1{5, 3.2};
AltStruct var2{2, 4.3};

// #2
struct IdString {
    std::string name;
    int identifier;
};

IdString get_string() {
    return {"foo", 42}; //Note the lack of explicit type.
}

// #3
std::vector<int> the_vec{4};
