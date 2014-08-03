#include <microscopes/common/relation/dataview.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/common/random_fwd.hpp>

#include <random>
#include <iostream>
#include <vector>
#include <memory>

using namespace std;
using namespace microscopes::common;
using namespace microscopes::common::relation;

static void
CheckDataview2DArray(
    const bool *data,
    const bool *mask,
    size_t n,
    size_t m,
    const dataview &d)
{
  MICROSCOPES_CHECK(d.shape().size() == 2, "not a 2D array");
  // check each slice
  for (size_t i = 0; i < n; i++) {
    vector<bool> seen(m, false);
    for (auto &p : d.slice(0, i)) {
      MICROSCOPES_CHECK(p.first[0] == i, "not a valid slice");
      MICROSCOPES_CHECK(p.first[1] < m, "out of bounds");
      const size_t idx = i*m + p.first[1];
      const bool value = p.second.get<bool>(0);
      MICROSCOPES_CHECK(data[idx] == value, "values don't match");
      MICROSCOPES_CHECK(!mask[idx], "data is masked");
      seen[p.first[1]] = true;
    }
    for (size_t j = 0; j < seen.size(); j++) {
      if (seen[j])
        continue;
      const size_t idx = i*m + j;
      MICROSCOPES_CHECK(mask[idx], "data is not masked");
    }
  }
}

static void
test1()
{
  random_device rd;
  rng_t r(rd());
  size_t A = 10;
  size_t B = 100;
  bool *data = new bool[A*B];
  bool *masks = new bool[A*B];

  for (size_t u = 0; u < A; u++) {
    for (size_t m = 0; m < B; m++) {
      const size_t idx = u*B + m;
      // coin flip to see if this data is present
      if (bernoulli_distribution(0.2)(r)) {
        masks[idx] = false;
        // coin flip to see if friends
        data[idx] = bernoulli_distribution(0.8)(r);
      } else {
        masks[idx] = true;
      }
    }
  }

  unique_ptr<dataview> view(
    new row_major_dense_dataview(
        reinterpret_cast<uint8_t*>(data), masks,
        {A, B}, runtime_type(TYPE_B)));

  CheckDataview2DArray(data, masks, A, B, *view);
}

int
main(void)
{
  test1();
  return 0;
}
