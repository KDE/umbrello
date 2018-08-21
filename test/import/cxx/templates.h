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
