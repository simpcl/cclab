#include "resource_cache.h"

#include <iostream>
using namespace std;

class Object {
 public:
  Object(int res);
  ~Object();

 public:
  void Show();

 private:
  int res_;
};


Object::Object(int res) : res_(res) {
}

Object::~Object() {
  std::cout << "Object destroy! " << "res: " << res_ << std::endl;
}

void Object::Show() {
  std::cout << "Show Object res: " << res_ << std::endl;
}

void Test() {
  ResourceCache<Object> rc(10);
  rc.Get("abc");

  std::shared_ptr<Object> obj = std::make_shared<Object>(1);
  bool ret = rc.Set("1", obj);
  if (!ret) {
    cout << "cache set 1 failed!" << endl;
  }
  std::shared_ptr<Object> obj2 = rc.Get("1");
  //obj2->Show();
  if (obj2) {
    obj2->Show();
  }

  rc.Set("1", std::make_shared<Object>(2));
  if (!ret) {
    cout << "cache set 1 failed!" << endl;
  }
  obj2 = rc.Get("1");
  if (obj2) {
    obj2->Show();
  }
}

