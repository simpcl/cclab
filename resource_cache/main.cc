
#include "resource_cache.h"

#include <iostream>
using namespace std;


extern void Test();

int main()
{
  std::cout << "beginning ..." << std::endl;
  Test();

  ::ResourceCache<int> cache(100);

  return 0;
}
