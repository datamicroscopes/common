#include <microscopes/common/static_vector.hpp>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

int
main(void)
{
  microscopes::common::static_vector<int, 10> s1;
  microscopes::common::static_vector<size_t, 30> s2({1, 2});
  std::vector<unsigned> test3({1, 2, 3});
  microscopes::common::static_vector<unsigned, 3> s3(
      test3.begin(), test3.end()), s4;

  cout << (s3 < s4) << endl;

  map<microscopes::common::static_vector<size_t, 30>, int> m;
  m[s2] = 30;
  cout << m.size() << endl;

  return 0;
}
