// https://en.wikipedia.org/wiki/C%2B%2B11#Explicit_overrides_and_final

// #1
struct Base {
    virtual void some_func(float);
};

struct Derived : Base {
    virtual void some_func(int) override; // ill-formed - doesn't override a base class method
};

// #2
struct Base1 final { };

struct Derived1 : Base1 { }; // ill-formed because the class Base1 has been marked final

// #3
struct Base2 {
    virtual bool c() const final override;  // from
    virtual bool d() const override final;  // https://bugs.kde.org/show_bug.cgi?id=397666
    virtual void f() final;
};

struct Derived2 : Base2 {
    void f(); // ill-formed because the virtual function Base2::f has been marked final
};
