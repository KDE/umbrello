
class AbstractClass {
public:
 virtual void test() = 0;
 virtual void test1();
};

class NoAbstractClass {
public:
 virtual void test();
 virtual void test1();
};
