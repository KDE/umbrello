template <class T>
class t1 {
public:
  t1() {}
  ~t1() {}
};

template <class T>
class t2: public t1<T> {
public:
  t2() {}
  ~t2() {}
};

template <class T, class U>
class t3 {
public:
  t3() {}
  ~t3() {}
};

template <class T, class U>
class t4: public t3<T, U> {
public:
  t4() {}
  ~t4() {}
};
