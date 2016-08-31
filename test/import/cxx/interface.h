class Interface {
public:
 static const int member;
 virtual ~Interface();
 virtual void test() = 0;
 virtual void test1() = 0;
 virtual void test2() = 0;
};

class NoInterfaceWrongMember {
public:
 const int member;
 virtual void test() = 0;
 virtual void test1() = 0;
 virtual void test2() = 0;
};

class NoInterfaceNotAllMethodsPureVirtual {
public:
 static const int member;
 virtual void test();
 virtual void test1() = 0;
 virtual void test2() = 0;
};
