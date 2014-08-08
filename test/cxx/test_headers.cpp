#include <microscopes/common/static_vector.hpp>
#include <vector>
#include <map>

int
main(void)
{
  microscopes::common::static_vector<int, 10> s1;
  microscopes::common::static_vector<size_t, 30> s2({1, 2});
  std::vector<unsigned> test3({1, 2, 3});
  microscopes::common::static_vector<unsigned, 3> s3(
      test3.begin(), test3.end());

  std::map<microscopes::common::static_vector<int, 10>, int> m;

  return 0;
}
