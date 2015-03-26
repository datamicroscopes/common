#include <microscopes/common/group_manager.hpp>

using namespace std;
using namespace microscopes::common;

typedef group_manager<size_t> group;

static inline bool
almost_eq(float a, float b)
{
  return fabs(a - b) <= 1e-5;
}

template <typename T>
static void assert_vectors_equal(const vector<T> &as, const vector<T> &bs)
{
  MICROSCOPES_CHECK(as.size() == bs.size(), "size");
  for (size_t i = 0; i < as.size(); i++)
    MICROSCOPES_CHECK(as[i] == bs[i], "element");
}

static void
test_serialization()
{
  group g(10);

  g.get_hp_mutator("alpha").set<float>(2.0, 0);

  const vector<ssize_t> assignment_vec({
      -1, 2, 1, 0, 6, 1, 2, -1, -1, 5
  });
  for (size_t i = 0; i < 7; i++)
    g.create_group();
  g.delete_group(3);
  for (size_t i = 0; i < assignment_vec.size(); i++) {
    if (assignment_vec[i] == -1)
      continue;
    g.add_value(assignment_vec[i], i)++;
  }

  const auto serialized = g.serialize([](size_t i) {
    return to_string(i);
  });

  group g1(serialized, [](const string &s) {
      return strtoul(s.c_str(), nullptr, 10);
  });

  MICROSCOPES_CHECK(
      almost_eq(
        g.get_hp_mutator("alpha").accessor().get<float>(0),
        g1.get_hp_mutator("alpha").accessor().get<float>(0)),
    "did not save alpha properly");

  assert_vectors_equal(g.assignments(), g1.assignments());
  MICROSCOPES_CHECK(g.ngroups() == g1.ngroups(), "ngroups");
  for (auto gid : g.groups())
    MICROSCOPES_CHECK(g.group(gid) == g1.group(gid), "group count/data");
}

int
main(void)
{
  test_serialization();
  return 0;
}
