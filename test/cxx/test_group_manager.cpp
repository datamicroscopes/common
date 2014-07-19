#include <microscopes/common/group_manager.hpp>

using namespace std;
using namespace microscopes::common;

typedef fixed_group_manager<size_t> fixed_group;
typedef group_manager<size_t> group;

static inline bool
almost_eq(float a, float b)
{
  return fabs(a - b) <= 1e-5;
}

template <typename T>
static void assert_vectors_equal(const vector<T> &as, const vector<T> &bs)
{
  MICROSCOPES_DCHECK(as.size() == bs.size(), "size");
  for (size_t i = 0; i < as.size(); i++)
    MICROSCOPES_DCHECK(as[i] == bs[i], "element");
}

static void
test_fixed_serialization()
{
  const size_t k = 3;
  fixed_group fg(10, k);

  for (size_t i = 0; i < k; i++)
    fg.get_hp_mutator("alphas").set<float>(1.+float(i), i);

  const vector<ssize_t> assignment_vec({
      -1, 2, 1, 0, 0, 1, 2, -1, -1, 0
  });
  for (size_t i = 0; i < assignment_vec.size(); i++) {
    if (assignment_vec[i] == -1)
      continue;
    fg.add_value(assignment_vec[i], i)++;
  }

  const auto serialized = fg.serialize([](size_t i) {
    return to_string(i);
  });

  fixed_group fg1(serialized, [](const string &s) {
      return strtoul(s.c_str(), nullptr, 10);
  });

  for (size_t i = 0; i < k; i++)
    MICROSCOPES_DCHECK(
        almost_eq(
          fg.get_hp_mutator("alphas").accessor().get<float>(i),
          fg1.get_hp_mutator("alphas").accessor().get<float>(i)),
      "did not save alphas properly");

  assert_vectors_equal(fg.assignments(), fg1.assignments());
  MICROSCOPES_DCHECK(fg.ngroups() == fg1.ngroups(), "ngroups");
  for (size_t i = 0; i < fg.ngroups(); i++)
    MICROSCOPES_DCHECK(fg.group(i) == fg1.group(i), "group count/data");
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

  MICROSCOPES_DCHECK(
      almost_eq(
        g.get_hp_mutator("alpha").accessor().get<float>(0),
        g1.get_hp_mutator("alpha").accessor().get<float>(0)),
    "did not save alpha properly");

  assert_vectors_equal(g.assignments(), g1.assignments());
  MICROSCOPES_DCHECK(g.ngroups() == g1.ngroups(), "ngroups");
  for (auto gid : g.groups())
    MICROSCOPES_DCHECK(g.group(gid) == g1.group(gid), "group count/data");
}

int
main(void)
{
  test_fixed_serialization();
  test_serialization();
  return 0;
}
